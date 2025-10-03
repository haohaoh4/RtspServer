#include "TcpStream.h"
#include <iostream>

TcpStream::TcpStream(std::string addr) {
	
	SOCKET s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
	}

	//inet_pton(AF_INET6, addr.c_str(), &((sockaddr_in6*)&((sockaddr_in6*)&addr)->sin6_addr));,
}