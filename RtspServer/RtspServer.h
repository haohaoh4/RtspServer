#pragma once

#include "TcpStream.h"
#include "RtspSession.h"
#include <iostream>
#include <vector>

class RtspServer
{
	static inline std::once_flag wsa_flag;
public:
	struct Config {
		std::string address = "0.0.0.0";
		//uint16_t port = 554;
		uint16_t port = 8554;
		Config() : port(8554), address("0.0.0.0") {
			
		}
	};
	RtspServer(const Config& cfg = Config());
	~RtspServer() noexcept;
	void run();
	//static void stop();
	Config config;
	socket_t server_sock;
	//RTSP sessions management
	std::vector<std::unique_ptr<RtspSession>> sessions;

	inline static bool volatile running;
	// ctrl-c handler to stop the server
	static void ctrlc_handler(int);
};

