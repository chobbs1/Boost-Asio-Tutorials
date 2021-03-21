#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;

std::string output_str()
{
    std::time_t now = std::time(0);
    return std::ctime(&now);
}

int main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::acceptor acceptor(io_service,tcp::endpoint(tcp::v4(),13));

        while(true) {
            tcp::socket socket(io_service);

            acceptor.accept(socket);

            std::string message = output_str();

            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    
    return 0;
}   