#include "RtspServer.h"

RtspServer::RtspServer(const Config& cfg) {
	std::cout << config.address << ":" << config.port << std::endl;
	std::call_once(wsa_flag, []() {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
		}
		std::cout << "Startup WSA" << std::endl;
		});


}

RtspServer::~RtspServer() {
	WSACleanup();
	std::cout << "Cleanup WSA" << std::endl;
}