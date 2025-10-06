#pragma once

#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mutex>
#include <span>
#include <array>

//#include <threads.h>

#pragma comment(lib, "ws2_32.lib")

class TcpStream
{
	SOCKET sock;
	//std::string addr;
	//UINT16 port;

	//char buffer[4096];
	//int buffer_pos = 0;
	//int buffer_end = 0;

public:
	TcpStream(SOCKET sock);
	~TcpStream() noexcept;
	SOCKET getSocket() const;

	bool write(std::span<const char> data);
	int read(std::span<char> data);
};




