#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>
#include <iostream>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

using message_handler = std::function<void (std::string message)>;
using error_handler = std::function<void (size_t)>;

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::io_context& io_context) 
            : m_socket {io_context}
            , m_stream_buffer {}
        {
        }

        void Start(size_t client_id,message_handler&& on_message,error_handler&& on_error)
        {
            m_on_message = on_message;
            m_on_error = on_error;
            m_client_id = ++client_id;

            AsyncRead();
        }

        void AsyncRead()
        {
            boost::asio::async_read_until(m_socket, m_stream_buffer, '\n',boost::bind(&Session::OnRead,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

        }

        void OnRead(boost::system::error_code error, std::size_t bytes_transferred)
        {
            if(!error) {
                std::istream is(&m_stream_buffer);
                std::stringstream sstream;
                sstream << "Client " << m_client_id << ": " << is.rdbuf();
                std::cout << sstream.str();
                m_on_message(sstream.str());

                AsyncRead();
            } else {
                m_socket.close();
                m_on_error(m_client_id);
            }
        }

        void Post(std::string& message) 
        {
            bool idle = m_outgoing_messages.empty();
            m_outgoing_messages.push(message);

            if(idle)
            {
                AsyncWrite();
            }
        }

        void AsyncWrite()
        {
            io::async_write(m_socket,io::buffer(m_outgoing_messages.front()),std::bind(&Session::OnWrite,shared_from_this(),std::placeholders::_1,std::placeholders::_2));
        }

        void OnWrite(boost::system::error_code error, std::size_t bytes_transferred) {
            if(!error) {

                m_outgoing_messages.pop();

                if(!m_outgoing_messages.empty()) {
                    AsyncWrite();
                } 
            } else {
                m_socket.close();
                m_on_error(m_client_id);
            }
        }

    public:
        tcp::socket& GetSocket() {return m_socket;}

    private:
        tcp::socket m_socket;
        boost::asio::streambuf m_stream_buffer;
        std::queue<std::string> m_outgoing_messages;
        message_handler m_on_message;
        error_handler m_on_error;
        size_t m_client_id;
};

class Server
{
    public:
        Server(boost::asio::io_context& io_context,int port)
            : m_io_context { io_context }
            , m_acceptor {m_io_context,tcp::endpoint(tcp::v4(),port)}
            , m_clients {}
            , m_new_session {}
        {
        }

        void AcceptAsync() 
        {
            m_new_session = std::make_shared<Session>(m_io_context);
            m_acceptor.async_accept(m_new_session->GetSocket(),std::bind(&Server::OnConnection,this));
        }

        void OnConnection()
        {
            m_new_session->Start(m_clients.size(),std::bind(&Server::Post,this,std::placeholders::_1),std::bind(&Server::RemoveClient,this,std::placeholders::_1));
            m_clients.push_back(m_new_session);

            std::stringstream is;
            is << "Server: Received Client " << m_clients.size() << "\n\r";
            std::cout << is.str();

            Post(is.str());
            AcceptAsync();
        }

        void Post(std::string message)
        {
            for(auto &client : m_clients) {
                client->Post(message);
            }
        }

        void RemoveClient(size_t client_id)
        {
            m_clients.erase(m_clients.begin()+client_id,m_clients.begin()+client_id+1);

            std::stringstream ss;
            ss << "Client " << client_id << " has disconnected\n\r";
            Post(ss.str());
        }

        

    private:
        boost::asio::io_context& m_io_context;
        tcp::acceptor m_acceptor;
        std::vector<std::shared_ptr<Session>> m_clients;
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
