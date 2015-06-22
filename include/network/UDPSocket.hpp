#pragma once

#include "common.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Async UDP operations
 */
class UDPSocket {

public:

	/**
	 * @brief Inits with an asio::io_service and a port number to listen
	 */
	UDPSocket(asio::io_service& io_service, short listen_port);
	virtual ~UDPSocket() = default;

	/**
	 * @brief Async receive
	 */
	void receive ();

	/**
	 * @brief Async send
	 */
	void send (const std::string &msg, const std::string &ip_address, short port);

	/**
	 * @brief Returns the current buffer content
	 */
	std::string getBufferContent ();

	/**
	* @brief Wether the buffer holds new data or not
	*/
	bool hasNewData();

protected:

	/**
	 * @brief Copies the first 'length' bytes of the string 'content' into the buffer
	 */
	void copyToBuffer (const std::string &content, std::size_t length);

	/**
	 * @brief Copies the string 'content' into the buffer
	 */
	std::size_t copyToBuffer (const std::string &content);

private:

	asio::ip::udp::socket socket; ///< UDP socket
	asio::ip::udp::endpoint endpoint; ///< UDP endpoint
	int max_length; ///< Max buffer length
	std::size_t current_length; ///< Length of the data the buffer currently holding
	char *data; ///< Buffer
	bool bufferChanged; ///< Flag if the buffer holds new data
};

VR_NAMESPACE_END
