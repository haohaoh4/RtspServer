#include "TcpStream.h"
#include <iostream>


TcpStream::TcpStream(SOCKET sock) : sock(sock) {
	std::cout << "TcpStream created with socket: " << sock << std::endl;
	if (sock == INVALID_SOCKET) {
		throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
	}
	//setsockopt()
}
TcpStream::~TcpStream() {
	shutdown(sock, SD_BOTH);
}
SOCKET TcpStream::getSocket() const {
	return sock;
}
int TcpStream::read(std::span<char> data) {
	int recv_len = recv(sock, data.data(), data.size(), 0);
	if (recv_len == SOCKET_ERROR) {
		return -1;
	} else {
		return recv_len;
	}
}
bool TcpStream::write(std::span<const char> data) {
	auto datatmp = data;
	while(datatmp.size() > 0) {
		int sent_len = send(sock, datatmp.data(), datatmp.size(), 0); // 不保证一次发完所有数据
		if (sent_len == SOCKET_ERROR) {
			return false;
		}
		datatmp = datatmp.subspan(sent_len);
	}
	return true;
}
/*bool TcpStream::readLine(std::string& line) {
	line.clear();
	char* line_end = nullptr;
	while (line_end != nullptr) {
		int recv_len = recv(sock, buffer + buffer_end, sizeof(buffer) - buffer_end - 1, 0);
		if (recv_len == SOCKET_ERROR) {
			std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
			return false;
		}
		buffer_end += recv_len;
		if (buffer_end == sizeof(buffer) - 1) {
			memmove(buffer, buffer + buffer_pos, buffer_end - buffer_pos);
			buffer_pos = 0;
			buffer_end -= buffer_pos;
			if(buffer_end == sizeof(buffer) - 1) {
				std::cerr << "Line too long" << std::endl;
				return false;
			}
		}
		buffer[buffer_end] = '\0';
		line_end = strchr(buffer + buffer_pos, '\n');
	}
}*/