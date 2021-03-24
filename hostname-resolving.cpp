#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace io = boost::asio;
namespace ip = io::ip;
using tcp = ip::tcp;
using error_code = boost::system::error_code;

void static on_resolve(error_code& error, tcp::resolver::results_type results)
{
    for(tcp::endpoint const& endpoint : results) {
        std::cout << endpoint << std::endl;
    }
}

int main()
{
    io::io_context io_context;
    error_code error;

    tcp::resolver resolver(io_context);

    tcp::resolver::query query("google.com","80");
    tcp::resolver::results_type results = resolver.resolve(query,error);

    for(tcp::endpoint const& endpoint : results) {
        std::cout << endpoint << std::endl;
    }
    
    return 0;
}