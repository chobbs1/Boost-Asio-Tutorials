#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::io_context& io_context) 
            : m_socket {io_context}
            , m_stream_buffer {}
        {
        };

        void Start()
        {
            boost::asio::async_read_until(m_socket, m_stream_buffer, '\n', 
                boost::bind(&Session::HandleInput,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        };

        void HandleInput(error_code error, std::size_t bytes_transferred)
        {
            std::istream is(&m_stream_buffer);
            std::cout << is.rdbuf();
        }

    public:
        tcp::socket& GetSocket() {return m_socket;};

    private:
        tcp::socket m_socket;
        boost::asio::streambuf m_stream_buffer;
};

class Server
{
    public:
        Server(boost::asio::io_context& io_context,int port)
            : m_io_context { io_context }
            , m_acceptor {m_io_context,tcp::endpoint(tcp::v4(),port)}
            , m_new_session {}
        {

        };

        void AcceptAsync() {
            m_new_session = std::make_shared<Session>(m_io_context);
            m_acceptor.async_accept(m_new_session->GetSocket(),boost::bind(&Server::HandleConnection,this));
        };

        void HandleConnection()
        {
            m_new_session->Start();
            AcceptAsync();
        }

    private:
        boost::asio::io_context& m_io_context;
        tcp::acceptor m_acceptor;
        std::shared_ptr<Session> m_new_session;
};


int main()
{
    boost::asio::io_context io_context;
    Server server(io_context,15001);

    server.AcceptAsync();
    io_context.run();

    return 0;
}
