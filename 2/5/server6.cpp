/*
 * Data:                2019-03-28
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server6 server6.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server6 <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <string>

namespace asio = boost::asio;
namespace ip = asio::ip;
using boost::system::error_code;

void error(const std::string& what, error_code err)
{
    if(err == asio::error::operation_aborted) return;

    std::cerr << what << ": " << err.message() << std::endl;
}

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        std::cout << "Invocation: " << argv[0] << " <PORT>" << std::endl;
        return 1;
    }

    // io_context jest niezbędnym elementem, który służy do komunikacji z systemem operacyjnym
    asio::io_context context;
    error_code err;

    // utworzenie gniazda dla protokolu TCP
    ip::tcp::socket socket(context);

    
    /* utworzenie akceptora połączeń
       pierwszym argumentem konstruktora jest io_context
       drugim argumentem jest punkt końcowy, który tworzymy w locie
       wywołanie konstruktora z dwoma parametrami od razu binduje akceptor z punktem końcowym
    */
    ip::tcp::acceptor acceptor(context, 
            ip::tcp::endpoint(ip::tcp::v6(), boost::lexical_cast<unsigned short>(argv[1])));

    acceptor.listen(2);

    std::cout << "Server is listening for incoming connection..." << std::endl;

    while(true)
    {
        // pobranie połączenia z kolejki połączeń oczekujących
        acceptor.accept(socket, err);

        if(err)
        {
            error("accept", err);
            socket.close();
            acceptor.close();
            return 1;
        }

        auto remote = socket.remote_endpoint();
        std::string buff = "Laboratorium PUS";
        std::string temp;

        if(remote.address().to_v6().is_v4_mapped())
            temp = "IPv4 mapped";
        else
            temp = "IPv6";

        std::cout << temp + " " << remote.address() << ":" << remote.port() << std::endl;

        asio::write(socket, asio::buffer(buff), err);
        
        if(err)
        {
            error("write", err);
            socket.close();
            acceptor.close();
            return 1;
        }

        socket.close();
    }

    return 0;
}
