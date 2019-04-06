/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o client1_async client1_async.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./client1_async <adres IP> <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
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

    /* nawiązanie połączenia
       funkcja jest asynchroniczna, drugim argumentem jest akcja wykonywana po połączeniu (handler)
       zamiast zagnieżdzonych lambd można zadeklarować funkcję i przesłać ją jako handler, 
       lub jeszcze lepiej - całość zamknąć w klasie
       to jest jednak jedynie prosty przykład, więc wszystko dzieje się w mainie
    */
    socket.async_connect(endpoint, [&](const boost::system::error_code& err)
    {
        if(err)
        {
            print_error("connect", err);
            return;
        }

        sleep(3);

        std::cout << "After three-way handshake. Waiting for server response..." << std::endl;
        // odebranie danych - również asynchroniczne
        socket.async_read_some(asio::buffer(buff), [&](const boost::system::error_code& err, std::size_t bytes)
        {
            if(err)
            {
                print_error("read_some", err);
                return;
            }

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
