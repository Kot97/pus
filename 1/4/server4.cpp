/*
 * Data:                2019-03-19
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server4 server4.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./server4 <numer portu>
 */

#include <boost/lexical_cast.hpp>
#include "listener.hpp"

// rozmiar buforu na wiadomości
#define N 512

// w całym programie używam zliczania referencji (shared_ptr) w celu utrzymania obiektów przy życiu
// jest to konieczne w przypadku stosowania operacji asynchronicznych

int main(int argc, char const *argv[])
{
    if (argc != 2) 
    {
        std::cerr << "Invocation: " << argv[0] << " <PORT>" << std::endl;
        return 1;
    }

    // ponieważ serwer będzie działał w obrębie jednego komputera jako adres ustawiam loopback
    auto endpoint = ip::tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), 
                                        boost::lexical_cast<unsigned short>(argv[1]));

    asio::io_context context;

    // tworzę shared_ptr listenera i wywołuję jego konstruktor podając mu context,
    // punkt końcowy komunikacji (ten serwer) oraz tworzę shared_ptr chat_room
    std::make_shared< listener<N> >(context, endpoint, 
                                std::make_shared< chat_room<N> >())->run();

    // run() blokuje program tak długo, jak długo są do wykonania operacje asynchroniczne
    context.run();

    return 0;
}
