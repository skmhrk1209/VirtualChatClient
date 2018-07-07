#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include "utility.hpp"

class UDPSender {
   public:
    UDPSender(boost::asio::io_service &ioService,
              const boost::asio::ip::udp::endpoint &localPoint,
              const boost::asio::ip::udp::endpoint &remotePoint)
        : localEndpoint(localPoint),
          remoteEndpoint(remotePoint),
          socket(ioService) {
        socket.open(localEndpoint.protocol());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(localEndpoint);
    }

    template <typename... Args>
    void send(Args &&...);

    template <typename Function, typename... Args>
    void async_send(Function &&, Args &&...);

    boost::asio::ip::udp::endpoint localEndpoint;
    boost::asio::ip::udp::endpoint remoteEndpoint;

   private:
    boost::asio::ip::udp::socket socket;
};

template <typename... Args>
void UDPSender::send(Args &&... args) {
    std::stringstream sstream;

    auto index(0);

    static_cast<void>(std::initializer_list<int>{
        (sstream << (index++ ? " " : "") << args, 0)...});

    socket.send_to(boost::asio::buffer(sstream.str()), remoteEndpoint);
}

template <typename Function, typename... Args>
void UDPSender::async_send(Function &&handler, Args &&... args) {
    std::stringstream sstream;

    auto index(0);

    static_cast<void>(std::initializer_list<int>{
        (sstream << (index++ ? " " : "") << args, 0)...});

    socket.async_send_to(boost::asio::buffer(sstream.str()), remoteEndpoint,
                         handler);
}
