#pragma once
#define _WINSOCKAPI_

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "CircularQueue.h"
#include "NetDefine.h"

using namespace Common;
namespace NetLib
{
	class NetWorkLib;
	class Session
	{
	public:
		Session();
		void InitSession(const SOCKET connectSocket, const SOCKADDR_IN& connectInfo, const SESSION_KEY key);
		inline SOCKET GetSocket() const
		{
			return _Socket;
		}

		inline bool CanSendData() const
		{
			return (_pSendQueue->GetCurrentSize() > 0);
		}
		inline void	SetDisconnect()
		{
			_Alive = false;
		}
		inline bool GetConnection() const
		{
			return _Alive;
		}
		static SESSION_KEY GenerateSessionKey()
		{
			static SESSION_KEY key = 0;
			return key++;
		}
		void GetIP(WCHAR* out, size_t buffersize);
		inline SESSION_KEY GetSessionKey() const
		{
			return _Key;
		}
		USHORT GetPort();
	private:
		friend class NetWorkLib;
		SOCKET _Socket;

		CircularQueue* _pSendQueue;
		CircularQueue* _pRecvQueue;
		bool _Alive;
		SESSION_KEY _Key;
		SOCKADDR_IN _AddrInfo;
	};
}