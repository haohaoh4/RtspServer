#pragma once
#include <winsock2.h>
#include <cstdint>
#include <cstring>

struct RtpHeader {
	uint8_t vpxcc;       // Version(2), Padding(0), Extension(0), CSRC Count(0)
	uint8_t mpt;        // Marker(0), Payload Type(96)
	uint16_t seq;       // Sequence Number
	uint32_t timestamp; // Timestamp
	uint32_t ssrc;      // Synchronization Source
};
static_assert(sizeof(RtpHeader) == 12, "RtpHeader size is not 12 bytes");
inline void rtp_header_init(RtpHeader& header) {
	std::memset(&header, 0, sizeof(RtpHeader));
	header.vpxcc = (2 << 6); // Version 2
	header.mpt = 96;         // Payload Type 96
	header.seq = htons(0);
	header.timestamp = htonl(0);
	header.ssrc = htonl(0);
}
inline void rtp_header_set_seq(RtpHeader& header, uint16_t seq) {
	header.seq = htons(seq);
}
inline void rtp_header_set_timestamp(RtpHeader& header, uint32_t timestamp) {
	header.timestamp = htonl(timestamp);
}
inline void rtp_header_set_ssrc(RtpHeader& header, uint32_t ssrc) {
	header.ssrc = htonl(ssrc);
}
inline void rtp_header_set_marker(RtpHeader& header, bool marker) {
	if (marker) {
		header.mpt |= 0x80;
	}
	else {
		header.mpt &= 0x7F;
	}
}
inline void rtp_header_set_payload_type(RtpHeader& header, uint8_t payload_type) {
	header.mpt = (header.mpt & 0x80) | (payload_type & 0x7F);
}

struct RtcpHeader {
	uint8_t vpxrc;       // Version(2), Padding(0), Reception Report Count(0)
	uint8_t pt;         // Packet Type (200 for SR, 201 for RR, 202 for SDES, 203 for BYE, 204 for APP)
	uint16_t length;    // Length in words - 1
};
struct RtcpSr {
	RtcpHeader header;
	uint32_t ssrc;          // Sender SSRC
	uint32_t ntp_sec;      // NTP timestamp seconds
	uint32_t ntp_frac;     // NTP timestamp fraction
	uint32_t rtp_timestamp; // RTP timestamp
	uint32_t packet_count;  // Sender's packet count
	uint32_t octet_count;   // Sender's octet count
};
struct RtcpSdes {
	RtcpHeader header;
	uint32_t ssrc; // SSRC/CSRC
	// Followed by zero or more chunks of:
	// uint8_t type;
	// uint8_t length;
	// uint8_t value[length];
	// End with a zero byte
};
RtcpSr* rtcp_sr_init(RtcpSr& sr, uint32_t ssrc, uint32_t rtp_timestamp, uint32_t ntp_sec, uint32_t ntp_frac, uint32_t packet_count, uint32_t octet_count) {
	std::memset(&sr, 0, sizeof(RtcpSr));
	sr.header.vpxrc = (2 << 6); // Version 2
	sr.header.pt = 200;         // Packet Type 200 for SR
	sr.header.length = htons((sizeof(RtcpSr) / 4) - 1);
	sr.ssrc = htonl(ssrc);
	sr.ntp_sec = htonl(ntp_sec);
	sr.ntp_frac = htonl(ntp_frac);
	sr.rtp_timestamp = htonl(rtp_timestamp);
	sr.packet_count = htonl(packet_count);
	sr.octet_count = htonl(octet_count);
	return &sr;
}