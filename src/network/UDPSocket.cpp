#include "network/UDPSocket.hpp"

VR_NAMESPACE_BEGIN

using asio::ip::udp;

UDPSocket::UDPSocket (asio::io_service& io_service, short listen_port)
	: socket(io_service, udp::endpoint(udp::v4(), listen_port)), current_length(0), bufferChanged(true) {

	max_length = Settings::getInstance().NETWORK_BUFFER_SIZE;
	data = new char[max_length];
}

void UDPSocket::receive () {
	Settings::getInstance().NETWORK_LISTEN = false;
	socket.async_receive_from(asio::buffer(data, max_length), endpoint, [this] (std::error_code ec, std::size_t bytes_recvd) {
		if (!ec && bytes_recvd > 0) {
			current_length = bytes_recvd;
			bufferChanged = true;
		}
		
		Settings::getInstance().NETWORK_LISTEN = true;
	});
}

void UDPSocket::send (const std::string &msg, const std::string &ip_address, short port) {
	std::size_t length = copyToBuffer(msg);

	endpoint = udp::endpoint(asio::ip::address::from_string(ip_address), port);
	socket.async_send_to(asio::buffer(data, length), endpoint, [this] (std::error_code ec, std::size_t bytes_sent) {
		/*if (!ec && bytes_sent > 0) {
			std::cout << "Sent: '" << bytes_sent << "'\n";
		}*/
	});

	// Packet sent. Wait for the next change to send a packet
	Settings::getInstance().NETWORK_NEW_DATA = false;
}

bool UDPSocket::hasNewData () {
	return bufferChanged;
}

std::string UDPSocket::getBufferContent () {
	bufferChanged = false;
	return std::string(data, data + current_length);
}

void UDPSocket::copyToBuffer (const std::string &content, std::size_t length) {
	strncpy(data, content.c_str(), length);
	data[length] = 0;
}

std::size_t UDPSocket::copyToBuffer (const std::string &content) {
	std::size_t length = content.size() * sizeof(std::string::value_type);
	copyToBuffer(content, length);
	return length;
}

VR_NAMESPACE_END
