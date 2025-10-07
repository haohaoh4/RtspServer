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
	uint32_t rtp_seq;
	uint32_t rtp_octet_count;

	std::ifstream h264_file;
	uint8_t* m_nalu_buf;
	int m_nalu_buf_size;
	int get_frame_from_file(uint8_t* nalu_buf, size_t buf_size);	
	bool rtp_send_H264_Frame(const struct RtpHeader& rtp_header, const uint8_t* nalu_buf, size_t nalu_size, bool& is_marker);
	int timestamp_inc; // arbitrary timestamp increment
	int timestamp, rtcp_sr_timestamp;
	uint64_t m_ntp_timestamp;
	char* read_buffer = nullptr;

	time_point last_rtcp_message;
	void rtcp_message();
	uint64_t ntp64_now(std::chrono::system_clock::time_point now);

	time_duration m_rtcp_interval;
};

