#pragma once

#include "TcpStream.h"
#include <iostream>

class RtspServer
{
	static inline std::once_flag wsa_flag;
public:
	struct Config {
		std::string address = "::1";
		uint16_t port = 8554;
	};
	RtspServer(const Config& cfg = Config());
	~RtspServer() noexcept;
	void run();
	void stop();
	Config config;

};
void func();

