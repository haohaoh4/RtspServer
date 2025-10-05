#include "RtspSession.h"
#include <iostream>
#include <array>

RtspSession::RtspSession(SOCKET client_sock) : stream(client_sock) {

}
RtspSession::~RtspSession() {

}
bool RtspSession::on_readable() {
	last_active = clock::now();
	bool res = false;
	std::array<char, 4096> buffer;
	res = stream.read(buffer);

	auto pr = parser.parse(std::span<const char>(buffer.data(), buffer.size()));
	if(pr == RtspParser::PR_ERROR) {
		// parse error
		return false;
	}else if(pr == RtspParser::PR_INCOMPLETE) {
		// need more data
		return true;
	}
	else {
		RtspRequest req;
		while(!parser.requests.empty()) {
			req = parser.requests.front();
			parser.requests.pop();
		
			std::cout << "Received RTSP request: " << int(req.method) << " " << req.url << " " << req.version << std::endl;
			switch (req.method) {
			case RtspMethod::OPTIONS: {
				//std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nPublic: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n";
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nPublic: OPTIONS, DESCRIBE\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::DESCRIBE: {
				std::string rtsp_rep = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nContent-Base: " + req.url + "\r\nContent-Type: application/sdp\r\nContent-Length: ";
				std::string sdp_rep = "v=0\r\no=- 0 0 IN IP4\r\n\r\n";
				//std::string sdp_body = "m=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\n";
				std::string response = rtsp_rep + std::to_string(sdp_rep.size()) + "\r\n\r\n" + sdp_rep;
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::SETUP:{
				// Transport TP/AVP/TCP;unicast;interleaved=
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\nSession: 12345678\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::PLAY: {
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nSession: 12345678\r\nRTP-Info: url=" + req.url + ";seq=0;rtptime=0\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				// send RTP packets here
				response = "$\x00\x00\x1c\x80" + std::string(28, '\x00') + "\x65" + std::string(99, '\x00') + "\x00" + std::string(1000, '\x00');
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::TEARDOWN: {
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nSession: 12345678\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				// close session
				return false;
				break;
			}
			default:
				// unhandled method
				std::string response = "RTSP/1.0 501 Not Implemented\r\nCSeq: " + req.headers["CSeq"] + "\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
		}
		parser.reset();
	}
	
	return true;
}
SOCKET RtspSession::getSocket() const {
	return stream.getSocket();
}
