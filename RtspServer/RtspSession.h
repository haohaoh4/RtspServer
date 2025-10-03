#pragma once
#include "RtspParser.h"
#include "TcpStream.h"

class RtspSession
{
	RtspParser parser;
	TcpStream stream;
public:
	//SOCKET client_sock;
	RtspSession(SOCKET client_sock);
	~RtspSession() noexcept;
	bool on_readable();
	SOCKET getSocket() const;
};

