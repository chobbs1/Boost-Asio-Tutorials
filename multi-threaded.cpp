#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

io::io_context io_context;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        void AsyncWrite()
        {
            io::async_write(m_socket,m_streambuf,io::bind_executor(m_write,boost::bind(&Session::OnWrite,shared_from_this(),boost::placeholders::_1,boost::placeholders::_2)));
        }

        void OnWrite(error_code error,std::size_t bytes_transferred)
        {
            if(!error) {
                AsyncWrite();
            }
        }

        void AsyncRead()
        {   
            io::async_read(m_socket,m_streambuf,io::bind_executor(m_write,boost::bind(&Session::OnRead,shared_from_this(),boost::placeholders::_1,boost::placeholders::_2)));

        }
        
        void OnRead(error_code error,std::size_t bytes_transferred)
        {
            AsyncWrite();
        }

    private:
        io::io_context::strand m_read;
        io::io_context::strand m_write;
        io::streambuf m_streambuf;
        tcp::socket m_socket;
};


void start_thread()
{
    io_context.run();
}

int main()
{
    std::vector<std::thread> threads;

    auto count {std::thread::hardware_concurrency() *2};
    std::cout << "Threads = " << count << std::endl;

    for(int i=0;i<count;i++) {
        threads.push_back(std::thread{start_thread});
    }

    for(auto &thread : threads) {
        if(thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}