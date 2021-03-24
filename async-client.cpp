#include <iostream>
#include <boost/asio.hpp>

namespace io = boost::asio;
namespace ip = io::ip;
using tcp = ip::tcp;
using error_code = boost::system::error_code;
using namespace std::placeholders;

class Application
{
    public:
        Application(io::io_context& io_context, std::string const& hostname)
            : m_resolver {io_context}
            , m_socket {io_context}
            , m_request {}
            , m_response {}
        {
            m_request = "GET / HTTP/1.1\n"
                  "Host: " + hostname + "\n"
                  "Connection: close\n\n";
            
            m_resolver.async_resolve(hostname,"http",std::bind(&Application::OnResolve,this,_1,_2));
        }

    private:
        void OnResolve(error_code error, tcp::resolver::results_type results)
        {
            std::cout << "OnResolve" << std::endl;
            io::async_connect(m_socket,results,std::bind(&Application::OnConnect,this,_1,_2));
        }

        void OnConnect(error_code error, tcp::endpoint const& endpoint)
        {
            std::cout << "OnConnect" << std::endl;
            io::async_write(m_socket,io::buffer(m_request),std::bind(&Application::OnWrite,this,_1,_2));
        }

        void OnWrite(error_code error, std::size_t bytes_transferred)
        {
            std::cout << "OnWrite" << std::endl;
            io::async_read(m_socket,m_response,std::bind(&Application::OnRead,this,_1,_2));
        }

        void OnRead(error_code error, std::size_t bytes_transferred)
        {
            std::cout << "OnRead" << std::endl;
            std::cout <<  std::istream(&m_response).rdbuf() << std::endl;  
        }

        void OnError() 
        {
            std::cout << "OnError" << std::endl;
            error_code error;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        }

        tcp::resolver m_resolver;
        tcp::socket m_socket;
        std::string m_request;
        io::streambuf m_response;
};

int main(int argc,char* argv[])
{
    io::io_context io_context;
    Application app(io_context,argv[1]);

    io_context.run();

    return 0;
}