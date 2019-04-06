/*
 * Data:                2019-03-18
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server1_async server1_async.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server1_async <numer portu>
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <chrono>
#include <string>
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

    // utworzenie gniazda dla protokolu TCP
    ip::tcp::socket socket(context);

    /* utworzenie akceptora połączeń
       pierwszym argumentem konstruktora jest io_context
       drugim argumentem jest punkt końcowy, który tworzymy w locie
       wywołanie konstruktora z dwoma parametrami od razu binduje akceptor z punktem końcowym
    */
    ip::tcp::acceptor acceptor(context, 
            ip::tcp::endpoint(ip::tcp::v4(), boost::lexical_cast<unsigned short>(argv[1])));

    acceptor.listen(2);

    std::cout << "Server is listening for incoming connection..." << std::endl;

    // pobranie połączenia z kolejki połączeń oczekujących
    acceptor.async_accept(socket, [&](const boost::system::error_code& err)
    {
        if(err)
        {
            print_error("accept", err);
            return;
        }

        auto remote = socket.remote_endpoint();

        std::cout << "TCP connection accepted from " << remote.address() << ":" << remote.port() << std::endl;

        sleep(6);

        std::cout << "Sending current date and time..." << std::endl;

        sleep(2);

        // zapisanie w buforze aktualnego czasu
        std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(p);
        std::string buff = std::ctime(&t);
        //konieczne do usunięcia \n i wstawienia końca (inaczej na końcu wiadomości są śmieci)
        buff.erase(buff.size()-1, 1).push_back('\0');

        // wysłanie aktualnego czasu do klienta
        asio::async_write(socket, asio::buffer(buff), [&](const boost::system::error_code& err, std::size_t bytes)
        {
            // zamknięcie połączenia przez klienta
            if(err == asio::error::operation_aborted)
            {
                sleep(4);
                std::cout << "Connection terminated by client " <<
                        "(received FIN, entering CLOSE_WAIT state on connected socked)..." << std::endl;
            }

            if(err)
            {
                print_error("write", err);
                return;
            }

            sleep(12);

            std::cout << "Closing connected socket (sending FIN to client)..." << std::endl;
            socket.close();

            sleep(5);

            std::cout << "Closing acceptor and terminating server..." << std::endl;
            acceptor.close();

            sleep(3);
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
