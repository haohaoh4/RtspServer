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
