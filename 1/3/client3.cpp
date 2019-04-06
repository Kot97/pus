/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client3 client3.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client3 <adres IP> <numer portu>
 */

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <iostream>
#include <string>

namespace asio = boost::asio;

// funkcja wypisująca błąd na standardowe wyjście błędów
void print_error(const std::string msg, const boost::system::error_code& err)
{
    if(err == asio::error::operation_aborted) return;
    std::cerr << msg << "(): " << err.message() << std::endl;
}

int main(int argc, char** argv) 
{
    if (argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << " <IPv4 ADDRESS> <PORT>" << std::endl;
        return 1;
    }

    constexpr int SIZE = 256;   //rozmiar maksymalnej wiadomości
    asio::io_context context;

    // utworzenie i otwarcie gniazda dla protokolu UDP
    asio::ip::udp::socket socket(context);
    socket.open(asio::ip::udp::v4());

    // utworzenie punktu końcowego komunikacji
    asio::ip::udp::endpoint endpoint(asio::ip::make_address_v4(argv[1]), 
                                    boost::lexical_cast<unsigned short>(argv[2]));

    // skojarzenie gniazda z adresem
    socket.connect(endpoint);
    
    std::array<char, 256> response;

    while(true)
    {
        // pobranie wiadomości
        std::string message;
        message.clear();
        std::getline(std::cin, message);
        boost::system::error_code err;

        // wysłanie wiadomości do serwera
        socket.send(asio::buffer(message), 0, err);

        if(err) print_error("send", err);

        // odbiór odpowiedzi serwera
        std::size_t bytes = socket.receive(asio::buffer(response), 0, err);
        response[bytes] = '\0';

        // jeśli był jakiś błąd to go wypisujemy
        if(err) print_error("receive", err);

        // wiadomość pusta - koniec komunikacji
        if(message.empty())
        {
            std::cout << "End of connection" << std::endl;
            socket.close();
            break;
        }

        std::cout << response.data() << std::endl;
    }

    return 0;
}
