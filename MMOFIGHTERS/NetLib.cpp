#include "NetLib.h"
#include "Logger.h"
#include "Session.h"
#include "NetDefine.h"
#include "MessageFormat.h"
#include "ObjectPool.h"

using namespace Common;
using namespace NetLib;

NetWorkLib::~NetWorkLib()
{
	//Session들 메모리 풀로 반납. 
	for (auto& session : _Sessions)
	{
		Session* cur = session.second;
		_SessionPool->deAllocate(cur);
	}

	closesocket(_ListenSocket);
	::WSACleanup();
}

eERROR_MESSAGE NetWorkLib::Init()
{
	WSADATA wsaData;

	int Flag;
	int errorCode;
	Flag = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"WSA_START_UP_FAIL");
		return eERROR_MESSAGE::WSA_START_FAIL;
	}

	_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (_ListenSocket == INVALID_SOCKET)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"MAKE_SOCKET_ERROR");
		return eERROR_MESSAGE::MAKE_SOCKET_FAIL;
	}

	ULONG on = 1;
	Flag = ::ioctlsocket(_ListenSocket, FIONBIO, &on);
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"Set nonblocking IO Error");
		return eERROR_MESSAGE::SET_SOCKET_NONBLOCKING_FAIL;
	}

	LINGER option;
	option.l_onoff = 1;
	option.l_linger = 0;
	Flag = ::setsockopt(_ListenSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&option), sizeof(option));
	if (Flag != 0)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"SET LINGER ERROR");
		return eERROR_MESSAGE::SET_LINGER_FAIL;
	}

	if (!ReadConfig())
	{
		Logger::Logging(-1, __LINE__, L"READ SERVER_CONFIG_ERROR");
		return eERROR_MESSAGE::SET_SERVER_CONFIG_FAIL;
	}

	SOCKADDR_IN addrInfo;
	::ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_port = htons(_ServerConfig._Port);
	::InetPton(AF_INET, L"0.0.0.0", &addrInfo.sin_addr);

	Flag = ::bind(_ListenSocket, reinterpret_cast<SOCKADDR*>(&addrInfo), sizeof(addrInfo));
	if (Flag == SOCKET_ERROR)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"BIND_SERVER_FAIL");
		return eERROR_MESSAGE::BIND_SERVER_FAIL;
	}

	Flag = ::listen(_ListenSocket, SOMAXCONN_HINT(65535));
	if (Flag == SOCKET_ERROR)
	{
		errorCode = ::WSAGetLastError();
		Logger::Logging(errorCode, __LINE__, L"LISTEN_ERROR");
		return eERROR_MESSAGE::SET_LISTEN_FAIL;
	}

	_Sessions.reserve(SESSION_RESERVE_SIZE);
	return eERROR_MESSAGE::SUCCESS;
}

void NetWorkLib::Process()
{
	int Flag;

	TIMEVAL option = { 0,0 };

	const int SELECT_MAX_CNT = 63;
	int maxLoopCnt = 0;

	/*이번 프레임의 인원들에 대해서만 처리해줌.*/
	auto SesionStart_iter = _Sessions.begin();
	auto iter = _Sessions.begin();
	auto SessionEnd_iter = _Sessions.end();

	while (true)
	{
		FD_SET readSet;
		FD_SET writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		maxLoopCnt = 0;
		FD_SET(_ListenSocket, &readSet);

		for (; iter != SessionEnd_iter && maxLoopCnt < SELECT_MAX_CNT; ++iter)
		{
			Session* curSession;
			curSession = iter->second;
			FD_SET(curSession->GetSocket(), &readSet);
			if (curSession->CanSendData())
			{
				FD_SET(curSession->GetSocket(), &writeSet);
			}
			++maxLoopCnt;
		}

		Flag = ::select(0, &readSet, &writeSet, nullptr, &option);
		if (Flag == SOCKET_ERROR)
		{
			Logger::Logging(static_cast<int>(eERROR_MESSAGE::SELECT_FAIL), __LINE__, L"SELECT ERROR");
			DebugBreak();
		}
		if (FD_ISSET(_ListenSocket, &readSet))
		{
			_AcceptProc();
		}

		for (; SesionStart_iter != iter; ++SesionStart_iter)
		{
			Session* curSession;
			curSession = SesionStart_iter->second;
			if (FD_ISSET(curSession->GetSocket(), &readSet))
			{
				_RecvProc(curSession);
			}

			if (FD_ISSET(curSession->GetSocket(), &writeSet))
			{
				//SendQueue에 복사된거 보내기.
				_SendProc(curSession);
			}
		}

		if (SesionStart_iter == SessionEnd_iter)
		{
			break;
		}

	}
}

