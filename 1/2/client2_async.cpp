/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client2_async client2_async.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client2_async <adres IP> <numer portu> <wiadomość>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/lexical_cast.hpp>
#include <array>
#include <iostream>
#include <string>
#include <thread>

namespace asio = boost::asio;

void print_error(const std::string msg, const boost::system::error_code& err)
{
    if(err == asio::error::operation_aborted) return;
    std::cerr << msg << "(): " << err.message() << std::endl;
}

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
    std::array<char, 256> response;

    // wysłanie wiadomości
    socket.async_send_to(asio::buffer(message), endpoint, [&](const boost::system::error_code& err, std::size_t bytes)
    {
        if(err)
        {
            print_error("send_to", err);
            return;
        }

        // odpowiedź serwera
        socket.async_receive_from(asio::buffer(response), endpoint, [&](const boost::system::error_code& err, std::size_t bytes)
        {
            if(err)
            {
                print_error("receive_from", err);
                return;
            }
            std::cout << "Server response: " << response.data() << std::endl;
            socket.close();
        });

    });

    // odpalenie obsługi handlerów na osobnym wątku
    std::thread thread([&](){ context.run(); });
    // jesli wolimy, by komunikacja działała w tle i nie chcemy czekać, możemy zrobić tu detach()

    // tutaj możemy robić co chcemy (równolegle do obsługi komunikacji)
    // ...

    // jeżeli nie użyliśmy detach()
    thread.join();

    return 0;
}
