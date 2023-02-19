#include <iostream>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

namespace net = boost::asio;
using net::ip::tcp;

using namespace std::literals;

int main() {
    static const int port = 3333;

    net::io_context io_context;

    // используем конструктор tcp::v4 по умолчанию для адреса 0.0.0.0
    tcp::acceptor acceptor(io_context, tcp::endpoint(net::ip::make_address("127.0.0.1"), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    // ...
}