void NetWorkLib::_RecvProc(Session* session)
{
	int recvLen;
	int errorCode;

	CircularQueue* const pRecvQ = session->_pRecvQueue;
	recvLen = ::recv(session->GetSocket(), pRecvQ->GetRearPtr(), pRecvQ->GetDirect_EnqueueSize(), 0);
	if (recvLen <= 0)
	{
		errorCode = ::WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK
			|| recvLen == 0
			|| errorCode == WSAECONNABORTED
			|| errorCode == WSAECONNRESET)
		{
#ifdef GAME_DEBUG
			printf("RECV ERROR NORMAL DISCONNECT!\n");
#endif
			OnDestroyProc(session->GetSessionKey());
			return;
		}
		//다른 에러가 기록되는 경우
#ifdef GAME_DEBUG
		printf("RECV ERROR UNUSAL DISCONNECT!\n");
#endif
		Logger::Logging(errorCode, __LINE__, L"Recv Error");
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	pRecvQ->MoveRear(recvLen);

	int peekMessageLen;
	int payLoadLen;
	int dequeueLen;

	//auto& SbufferPool = ObjectPool<SerializeBuffer, static_cast<size_t>(NETLIB_POOL_SIZE::SBUFFER_POOL_SIZE)>::getInstance();

	// 한번 받은 메시지는 모두 처리한다.
	while (true)
	{
		header_t header;
		peekMessageLen = pRecvQ->Peek(reinterpret_cast<char*>(&header), sizeof(header_t));
		if (peekMessageLen < sizeof(header_t))
		{
			break;
		}

		if (header._Code != SIGNITURE)
		{
			OnDestroyProc(session->GetSessionKey());
			break;
		}

		payLoadLen = header._PayloadLen;
		//메시지 완성되었는지 확인
		if (pRecvQ->GetCurrentSize() < payLoadLen + sizeof(header_t))
		{
			break;
		}

		SerializeBuffer* sbuffer = _SbufferPool->allocate();
		sbuffer->clear();
		dequeueLen = pRecvQ->Dequeue(sbuffer->getBufferPtr(), payLoadLen + sizeof(header_t));
		//여기서 dequeueLen이 요청한 크기보다 작은건 내가 잘못만든거임. 

		if (dequeueLen != payLoadLen + sizeof(header_t))
		{
			Logger::Logging(static_cast<int>(eERROR_MESSAGE::RECV_DEQUEUE_ERROR), __LINE__, L"RECV DEQUEUE ERROR");
			_SbufferPool->deAllocate(sbuffer);
			DebugBreak();
			break;
		}
		sbuffer->moveWritePos(dequeueLen);
		sbuffer->getData(reinterpret_cast<char*>(&header), sizeof(header_t));
#ifdef GAME_DEBUG
		int debugKey = session->GetSessionKey();
		printf("============================================================\n");
		printf("SESSION Key : %d | In Network | \n", debugKey);
		printf("RECEIVE HEADER , CODE : %d | TYPE :%d | PAYLOADLEN : %d\n", header->_Code, header->_MessageType, header->_PayloadLen);
		printf("============================================================\n");
#endif
		//payload만 넘기기
		OnRecvProc(sbuffer, header._MessageType, session->GetSessionKey());
		_SbufferPool->deAllocate(sbuffer);
	}
	return;
}

void NetWorkLib::_AcceptProc()
{
	int errorCode;
	SOCKET connectSocket;
	SOCKADDR_IN connectInfo;
	ZeroMemory(&connectInfo, sizeof(connectInfo));
	int conncetLen = sizeof(connectInfo);

	connectSocket = ::accept(_ListenSocket, reinterpret_cast<SOCKADDR*>(&connectInfo), &conncetLen);
	if (connectSocket == INVALID_SOCKET)
	{
		errorCode = ::WSAGetLastError();
		if (errorCode != WSAEWOULDBLOCK)
		{
			return;
		}
		Logger::Logging(errorCode, __LINE__, L"ACCPET Error");
		return;
	}

	//성공적으로 Accpet
	//세션 생성
	Session* newSession = _SessionPool->allocate();
	int key = newSession->GenerateSessionKey();
	newSession->InitSession(connectSocket, connectInfo, key);
	_Sessions.insert({ key, newSession });

	OnAcceptProc(key);
}

