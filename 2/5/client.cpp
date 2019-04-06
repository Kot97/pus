/*
 * Data:                2019-03-28
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client client.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client <adres IP> <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <array>

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
    if (argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << " <IP ADDRESS> <PORT>" << std::endl;
        return 1;
    }

    std::array<char, 256> buff;

    // io_context jest niezbędnym elementem, który służy do komunikacji z systemem operacyjnym
    asio::io_context context;
    error_code err;

    auto addr = asio::ip::make_address(argv[1]);

    // utworzenie i otwarcie gniazda dla protokolu TCP
    asio::ip::tcp::socket socket(context);
    if(addr.is_v4())
        socket.open(asio::ip::tcp::v4(), err);
    if(addr.is_v6())
        socket.open(asio::ip::tcp::v6(), err);

    if(err)
    {
        error("open", err);
        return 1;
    }

    // utworzenie punktu końcowego komunikacji
    // pierwszym argumentem konstruktora jest adres IP, drugim numer portu
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(argv[1]), 
                                    boost::lexical_cast<unsigned short>(argv[2]));

    // nawiązanie połączenia
    socket.connect(endpoint, err);

    if(err)
    {
        error("connect", err);
        return 1;
    }

    // odebranie danych
    std::size_t bytes = socket.read_some(asio::buffer(buff), err);
    buff[bytes] = '\0';

    if(err)
    {
        error("read_some", err);
        return 1;
    }
    
    std::cout << "Received server response: " << buff.data() << std::endl;

    socket.close();
    return 0;
}
