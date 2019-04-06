/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server3 server3.cpp libpalindrome.c -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server3 <numer portu>
 */

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <cctype>
#include <array>
// usunąłem wypisywanie na ekran w libpalindrome, bo tylko przeszkadzało
#include "libpalindrome.h"

namespace asio = boost::asio;
namespace ip = asio::ip;

void print_error(const std::string msg, const boost::system::error_code& err)
{
    if(err == asio::error::operation_aborted) return;
    std::cerr << msg << "(): " << err.message() << std::endl;
}

// funkcja sprawdzająca, czy tablica charów dowolnego rozmiaru zawiera jedynie cyfry
template<std::size_t N>
bool is_numeric_sequence(const std::array<char, N>& message)
{
    for(auto& i : message)
        if(i == '\0') break;
        else if(!std::isdigit(i)) if(!std::isspace(i)) return false;
    return true;
}

// funkcja zwracająca odpowiedź serwera na wiadomość klienta
template<std::size_t N>
std::string get_response(const std::array<char, N>& message, std::size_t bytes)
{
    if(message.empty()) return std::string("");

    if(is_numeric_sequence(message))
    {
        if(is_palindrome(const_cast<char*>(message.data()), bytes)) return std::string("IS PALINDROME");
        else return std::string("IS NOT PALINDROME");
    }

    return std::string("IS NOT NUMERIC SEQUENCE");
}

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        std::cout << "Invocation: " << argv[0] << " <PORT>" << std::endl;
        return 1;
    }

    constexpr int SIZE = 256;   //rozmiar maksymalnej wiadomości
    asio::io_context context;

    // utworzenie gniazda UDP i zbindowanie go
    ip::udp::socket socket(context, 
            ip::udp::endpoint(ip::udp::v4(), boost::lexical_cast<unsigned short>(argv[1])));

    std::cout << "Server is listening for incoming connection..." << std::endl;

    while(true)
    {
        std::array<char, SIZE> message;
        std::string response;
        boost::system::error_code err;
        // punkt końcowy dla komunikacji, zostanie uzupełniony po odebraniu danych za pomocą receive_from
        ip::udp::endpoint remote;

        // odbiór wiadomości od klienta
        std::size_t bytes = socket.receive_from(asio::buffer(message), remote, 0, err);
        message[bytes] = '\0';

        if(err) print_error("read_some", err);

        if(bytes)
        {
            std::cout << "From " << remote.address() << ":" << remote.port()
                    << " message: ";

            std::cout << message.data() << std::endl;
        }
            
        response.clear();
        response = get_response(message, bytes);
        socket.send_to(asio::buffer(response), remote, 0, err);

        if(err) print_error("send_to", err);

        // koniec komunikacji
        if(!bytes)
        {
            socket.close();
            break;
        }
    }

    return 0;
}