void NetWorkLib::_SendProc(Session* session)
{
	int sendLen;
	int errorCode;
	CircularQueue* const pSendQueue = session->_pSendQueue;
	int sendQLen = pSendQueue->GetDirect_DequeueSize();

	sendLen = ::send(session->GetSocket(), pSendQueue->GetFrontPtr(), sendQLen, 0);
	if (sendLen == SOCKET_ERROR)
	{
		// send시 WOULDBLOCK는 L4의 송신버퍼가 꽉찼다는거다. 이건 상대방의 수신이 다 찼다는거임. 
		// 나머지 에러들은 연결이 끊겼거나.. 등에는 그냥 끊어주면 됨.
		// 안끊어줘야할 사유가 있나?
#ifdef GAME_DEBUG
		printf("SEND ERROR\n");
#endif
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	if (sendLen < sendQLen)
	{
#ifdef GAME_DEBUG
		printf("L4 BUFFER IS FULL DISCONNECT!\n");
#endif
		OnDestroyProc(session->GetSessionKey());
		return;
	}
	pSendQueue->MoveFront(sendLen);
}

void NetWorkLib::SendUniCast(const SESSION_KEY sessionKey, SerializeBuffer* message, const size_t messageLen)
{
	int enqueueLen;
	const auto& iter = _Sessions.find(sessionKey);
	if (iter != _Sessions.end())
	{
		Session* findSession = iter->second;
		if (findSession->GetConnection() == false)
		{
			return;
		}

		CircularQueue* const curSendQ = findSession->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message->getBufferPtr(), messageLen);
		message->moveReadPos(enqueueLen);

		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(findSession->GetSessionKey());
			return;
		}
	}
	return;
}

void NetWorkLib::SendBroadCast(SerializeBuffer* message, const size_t messageLen)
{
	int enqueueLen;
	for (auto& session : _Sessions)
	{
		if (session.second->GetConnection() == false)
		{
			continue;
		}

		CircularQueue* const curSendQ = session.second->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message->getBufferPtr(), messageLen);

		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(session.second->GetSessionKey());
			continue;
		}
	}
	message->moveReadPos(messageLen);
}

void NetWorkLib::SendBroadCast(SESSION_KEY exceptSession, SerializeBuffer* message, const size_t messageLen)
{
	int enqueueLen;
	for (auto& session : _Sessions)
	{
		if (session.second->GetConnection() == false)
		{
			continue;
		}
		if (session.first == exceptSession)
		{
			continue;
		}

		CircularQueue* const curSendQ = session.second->_pSendQueue;
		enqueueLen = curSendQ->Enqueue(message->getBufferPtr(), messageLen);

		if (enqueueLen < static_cast<int>(messageLen))
		{
#ifdef GAME_DEBUG
			printf("L7 BUFFER IS FULL DISCONNECT!\n");
#endif
			Logger::Logging(-1, __LINE__, L"L7 Buffer is FULL");
			OnDestroyProc(session.second->GetSessionKey());
			continue;
		}
	}
	message->moveReadPos(messageLen);
}

void NetWorkLib::Disconnect(SESSION_KEY sessionKey)
{
	const auto& iter = _Sessions.find(sessionKey);
	Session* cur = iter->second;
	if (iter != _Sessions.end())
	{
		cur->SetDisconnect();
		return;
	}
	return;
}

void NetWorkLib::CleanupSession()
{
	auto iter = _Sessions.begin();
	auto iter_e = _Sessions.end();

	for (; iter != iter_e; )
	{
		Session* cur = iter->second;
		int sessionKey = iter->first;
		if (cur->GetConnection() == false)
		{
			closesocket(cur->GetSocket());
			_SessionPool->deAllocate(cur);
			iter = _Sessions.erase(iter);
			continue;
		}
		++iter;
	}
}

void NetLib::NetWorkLib::registSessionPool(ObjectPool<Session, SESSION_POOL_SIZE, false>* sessionpool)
{
	_SessionPool = sessionpool;
}

void NetLib::NetWorkLib::registSBufferPool(ObjectPool<SerializeBuffer, SBUFFER_POOL_SIZE, false>* sbufferpool)
{
	_SbufferPool = sbufferpool;
}

bool NetWorkLib::ReadConfig()
{
	FILE* fp;
	_wfopen_s(&fp, L"ServerConfig.txt", L"rb");
	if (fp == NULL)
	{
		return false;
	}

	const WCHAR* delims = L"\t, \r\n";
	const int BUFFER_SIZE = 128;
	WCHAR buffer[BUFFER_SIZE] = { 0, };
	WCHAR* Token;
	//Read Header
	fgetws(buffer, BUFFER_SIZE, fp);
	memset(buffer, 0, sizeof(buffer));
	//Read {
	fgetws(buffer, BUFFER_SIZE, fp);
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		if (fgetws(buffer, BUFFER_SIZE, fp) == nullptr)
		{
			break;
		}

		//PORT  포트정보
		WCHAR* tmp = nullptr;
		Token = wcstok_s(buffer, delims, &tmp);

		if (wcscmp(L"PORT", Token) == 0)
		{
			Token = wcstok_s(NULL, delims, &tmp);
			swscanf_s(Token, L"%hd", &_ServerConfig._Port);
			continue;
		}
	}
	fclose(fp);
	return true;
}
