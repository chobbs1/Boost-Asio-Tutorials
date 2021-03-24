#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

using error_code = boost::system::error_code;
namespace io = boost::asio;

io::io_context io_context;
io::high_resolution_timer timer(io_context);

void CalculateTime(error_code error);

auto now()
{
    return std::chrono::high_resolution_clock::now();
}

auto begin = now();

void AsyncWait()
{
    timer.expires_after(std::chrono::seconds(1));
    timer.async_wait(std::bind(CalculateTime,std::placeholders::_1));
}

void CalculateTime(error_code error)
{
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now() - begin).count();
    std::cout << "Timeout after: " << elapsed << std::endl;
    AsyncWait();
}

int main()
{
    AsyncWait();
    io_context.run();

    return 0;
}