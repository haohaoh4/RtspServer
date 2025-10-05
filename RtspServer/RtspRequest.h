#pragma once
#include <string>
#include <map>

enum class RtspMethod{
	NONE = 0,
	OPTIONS,
	DESCRIBE,
	SETUP,
	PLAY,
	PAUSE,
	TEARDOWN,
	GET_PARAMETER,
	SET_PARAMETER,
	ANNOUNCE,
	RECORD,
	REDIRECT,
	UNKNOWN
};
// method text
inline RtspMethod get_rtsp_method(const std::string& method_str) {
	if (method_str == "OPTIONS") return RtspMethod::OPTIONS;
	if (method_str == "DESCRIBE") return RtspMethod::DESCRIBE;
	if (method_str == "SETUP") return RtspMethod::SETUP;
	if (method_str == "PLAY") return RtspMethod::PLAY;
	if (method_str == "PAUSE") return RtspMethod::PAUSE;
	if (method_str == "TEARDOWN") return RtspMethod::TEARDOWN;
	if (method_str == "GET_PARAMETER") return RtspMethod::GET_PARAMETER;
	if (method_str == "SET_PARAMETER") return RtspMethod::SET_PARAMETER;
	if (method_str == "ANNOUNCE") return RtspMethod::ANNOUNCE;
	if (method_str == "RECORD") return RtspMethod::RECORD;
	if (method_str == "REDIRECT") return RtspMethod::REDIRECT;
	return RtspMethod::UNKNOWN;
}

struct RtspRequest {
	RtspMethod method;
	std::string url;
	std::string version;
	std::map<std::string, std::string> headers;
};