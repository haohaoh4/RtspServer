#include "RtspSession.h"
#include <iostream>

RtspSession::RtspSession(SOCKET client_sock):stream(client_sock) {

}
RtspSession::~RtspSession() {

}
bool RtspSession::on_readable() {
	char buffer[4096];
	int bytes_received = recv(stream.getSocket(), buffer, sizeof(buffer) - 1, 0);
	std::cout << "Received " << bytes_received << " bytes" << std::endl;
	if (bytes_received == SOCKET_ERROR) {
		std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	if (bytes_received == 0) {
		std::cout << "Client disconnected" << std::endl;
		return false;
	}
	return true;
}
SOCKET RtspSession::getSocket() const {
	return stream.getSocket();
}
