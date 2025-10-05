#include "RtspServer.h"
#include <algorithm>

RtspServer::RtspServer(const Config& cfg) : config(cfg) {
	std::cout << config.address << ":" << config.port << std::endl;
	std::call_once(wsa_flag, []() {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
		}
		std::cout << "Startup WSA" << std::endl;
		});

	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock == INVALID_SOCKET) {
		throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
	}

	sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, config.address.c_str(), &server_addr.sin_addr);
	server_addr.sin_port = htons(config.port);
	if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		closesocket(server_sock);
		throw std::runtime_error("bind failed: " + std::to_string(WSAGetLastError()));
	}

	if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(server_sock);
		throw std::runtime_error("listen failed: " + std::to_string(WSAGetLastError()));
	}
	std::cout << "Server listening on " << config.address << ":" << config.port << std::endl;
}

RtspServer::~RtspServer() {
	WSACleanup();
	std::cout << "Cleanup WSA" << std::endl;
}

void RtspServer::run() {
	std::cout << "Server running..." << std::endl;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(server_sock, &readfds);
	while (1) {
		std::cout << "Waiting for connections..." << std::endl;
		fd_set tmpfds = readfds;
		int activity = select(0, &tmpfds, NULL, NULL, NULL);
		if (activity == SOCKET_ERROR) {
			std::cerr << "select failed: " << WSAGetLastError() << std::endl;
			break;
		}
		std::cout << "Activity detected..." << activity << std::endl;

		if (FD_ISSET(server_sock, &tmpfds)) {
			SOCKET client_sock = accept(server_sock, NULL, NULL);
			if (client_sock == INVALID_SOCKET) {
				std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
				continue;
			}
			sessions.push_back(std::make_unique<RtspSession>(client_sock));
			FD_SET(client_sock, &readfds);
			std::cout << "Accepted new connection, total sessions: " << sessions.size() << std::endl;
		}
		std::cout << "Checking sessions for readability..." << std::endl;
		for(auto it=sessions.begin(); it!=sessions.end(); ) {
			//std::cout << "Checking session with socket: " << (*it)->getSocket() << std::endl;
			if (FD_ISSET((*it)->getSocket(), &tmpfds)){
				std::cout << "Data available on socket: " << (*it)->getSocket() << std::endl;
				if (!(*it)->on_readable()) {
					std::cout << "Session closed, total sessions: " << sessions.size() - 1 << std::endl;
					FD_CLR((*it)->getSocket(), &readfds);
					//it = sessions.erase(it);
					if(it == sessions.end()-1) {
						sessions.pop_back();
						break;
					}
					std::swap(*it, sessions.back());
					sessions.pop_back();
					continue;
				}
			}
			++it;
		}


	}

}