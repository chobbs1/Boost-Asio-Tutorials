#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <iostream>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

using message_handler = std::function<void (std::string)>;
using error_handler = std::function<void ()>;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::io_context& io_context) 
            : m_socket {io_context}
            , m_stream_buffer {}
        {
        }

        void Start(message_handler&& on_message, error_code&& on_error)
        {

            boost::asio::async_read_until(m_socket, m_stream_buffer, '\n',boost::bind(&Session::HandleInput,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

        }

        void HandleInput(boost::system::error_code error, std::size_t bytes_transferred)
        {
            std::istream is(&m_stream_buffer);
            std::cout << is.rdbuf();
        }

        void Post() 
        {
            
        }

    public:
        tcp::socket& GetSocket() {return m_socket;}

    private:
        tcp::socket m_socket;
        boost::asio::streambuf m_stream_buffer;
        std::queue<std::string> m_outgoing_messages;
        message_handler on_message;
        error_handler on_error;
};

class Server
{
    public:
        Server(boost::asio::io_context& io_context,int port)
            : m_io_context { io_context }
            , m_acceptor {m_io_context,tcp::endpoint(tcp::v4(),port)}
            , m_clients {}
        {

        }

        void AcceptAsync() 
        {
            std::shared_ptr<Session> new_session { std::make_shared<Session>(m_io_context) };
            m_acceptor.async_accept(new_session->GetSocket(),boost::bind(&Server::OnConnection,this,boost::placeholders::_1));
        }

        void OnConnection(std::shared_ptr<Session> new_session)
        {
            new_session->Start(Post,RemoveClient);
            AcceptAsync();
        }

        void Post()
        {

        }

        void RemoveClient()
        {

        }

    private:
        boost::asio::io_context& m_io_context;
        tcp::acceptor m_acceptor;
        std::vector<std::shared_ptr<Session>> m_clients;
};


int main()
{
    boost::asio::io_context io_context;
    Server server(io_context,15001);

    server.AcceptAsync();
    io_context.run();

    return 0;
}
