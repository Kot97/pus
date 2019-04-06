/*
 * Data:                2019-03-17
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server2 server2.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server2 <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <string>
#include <array>

namespace asio = boost::asio;
namespace ip = asio::ip;

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        std::cout << "Invocation: " << argv[0] << " <PORT>" << std::endl;
        return 1;
    }

    // io_context jest niezbędnym elementem, który służy do komunikacji z systemem operacyjnym
    asio::io_context context;

    // utworzenie gniazda UDP i zbindowanie go
    ip::udp::socket socket(context, 
            ip::udp::endpoint(ip::udp::v4(), boost::lexical_cast<unsigned short>(argv[1])));

    std::cout << "Server is listening for incoming connection..." << std::endl;

    // punkt końcowy dla komunikacji, zostanie uzupełniony po odebraniu danych za pomocą receive_from
    ip::udp::endpoint remote;

    // oczekiwanie na dane od klienta
    std::array<char, 256> buff;
    std::size_t bytes = socket.receive_from(asio::buffer(buff), remote);
    // dzięki temu unikamy wypisywania "śmieci"
    buff[bytes] = '\0';

    std::cout << "UDP datagram received from " << remote.address() << ":" << remote.port()
             << ". Echoing message...\n" << buff.data() << std::endl;

    sleep(4);

    // wysłanie odpowiedzi
    socket.send_to(asio::buffer(buff), remote);

    socket.close();

    return 0;
}
