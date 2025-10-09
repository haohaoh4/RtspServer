#include "RtspSession.h"
#include <iostream>
#include <array>
#include <span>
#include <algorithm>
#include <filesystem>
#include "RtpStructs.h"

RtspSession::RtspSession(SOCKET client_sock, const std::string& h264_filename) : stream(client_sock) {
	const size_t buffer_size = 64 * 1024 * 1024;
	read_buffer = new char[buffer_size];
	h264_file.rdbuf()->pubsetbuf(read_buffer, buffer_size);
	h264_file.open(h264_filename, std::ios::in | std::ios::binary);
	if (!h264_file.is_open()) {
		throw std::runtime_error("Failed to open H264 file: " + h264_filename);
	}
	m_nalu_buf_size = 1024 * 1024;
	m_nalu_buf = new uint8_t[m_nalu_buf_size];

	rtp_enabled = false;
	rtp_seq = 0;
	rtp_octet_count = 0;

	fps = 30;
	timestamp = 0;
	rtcp_sr_timestamp = 0;
	m_ntp_timestamp = 0;
	m_rtcp_interval = std::chrono::milliseconds(5000);
	rtp_timeout = std::chrono::milliseconds(1000 / fps);
	timestamp_inc = 90000 / fps; // 90kHz clock

	//last_rtcp_message = clock::now();

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::cout.tie(nullptr);
}
RtspSession::~RtspSession() {
	
}
bool RtspSession::on_readable() {
	//last_active = clock::now();
	std::array<char, 4096> buffer;
	int read_size = stream.read(buffer);
	if(read_size <= 0) {
		// connection closed or error
		return false;
	}
	auto pr = parser.parse(std::span<const char>(buffer.data(), read_size));
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
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nPublic: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::DESCRIBE: {
				std::cout << " get DESCRIBE for " << req.url << std::endl;
				std::string rtsp_rep = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nContent-Base: " + req.url + "\r\nContent-Type: application/sdp\r\nContent-Length: ";
				std::string sdp_body = "v=0\r\no=- 0 0 IN IP4 0.0.0.0\r\ns=MyRtspStream\r\nt=0 0\r\na=control:*\r\n";
				sdp_body += "m=video 0 RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\n";
				sdp_body += "i=My live server\r\n";
				sdp_body += "c=IN IP4 0.0.0.0\r\n";
				//sdp_body += "a=fmtp:96 packetization-mode=1;profile-level-id=42A01E;sprop-parameter-sets=Z0IAH5WoFAFuQA==,aM4wpIA=\r\n";
				sdp_body += "a=fmtp:96 packetization-mode=1\r\n";
				sdp_body += "a=control:trackID=0\r\n";
				//sdp_body += "a=setup:rtp/avp/tcp\r\n";
				//sdp_body += "a=framerate:" + std::to_string(fps) + "\r\n";
				std::string response = rtsp_rep + std::to_string(sdp_body.size()) + "\r\n\r\n" + sdp_body;
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::SETUP:{
				if(req.headers["Transport"].find("RTP/AVP/TCP") == std::string::npos) {
					// only support RTP over RTSP (TCP)
					std::string response = "RTSP/1.0 461 Unsupported Transport\r\nCSeq: " + req.headers["CSeq"] + "\r\n\r\n";
					stream.write(std::span<const char>(response.data(), response.size()));
					break;
				}
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\nSession: 12345678\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				break;
			}
			case RtspMethod::PLAY: {
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nSession: 12345678\r\nRTP-Info: url=" + req.url + ";seq=" + std::to_string(rtp_seq) + ";rtptime=" + std::to_string(timestamp) + "\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				// send RTP packets here
				rtp_enabled = true;
				rtp_timeout = std::chrono::milliseconds(1000 / fps);
				
				last_rtcp_message = clock::now();
				m_ntp_timestamp = ntp64_now(std::chrono::system_clock::now());
				rtcp_message();

				rtp_time_out();
				break;
			}
			case RtspMethod::TEARDOWN: {
				std::string response = "RTSP/1.0 200 OK\r\nCSeq: " + req.headers["CSeq"] + "\r\nSession: 12345678\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				// close session
				rtp_enabled = false;
				return false;
				break;
			}
			default:
				// unhandled method
				std::string response = "RTSP/1.0 501 Not Implemented\r\nCSeq: " + req.headers["CSeq"] + "\r\n\r\n";
				stream.write(std::span<const char>(response.data(), response.size()));
				rtp_enabled = false;
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
bool RtspSession::rtp_time_out() {
	auto now = clock::now();
	next_rtp_timeout = now + rtp_timeout;
	m_ntp_timestamp = ntp64_now(std::chrono::system_clock::now());
	
	// send rtp interleaved packet
	RtpHeader rtp_header;
	rtp_header_init(rtp_header);
	//rtp_header_set_seq(rtp_header, rtp_seq++);
	rtp_header_set_timestamp(rtp_header, timestamp);
	rtcp_sr_timestamp = timestamp;
	rtp_header_set_ssrc(rtp_header, 0x12345678);
	// rtp payload

	bool is_marker;
	do {
		//int frame_size = 1000000;
		//auto* nalu_buf = new uint8_t[frame_size];
		int nalu_size = get_frame_from_file(m_nalu_buf, m_nalu_buf_size);
		//std::cout << "Got NALU of size: " << nalu_size << std::endl;
		if (nalu_size <= 0) {
			//delete[] nalu_buf;
			return false;
		}
		if (!rtp_send_H264_Frame(rtp_header, m_nalu_buf, nalu_size, is_marker)) {
			//delete[] nalu_buf;
			return false;
		}
		//delete[] nalu_buf;
	} while (!is_marker);


	if (now - last_rtcp_message > m_rtcp_interval) {
		last_rtcp_message = now;
		rtcp_message();
	}

	timestamp += timestamp_inc;
	//std::cout << "Sent RTP packet, seq=" << rtp_seq - 1 << std::endl;
	return true;
}
int RtspSession::get_frame_from_file(uint8_t* nalu_buf, size_t buf_size) {
	if (!h264_file.is_open() || h264_file.eof()) {
		//throw std::runtime_error("H264 file not open or EOF reached");
		return -1;
	}
	int H264_start_code_size;
	//nalu_buf[0] = 123;
	h264_file.read(reinterpret_cast<char*>(nalu_buf), buf_size);

	//std::cout << "Read file header: " << std::hex 
	//	<< int(nalu_buf[0]) << " " << int(nalu_buf[1]) << " " << int(nalu_buf[2]) << " " << int(nalu_buf[3]) << std::dec << std::endl;
	if(nalu_buf[0] == 0 && nalu_buf[1] == 0 && nalu_buf[2] == 0 && nalu_buf[3] == 1) {
		H264_start_code_size = 4;
	} else if(nalu_buf[0] == 0 && nalu_buf[1] == 0 && nalu_buf[2] == 1) {
		H264_start_code_size = 3;
	} else {
		// invalid start code
		return -1;
	}
	size_t nalu_size = 0;
	for (nalu_size = H264_start_code_size; nalu_size < buf_size - 4; nalu_size++) {
		if (nalu_buf[nalu_size] == 0 && nalu_buf[nalu_size + 1] == 0 && ((nalu_buf[nalu_size + 2] == 1) || (nalu_buf[nalu_size + 2] == 0 && nalu_buf[nalu_size + 3] == 1))) {
			break;
		}
	}
	h264_file.seekg(nalu_size - buf_size, std::ios::cur);
	return nalu_size;
}

bool RtspSession::rtp_send_H264_Frame(const struct RtpHeader& rtp_header, const uint8_t* nalu_buf, size_t nalu_size, bool& is_marker) {
	if (!rtp_enabled) {
		return false;
	}
	//skip header
	if(nalu_buf[0] == 0 && nalu_buf[1] == 0 && nalu_buf[2] == 0 && nalu_buf[3] == 1) {
		// 4 bytes start code
		nalu_buf += 4;
		nalu_size -= 4;
	} else if(nalu_buf[0] == 0 && nalu_buf[1] == 0 && nalu_buf[2] == 1) {
		// 3 bytes start code
		nalu_buf += 3;
		nalu_size -= 3;
	} else {
		// invalid start code
		return false;
	}
	uint8_t nal_unit_type = nalu_buf[0] & 0x1F;
	if(nal_unit_type == 7 || nal_unit_type == 8 || nal_unit_type >= 6) {
		//timestamp -= timestamp_inc;
		is_marker = false;
	}
	else {
		is_marker = true;
	}
	//std::cout << "Sending H264 NALU of size: " << nalu_size << std::endl;
	if (nalu_size < 1400) {
		rtp_octet_count += nalu_size;

		RtpHeader frag_rtp_header = rtp_header;
		std::array<uint8_t, 1500> rtp_packet;
		rtp_packet[0] = '$';
		rtp_packet[1] = 0; // channel 0 for RTP
		uint16_t rtp_packet_size = htons(uint16_t(nalu_size + sizeof(RtpHeader)));
		
		if (is_marker == false) {
			// set marker to 0
			rtp_header_set_marker(frag_rtp_header, false);
		}
		else {
			rtp_header_set_marker(frag_rtp_header, true);
		};
		rtp_header_set_seq(frag_rtp_header, rtp_seq++);
		std::memcpy(&rtp_packet[2], &rtp_packet_size, 2);
		std::memcpy(&rtp_packet[4], &frag_rtp_header, sizeof(RtpHeader));
		std::memcpy(&rtp_packet[4 + sizeof(RtpHeader)], nalu_buf, nalu_size);
		return stream.write(std::span<const char>(reinterpret_cast<const char*>(rtp_packet.data()), nalu_size + sizeof(RtpHeader) + 4));
	}
	else {
		// need to fragment
		size_t offset = 1; // skip NALU header
		uint8_t nal_unit_type = nalu_buf[0] & 0x1F;
		uint8_t nal_ref_idc = (nalu_buf[0] & 0x60) >> 5;
		bool first_fragment = true;
		bool last_fragment = false;
		while (offset < nalu_size) {
			size_t fragment_size = (std::min)(size_t(1400 - sizeof(RtpHeader) - 2), nalu_size - offset);
			last_fragment = (offset + fragment_size >= nalu_size);

			rtp_octet_count += fragment_size + 2;

			std::array<uint8_t, 1500> rtp_packet;
			rtp_packet[0] = '$';
			rtp_packet[1] = 0; // channel 0 for RTP
			uint16_t rtp_packet_size = htons(uint16_t(fragment_size + sizeof(RtpHeader) + 2));
			std::memcpy(&rtp_packet[2], &rtp_packet_size, 2);
			RtpHeader frag_rtp_header = rtp_header;
			
			if (last_fragment && is_marker) {
				rtp_header_set_marker(frag_rtp_header, true);
			}
			else {
				rtp_header_set_marker(frag_rtp_header, false);
			}
			rtp_header_set_seq(frag_rtp_header, rtp_seq++);
			std::memcpy(&rtp_packet[4], &frag_rtp_header, sizeof(RtpHeader));
			// FU-A header
			rtp_packet[4 + sizeof(RtpHeader)] = (nal_ref_idc << 5) | 28; // FU-A NALU type
			rtp_packet[4 + sizeof(RtpHeader) + 1] = (first_fragment ? 0x80 : 0x00) | (last_fragment ? 0x40 : 0x00) | nal_unit_type;
			std::memcpy(&rtp_packet[4 + sizeof(RtpHeader) + 2], nalu_buf + offset, fragment_size);
			if (!stream.write(std::span<const char>(reinterpret_cast<const char*>(rtp_packet.data()), fragment_size + sizeof(RtpHeader) + 2 + 4))) {
				return false;
			}

			offset += fragment_size;
			first_fragment = false;
		}
		return true;
	}
}
void RtspSession::rtcp_message() {
	//return;
	std::cout << "Sending RTCP SR packet" << std::endl;
	RtcpSr rtcp_packet;
	rtcp_sr_init(rtcp_packet, 0x12345678, rtcp_sr_timestamp, uint32_t(m_ntp_timestamp >> 32), uint32_t(m_ntp_timestamp & 0xFFFFFFFF), rtp_seq - 1, rtp_octet_count);
	std::array<uint8_t, 1500> rtcp_buf;
	rtcp_buf[0] = '$';
	rtcp_buf[1] = 1; // channel 1 for RTCP
	uint16_t rtcp_packet_size = htons(uint16_t(sizeof(RtcpSr)));
	std::memcpy(&rtcp_buf[2], &rtcp_packet_size, 2);
	std::memcpy(&rtcp_buf[4], &rtcp_packet, sizeof(RtcpSr));
	stream.write(std::span<const char>(reinterpret_cast<const char*>(rtcp_buf.data()), sizeof(RtcpSr) + 4));

}

uint64_t RtspSession::ntp64_now(std::chrono::system_clock::time_point now) {
	using namespace std::chrono;
	//auto now = system_clock::now();
	auto duration = now.time_since_epoch();
	uint64_t sec = duration_cast<seconds>(duration).count() + 2208988800ULL; // NTP epoch starts in 1900, Unix in 1970
	uint64_t frac = ((duration_cast<nanoseconds>(duration).count() % 1000000000) * 0x100000000ULL) / 1000000000;
	return (sec << 32) | frac;

}
