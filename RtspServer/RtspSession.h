#pragma once
#include "RtspParser.h"
#include "TcpStream.h"
#include <chrono>

class RtspSession
{
public:
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using time_duration = std::chrono::milliseconds;
private:
	RtspParser parser;
	TcpStream stream;
	// last transmit time
	//time_point last_active;

public:
	//SOCKET client_sock;
	RtspSession(SOCKET client_sock);
	~RtspSession() noexcept;
	bool on_readable();
	SOCKET getSocket() const;

	bool rtp_enabled = false;
	//std::chrono rtp_timeout = 40ms;
	time_duration rtp_timeout;
	time_point next_rtp_timeout;
	bool rtp_time_out();

	
};

