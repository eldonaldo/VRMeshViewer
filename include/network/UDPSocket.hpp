#pragma once

#include "common.hpp"

VR_NAMESPACE_BEGIN

/**
 *
 */
class UDPSocket {
public:
	UDPSocket (asio::io_service &io_service, int port);
	virtual ~UDPSocket ();


	void receive();
	void send(std::size_t length);
protected:

	asio::ip::udp::socket socket;
	asio::ip::udp::endpoint senderEndpoint;
	enum {
		maxLength = 1024
	};
	char data[maxLength];
	int port;
};

VR_NAMESPACE_END
