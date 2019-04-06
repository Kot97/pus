/*
 * Data:                2019-03-17
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client1 client1.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client1 <adres IP> <numer portu>
 */

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <iostream>

namespace asio = boost::asio;

int main(int argc, char** argv) 
{
    if (argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << " <IPv4 ADDRESS> <PORT>" << std::endl;
        return 1;
    }

    std::array<char, 256> buff;

    // io_context jest niezbędnym elementem, który służy do komunikacji z systemem operacyjnym
    asio::io_context context;

    // utworzenie i otwarcie gniazda dla protokolu TCP
    asio::ip::tcp::socket socket(context);
    socket.open(asio::ip::tcp::v4());

    // utworzenie punktu końcowego komunikacji
    // pierwszym argumentem konstruktora jest adres IP, drugim numer portu
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address_v4(argv[1]), 
                                    boost::lexical_cast<unsigned short>(argv[2]));

    // funkcja sleep() ułatwia obserwację three-way handshake w snifferze
    sleep(1);

    // nawiązanie połączenia
    socket.connect(endpoint);

    sleep(3);

    std::cout << "After three-way handshake. Waiting for server response..." << std::endl;

    // odebranie danych
    socket.read_some(asio::buffer(buff));
    sleep(1);
    std::cout << "Received server response: " << buff.data() << std::endl;

    sleep(4);

    // zamknięcie połączenia
    std::cout << "Closing socket (sending FIN to server)..." << std::endl;
    socket.close();

    sleep(9);

    // po zakonczeniu aplikacji, gniazdo przez okreslony czas (2 * MSL) bedzie w stanie TIME_WAIT
    std::cout << "Terminating application. After receiving FIN from server, "
                    << "TCP connection will go into TIME_WAIT state." << std::endl;

    sleep(4);

    return 0;
}
