#include <network/UDPSocket.hpp>

VR_NAMESPACE_BEGIN

using asio::ip::udp;

UDPSocket::UDPSocket (asio::io_service& io_service, int port)
	: port(port), socket(io_service, udp::endpoint(udp::v4(), port)) {

}

void UDPSocket::receive() {
	cout << "receive" << endl;
	socket.async_receive_from(
		asio::buffer(data, maxLength), senderEndpoint, [this] (std::error_code errorCode, std::size_t bytesRecvd) {
		if (!errorCode && bytesRecvd > 0)
			cout << "asd" << endl;
		else
			receive();
		}
	);
}

void UDPSocket::send(std::size_t length) {
	cout << "send" << endl;
	socket.async_send_to(
		asio::buffer(data, length), senderEndpoint, [this] (std::error_code errorCode, std::size_t bytesSent) {
			receive();
		}
	);
}

UDPSocket::~UDPSocket () {
	socket.close();
}

VR_NAMESPACE_END
