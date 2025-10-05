#include "RtspParser.h"
#include <cstring>
#include <iostream>

RtspParser::RtspParser() {
	reset();
}
RtspParser::~RtspParser() noexcept {
	reset();
}
RtspParser::ParseResult RtspParser::parse(std::span<const char> data) {
	buffer.append(data.data(), data.size());
	last = buffer.size();

	for (; pos < last; pos++) {
		switch (state) {
			case S_RTSP_CONTENT:
				return rtsp_content_parse();
			case S_START:
				current_request = RtspRequest{ RtspMethod::NONE };

				if (buffer[pos] == '$') {
					state = S_RTSP_CONTENT;
					return rtsp_content_parse();
				} else if(buffer[pos] < 'A' || buffer[pos] > 'Z') {
					state = S_ERROR;
					return PR_ERROR;
				} else {
					mark = pos;
					state = S_METHOD;
				}
				break;
			case S_METHOD:
				if (buffer[pos] == ' ') {
					//mark = pos;
					state = S_URL;
					std::string method_str = buffer.substr(mark, pos - mark);
					current_request.method = get_rtsp_method(method_str);
					if (current_request.method == RtspMethod::UNKNOWN) {
						state = S_ERROR;
						return PR_ERROR;
					}
					beg = pos + 1;
				}
				else if (buffer[pos] < 'A' || buffer[pos] > 'Z') {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_URL:
				if (buffer[pos] == ' ') {
					state = S_VERSION;
					current_request.url = buffer.substr(mark, pos - mark);
					beg = pos + 1;
				}
				else if (buffer[pos] == '\r' || buffer[pos] == '\n' || buffer[pos] < 33 || buffer[pos]>126) {
					state = S_ERROR;
					return PR_ERROR;
				}
				else if (pos - mark > 2048) {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_VERSION:
				if (buffer[pos] == '\r') {
					state = S_REQUEST_LINE_ALMOST_DONE;
					current_request.version = buffer.substr(mark, pos - mark);
				}
				else if (buffer[pos] == '\n') {
					state = S_HEADER_START;
					current_request.version = buffer.substr(mark, pos - mark);
				}
				else if (buffer[pos] < 33 || buffer[pos]>126) {
					state = S_ERROR;
					return PR_ERROR;
				}
				else if (pos - mark > 64) {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_REQUEST_LINE_ALMOST_DONE:
				if (buffer[pos] == '\n') {
					state = S_HEADER_START;
				}
				else {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_HEADER_START:
				if (buffer[pos] == '\r') {
					state = S_HEADERS_ALMOST_DONE;
				}
				else if (buffer[pos] == '\n') {
					state = S_DONE;
					requests.push(current_request);
					goto ok_end;
				}
				else {
					mark = pos;
					state = S_HEADER_NAME;
				}
				break;
			case S_HEADER_NAME:
				if (buffer[pos] == ':') {
					header_start = mark;
					value_start = pos + 1;
					state = S_HEADER_VALUE;
				}
				else if (buffer[pos] == '\r' || buffer[pos] == '\n' || buffer[pos] < 33 || buffer[pos]>126) {
					state = S_ERROR;
					return PR_ERROR;
				}
				else if (pos - mark > 256) {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_HEADER_VALUE:
				if (buffer[pos] == '\r') {
					state = S_HEADER_ALMOST_DONE;
					// trim spaces
					while (value_start < pos && (buffer[value_start] == ' ' || buffer[value_start] == '\t')) {
						value_start++;
					}
					int value_end = pos - 1;
					while (value_end > value_start && (buffer[value_end] == ' ' || buffer[value_end] == '\t')) {
						value_end--;
					}
					int header_name_end = value_start - 1;
					while(header_name_end > header_start && (buffer[header_name_end] == ' ' || buffer[header_name_end] == '\t' || buffer[header_name_end] == ':')) {
						header_name_end--;
					}
					std::string header_name = buffer.substr(header_start, header_name_end - header_start + 1);
					std::string header_value = buffer.substr(value_start, value_end - value_start + 1);
					current_request.headers[header_name] = header_value;
					std::cout << "ADDED HEADER: '" << header_name << "' = '" << header_value << "'" << std::endl;
				}
				else if (buffer[pos] == '\n') {
					state = S_HEADER_START;
					// trim spaces
					while (value_start < pos && (buffer[value_start] == ' ' || buffer[value_start] == '\t')) {
						value_start++;
					}
					int value_end = pos - 1;
					while (value_end > value_start && (buffer[value_end] == ' ' || buffer[value_end] == '\t')) {
						value_end--;
					}
					std::string header_name = buffer.substr(header_start, value_start - header_start - 1);
					std::string header_value = buffer.substr(value_start, value_end - value_start + 1);
					current_request.headers[header_name] = header_value;
				}
				else if (pos - value_start > 4096) {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_HEADER_ALMOST_DONE:
				if (buffer[pos] == '\n') {
					state = S_HEADER_START;
				}
				else {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_HEADERS_ALMOST_DONE:
				if (buffer[pos] == '\n') {
					state = S_DONE;
					requests.push(current_request);
					goto ok_end;
				}
				else {
					state = S_ERROR;
					return PR_ERROR;
				}
				break;
			case S_DONE:
				// should not reach here
				state = S_ERROR;
				return PR_ERROR;
				break;
			case S_ERROR:
			default:
				state = S_ERROR;
				return PR_ERROR;
				break;
		}
	}
	if (state == S_ERROR) {
		return PR_ERROR;
	}
	//buffer.erase(0, beg);
	//pos -= beg;
	//mark 
	//beg = 0;
	return PR_INCOMPLETE;
ok_end:
	buffer.erase(0, pos);
	pos = 0;
	return PR_OK;
}
RtspParser::ParseResult RtspParser::rtsp_content_parse() {
	pos++;
	return PR_OK;
}

RtspRequest RtspParser::get_request() const {
	if(requests.empty()) {
		RtspRequest emptyRequest;
		emptyRequest.method = RtspMethod::UNKNOWN;
		return emptyRequest;
	}
	return requests.front();
}
void RtspParser::reset() {
	state = S_START;
	buffer.clear();
	pos = 0;
	beg = 0;
	last = 0;
	mark = 0;
	value_start = 0;
	header_start = 0;
	current_request = RtspRequest{ RtspMethod::NONE };
}