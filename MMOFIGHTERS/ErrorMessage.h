#pragma once

namespace Common
{
	enum class eERROR_MESSAGE
	{
		WSA_START_FAIL,
		MAKE_SOCKET_FAIL,
		SET_SOCKET_NONBLOCKING_FAIL,
		SET_LINGER_FAIL,
		SET_SERVER_CONFIG_FAIL,
		BIND_SERVER_FAIL,
		SET_LISTEN_FAIL,
		SELECT_FAIL,
		RECV_DEQUEUE_ERROR,

		ERROR_MESSAGE_CNT,

		SUCCESS,
	};
}