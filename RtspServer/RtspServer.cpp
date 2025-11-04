#include "RtspServer.h"
#include <algorithm>
#include <csignal>
#include "net_compat.h"
//
RtspServer::RtspServer(const Config& cfg) : config(cfg) {
	running = true;

#ifdef _WIN32
	timeBeginPeriod(2);
	std::call_once(wsa_flag, []() {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
		}
		std::cout << "Startup WSA" << std::endl;
		});
#endif	

	running = true;
	std::signal(SIGINT, ctrlc_handler);

	std::cout << config.address << ":" << config.port << std::endl;
	server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sock == INVALID_SOCKET) {
		throw std::runtime_error("socket failed: " + std::to_string(last_net_error()));
	}

	sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, config.address.c_str(), &server_addr.sin_addr);
	server_addr.sin_port = htons(config.port);

	if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		closesocket(server_sock);
		throw std::runtime_error("bind failed: " + std::to_string(last_net_error()));
		throw std::runtime_error("bind failed: ");
	}

	if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(server_sock);
		throw std::runtime_error("listen failed: " + std::to_string(last_net_error()));
	}
	std::cout << "Server listening on " << config.address << ":" << config.port << std::endl;
}

RtspServer::~RtspServer() {
	net_cleanup();
	std::cout << "Cleanup WSA" << std::endl;
}

void RtspServer::run() {
	std::cout << "Server running..." << std::endl;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(server_sock, &readfds);
	while (running) {

		int max_fd = server_sock;
		//std::cout << "Waiting for connections..." << std::endl;

		timeval timeout_storage = {};
		timeval* select_timeout_p = nullptr;
		auto now = RtspSession::clock::now();
		auto nearest_timeout = now + std::chrono::seconds(60);
		// format time to text
		//std::cout << "nearest_timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(nearest_timeout.time_since_epoch()).count() << std::endl;
		for (const auto& session : sessions) {
			max_fd = std::max(max_fd, (int)session->getSocket());
			if (session->rtp_enabled) {
				if(session.get()->next_rtp_timeout < nearest_timeout) {
					nearest_timeout = session->next_rtp_timeout;
				}
				//std::cout << "new nearest_timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(nearest_timeout.time_since_epoch()).count() << std::endl;
				select_timeout_p = &timeout_storage;
				//break;
			}
		}
		if (nearest_timeout < now) {
			nearest_timeout = now;
		}
		if (select_timeout_p != nullptr) {
			auto timeout_ms = std::chrono::duration_cast<std::chrono::milliseconds>(nearest_timeout - now).count();
			if (timeout_ms > 1000) timeout_ms = 1000;
			select_timeout_p->tv_sec = timeout_ms / 1000;
			select_timeout_p->tv_usec = (timeout_ms % 1000) * 1000;
			//std::cout << "Next RTP timeout in " << timeout_ms << " ms" << std::endl;
		}

		fd_set tmpfds = readfds;
		int activity = select(max_fd + 2, &tmpfds, NULL, NULL, select_timeout_p);
		if (activity == SOCKET_ERROR) {
			std::cerr << "select failed: " << last_net_error() << std::endl;
			break;
		}
		//std::cout << "Activity detected..." << activity << std::endl;

		if (FD_ISSET(server_sock, &tmpfds)) {
			socket_t client_sock = accept(server_sock, NULL, NULL);
			if (client_sock == INVALID_SOCKET) {
				std::cerr << "accept failed: " << last_net_error() << std::endl;
				continue;
			}
			sessions.push_back(std::make_unique<RtspSession>(client_sock)); // add new session
			FD_SET(client_sock, &readfds);
			std::cout << "Accepted new connection, total sessions: " << sessions.size() << std::endl;
		}
		//std::cout << "Checking sessions for readability and time out..." << std::endl;
		now = std::chrono::steady_clock::now();
		for(auto it=sessions.begin(); it!=sessions.end(); ) {
			//std::cout << "Checking session with socket: " << (*it)->getSocket() << std::endl;
			if ((*it)->rtp_enabled && (*it)->next_rtp_timeout <= now) {
				(*it)->rtp_time_out();
				//std::cout << "RTP timeout occurred for socket: " << (*it)->getSocket() << std::endl;
			}
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

void RtspServer::ctrlc_handler(int) {
	std::cout << "Ctrl-C pressed, stopping server..." << std::endl;
#ifdef _WIN32
	timeEndPeriod(2);
	WSACleanup();
#endif
	running = false;
}