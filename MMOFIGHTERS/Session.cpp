#include "Session.h"
#include "ObjectPool.h"

using namespace NetLib;

Session::Session()
{
	_pRecvQueue = new CircularQueue(1024);
	_pSendQueue = new CircularQueue(8192);
}

void Session::InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const int key)
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

