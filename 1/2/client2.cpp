/*
 * Data:                2019-03-17
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client2 client2.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client2 <adres IP> <numer portu> <wiadomość>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <iostream>

namespace asio = boost::asio;

int main(int argc, char** argv) 
{
    if (argc != 4) 
    {
        std::cerr << "Invocation: " << argv[0] << " <IPv4 ADDRESS> <PORT> <MESSAGE>" << std::endl;
        return 1;
    }

    // io_context jest niezbędnym elementem, który służy do komunikacji z systemem operacyjnym
    asio::io_context context;

    // utworzenie i otwarcie gniazda dla protokolu UDP
    asio::ip::udp::socket socket(context);
    socket.open(asio::ip::udp::v4());

    // utworzenie punktu końcowego komunikacji
    // pierwszym argumentem konstruktora jest adres IP, drugim numer portu
    asio::ip::udp::endpoint endpoint(asio::ip::make_address_v4(argv[1]), 
                                    boost::lexical_cast<unsigned short>(argv[2]));

    sleep(1);

    std::cout << "Sending message to " << argv[1] << ".\nWaiting for server response..." << std::endl;

    sleep(2);

    std::string message = argv[3];

    // wysłanie wiadomości
    socket.send_to(asio::buffer(message), endpoint);

    // odpowiedź serwera
    std::array<char, 256> response;
    socket.receive_from(asio::buffer(response), endpoint);

    std::cout << "Server response: " << response.data() << std::endl;

    socket.close();

    return 0;
}
