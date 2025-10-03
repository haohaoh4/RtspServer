#include "TcpStream.h"
#include <iostream>


TcpStream::TcpStream(SOCKET sock) : sock(sock) {
	std::cout << "TcpStream created with socket: " << sock << std::endl;
	if (sock == INVALID_SOCKET) {
		throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
	}

}
TcpStream::~TcpStream() {
	//add something
}
SOCKET TcpStream::getSocket() const {
	return sock;
}