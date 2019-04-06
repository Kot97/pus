/*
 * Data:                2019-04-05
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o udpv6 udpv6.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.LexicalCast
 * Uruchamianie:        ./udpv6 <adres IP lub nazwa domenowa> <numer portu>
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

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "::ffff:192:0:2:1"
// należy zmienić interface na ten, którego się używa
#define INTERFACE "wwp0s20u2i1"

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

    // nagłówek IP
    header::ipv6 ip_header;

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
    socket.set_option(asio::ip::ipv6_hdrincl(), err);
    if(err)
    {
        error("set_option", err);
        return 1;
    }

    socket.set_option(asio::ip::ipv6_checksum(), err);
    if(err)
    {
        error("set_option", err);
        return 1;
    }

    // wypełnienie pól nagłówka IP
    ip_header.flow(6, 0, 0);

    // identyfikator enkapsulowanego protokolu
    ip_header.next(IPPROTO_UDP);

    // maksymalna liczba skoków
    ip_header.hlim(64);

    // rozmiar pakietu oprócz nagłówka
    ip_header.plen(udp_header.length());

    // adres źródłowy
    if( ip_header.saddr(SOURCE_ADDRESS) != 1 )
    {
        std::cerr << "Error during source IPv6 address convertion to in6_addr" << std::endl;
        return 1;
    }

    // adres docelowy
    if( ip_header.daddr(endpoint.address().to_string().c_str()) != 1)
    {
        std::cerr << "Error during destination IPv6 address convertion to in6_addr" << std::endl;
        return 1;
    }

    // wypełnienie pól nagłówka UDP

    // port źródłowy
    udp_header.source(boost::lexical_cast<unsigned short>(SOURCE_PORT));
    // port docelowy
    udp_header.dest(boost::lexical_cast<unsigned short>(argv[2]));

    // rozmiar nagłówka UDP i danych - w tym przypadku tylko nagłówka
    udp_header.len(udp_header.length());

    udp_header.compute_checksum(ip_header.saddr(), ip_header.daddr()); 

    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << ip_header << udp_header;

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
