#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

class UDPReceiver {
   public:
    UDPReceiver(boost::asio::io_service &ioService,
                const boost::asio::ip::udp::endpoint &localPoint)
        : localEndpoint(localPoint), socket(ioService) {
        socket.open(localEndpoint.protocol());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(localEndpoint);
    }

    template <typename... Args>
    void receive(Args &&...);

    template <typename Function, typename... Args>
    void async_receive(Function &&, Args &&...);

    static const int BufferSize = 1 << 16;
    boost::array<char, BufferSize> buffer;

    boost::asio::ip::udp::endpoint localEndpoint;
    boost::asio::ip::udp::endpoint remoteEndpoint;

   private:
    boost::asio::ip::udp::socket socket;
};

template <typename... Args>
void UDPReceiver::receive(Args &&...) {
    socket.receive_from(boost::asio::buffer(buffer), remoteEndpoint);
}

template <typename Function, typename... Args>
void UDPReceiver::async_receive(Function &&handler, Args &&...) {
    socket.async_receive_from(boost::asio::buffer(buffer), remoteEndpoint,
                              handler);
}
