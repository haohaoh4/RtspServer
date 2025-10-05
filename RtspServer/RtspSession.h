#pragma once
#include "RtspParser.h"
#include "TcpStream.h"
#include <chrono>

class RtspSession
{
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	RtspParser parser;
	TcpStream stream;
	// last transmit time
	time_point last_active;

public:
	//SOCKET client_sock;
	RtspSession(SOCKET client_sock);
	~RtspSession() noexcept;
	bool on_readable();
	SOCKET getSocket() const;

	// process timeout
	//bool on_timeout();
};

