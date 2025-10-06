#pragma once
#include "RtspParser.h"
#include "TcpStream.h"
#include <chrono>
#include <fstream>

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
	RtspSession(SOCKET client_sock, const std::string& h264_filename = "test.h264");
	~RtspSession() noexcept;
	bool on_readable();
	SOCKET getSocket() const;

	bool rtp_enabled = false;
	//std::chrono rtp_timeout = 40ms;
	int fps;
	time_duration rtp_timeout;
	time_point next_rtp_timeout;
	bool rtp_time_out();
	uint16_t rtp_seq = 0;

	std::ifstream h264_file;
	int get_frame_from_file(uint8_t* nalu_buf, size_t buf_size);	
	bool rtp_send_H264_Frame(const struct RtpHeader& rtp_header, const uint8_t* nalu_buf, size_t nalu_size);
	int timestamp_inc; // arbitrary timestamp increment
	int timestamp;
};

