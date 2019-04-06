/*
 * Data:                2019-03-24
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o server5 server5.cpp -lpthread $BOOST_LIB/libboost_filesystem.a
 *                      $BOOST_LIB oznacza położenie bilbiotek, zazwyczaj jest to /usr/lib
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebne są biblioteki:
 *                               Boost.Asio, Boost.Beast, Boost.Lexical_Cast oraz Boost.Filesystem
 * Uruchamianie:        ./server5 <numer portu>
 */

#include "generate_index.hpp"
#include "listener.hpp"
#include "http_session.hpp"
#include <boost/lexical_cast.hpp>

int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        std::cerr << "Invocation: " << argv[0] << " <PORT>" << std::endl;
        return 1;
    }

    asio::io_context context;

    generate_index(fs::path("img"));

    ip::tcp::endpoint endpoint(ip::make_address_v4(address), 
                                boost::lexical_cast<unsigned short>(argv[1]));

    std::make_shared<listener>(context, endpoint) -> run();
    context.run();
    return 0;
}
