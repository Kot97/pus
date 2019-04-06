/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server2_async server2_async.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server2_async <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <string>
#include <array>
#include <thread>

namespace asio = boost::asio;
namespace ip = asio::ip;

void print_error(const std::string msg, const boost::system::error_code& err)
{
    if(err == asio::error::operation_aborted) return;
    std::cerr << msg << "(): " << err.message() << std::endl;
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

    // utworzenie gniazda UDP i zbindowanie go
    ip::udp::socket socket(context, 
            ip::udp::endpoint(ip::udp::v4(), boost::lexical_cast<unsigned short>(argv[1])));

    std::cout << "Server is listening for incoming connection..." << std::endl;

    // punkt końcowy dla komunikacji, zostanie uzupełniony po odebraniu danych za pomocą receive_from
    ip::udp::endpoint remote;

    // oczekiwanie na dane od klienta
    std::array<char, 256> buff;
    socket.async_receive_from(asio::buffer(buff), remote, [&](const boost::system::error_code& err, std::size_t bytes)
    {
        if(err)
        {
            print_error("receive_from", err);
            return;
        }

        // dzięki temu unikamy wypisywania "śmieci"
        buff[bytes] = '\0';

        std::cout << "UDP datagram received from " << remote.address() << ":" << remote.port()
             << ". Echoing message...\n" << buff.data() << std::endl;

        sleep(4);

        // wysłanie odpowiedzi
        socket.async_send_to(asio::buffer(buff), remote, [&](const boost::system::error_code& err, std::size_t bytes)
        {
            if(err)
            {
                print_error("send_to", err);
                return;
            }

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
