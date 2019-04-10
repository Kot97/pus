/*
 * Data:                2019-04-05
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o udpv6 udpv6.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.LexicalCast
 * Uruchamianie:        sudo ./udpv6 <adres IP lub nazwa domenowa> <numer portu>
 */

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "../header/udp.hpp"
#include "../header/ipv6.hpp"
#include "../header/options.hpp"



#define SOURCE_PORT 12344
#define SOURCE_ADDRESS "2a02:a31a:a03d:7580:7c97:5950:50f0:b1f0"
// należy zmienić interface na ten, którego się używa
#define INTERFACE "wlp2s0"

namespace asio = boost::asio;
using boost::system::error_code;

void error(const std::string& what, error_code err)
{
    if(err == asio::error::operation_aborted) return;

    std::cerr << what << ": " << err.message() << std::endl;
}

int main(int argc, char** argv) 
{
    if (argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << "<HOSTNAME OR IP ADDRESS> <PORT>" << std::endl;
        return 1;
    }

    // nagłówek UDP
    header::udp udp_header;

    asio::io_context context;
    error_code err;
    asio::ip::udp::resolver resolver(context);
    asio::ip::udp::endpoint endpoint;

    // surowe gniazdo
    asio::generic::raw_protocol::socket socket(context);

    auto result = resolver.resolve(argv[1], nullptr, err);
    if(err)
    {
        error("resolve", err);
        return 1;
    }

    for(auto&& i : result)
        if(i.endpoint().address().is_v6()) endpoint = i.endpoint();

    socket.open(asio::generic::raw_protocol(endpoint.protocol().family(), endpoint.protocol().protocol()));


    socket.set_option(asio::ip::ipv6_checksum(), err);
    if(err)
    {
        error("set_option", err);
        return 1;
    }

    // wypełnienie pól nagłówka UDP

    // port źródłowy
    udp_header.source(boost::lexical_cast<unsigned short>(SOURCE_PORT));
    // port docelowy
    udp_header.dest(boost::lexical_cast<unsigned short>(argv[2]));

    // rozmiar nagłówka UDP i danych - w tym przypadku tylko nagłówka
    udp_header.len(udp_header.length());

    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << udp_header;

    std::string address_with_interface(endpoint.address().to_v6().to_string());
    address_with_interface += "%";
    address_with_interface += INTERFACE;

    std::cout << address_with_interface << std::endl;

    asio::ip::udp::endpoint endpoint_with_interface(asio::ip::make_address_v6(address_with_interface), 
                                                        boost::lexical_cast<unsigned short>(argv[2]));

    socket.connect(endpoint_with_interface, err);
    if(err)
    {
        error("connect", err);
        return 1;
    }

    std::cout << "Sending UDP..." << std::endl;

    // wysyłanie datagramów co 1 sekunde
    while(true)
    {
        socket.send(request_buffer.data(), 0, err);
        if(err)
        {
            error("send_to", err);
            return 1;
        }
        
        sleep(1);
    }

    socket.close();
    return 1;
}
