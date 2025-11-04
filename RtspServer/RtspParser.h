#pragma once
#include <string>
#include <span>
#include <queue>
#include "RtspRequest.h"

class RtspParser
{
	enum {
		S_START,
		S_METHOD,
		S_URL,
		S_VERSION,
		S_REQUEST_LINE_ALMOST_DONE,
		S_HEADER_START,
		S_HEADER_NAME,
		S_HEADER_VALUE,
		S_HEADER_ALMOST_DONE, 
		S_HEADERS_ALMOST_DONE,
		S_DONE,
		S_RTSP_CONTENT_HEADER1,
		S_RTSP_CONTENT_HEADER2,
		S_RTSP_CONTENT_LEN1,
		S_RTSP_CONTENT_LEN2,
		S_ERROR
	} state;
	std::string buffer;
	int pos, beg, last, mark, value_start, header_start;
	int rtsp_content_packet_len;
public:
	RtspParser();
	~RtspParser() noexcept;
	
	void reset();
	enum ParseResult {
		PR_OK,
		PR_INCOMPLETE,
		PR_ERROR
	};
	//ParseResult parse(const char* data, size_t len);
	ParseResult parse(std::span<const char> data);
	RtspRequest get_request() const;
	std::queue<RtspRequest> requests;
	RtspRequest current_request;
	ParseResult rtsp_content_parse();

};

