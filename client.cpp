#include <boost/asio.hpp>
#include <iostream>

namespace io = boost::asio;
namespace ip = io::ip;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

int main()
{
    // establish socket with io_context
    io::io_context m_io_context;
    tcp::socket m_socket(m_io_context);

    // make endpoint for socket to connect to
    ip::address m_address {ip::make_address("127.0.0.1")};
    tcp::endpoint m_endpoint(m_address,80);

    // Connect socket to endpoint
    error_code error;
    m_socket.connect(m_endpoint,error);

    if(!error) {
        std::cout << "Connection established" << std::endl;
    } else {
        std::cout << "Connection failed" << std::endl;
    }

    return 0;
}