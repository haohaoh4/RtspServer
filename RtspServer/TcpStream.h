#pragma once

#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mutex>
#include <threads.h>

#pragma comment(lib, "ws2_32.lib")

class TcpStream
{
	SOCKET sock;
public:
	TcpStream(std::string addr);

};




