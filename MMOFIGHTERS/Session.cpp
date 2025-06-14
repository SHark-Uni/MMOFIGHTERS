#include "Session.h"
#include "ObjectPool.h"

using namespace NetLib;

Session::Session()
{
	_pRecvQueue = new CircularQueue(8192);
	_pSendQueue = new CircularQueue(RINGBUFFER_QUEUE_SIZE);
}

void Session::InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const SESSION_KEY key)
{
	_Socket = connectSocket;
	_AddrInfo = connectInfo;
	_Alive = true;
	_Key = key;

	_pRecvQueue->clear();
	_pSendQueue->clear();
}

void Session::GetIP(WCHAR* out, size_t buffersize)
{
	InetNtop(AF_INET, &_AddrInfo.sin_addr, out, sizeof(buffersize));
	return;
}

USHORT Session::GetPort()
{
	short ret = ntohs(_AddrInfo.sin_port);
	return ret;
}

