/*
 * Data:                2019-04-05
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o udp udp.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.LexicalCast
 * Uruchamianie:        ./udp <adres IP lub nazwa domenowa> <numer portu>
 */

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "../header/udp.hpp"
#include "../header/ipv4.hpp"
#include "../header/options.hpp"

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.0.2.1"

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

    // interesuje nas tylko adres IPv4
    for(auto&& i : result)
        if(i.endpoint().address().is_v4()) endpoint = i.endpoint();

    // otwarcie gniazda z właściwym protokołem
    socket.open(asio::generic::raw_protocol(endpoint.protocol().family(), endpoint.protocol().protocol()));
    // ustawienie opcji hdrincl
    socket.set_option(asio::ip::ip_hdrincl(), err);
    if(err)
    {
        error("set_option", err);
        return 1;
    }

    // utworzenie nagłówka udp
    header::udp udp_header;

    // utworzenie nagłówka ipv4
    header::ipv4 ip_header;

    // wypełnienie nagłówka ip 
    ip_header.ihl(5);
    ip_header.version(4);
    ip_header.tos(0);

    // długość (nagłówek IP + dane)
    ip_header.tot_len(ip_header.length() + udp_header.length());

    // wypełniane przez jądro systemu, jeżeli podana wartość to zero
    ip_header.id(0);

    ip_header.frag_off(0);
    ip_header.ttl(255);

    // identyfikator enkapsulowanego protokolu
    ip_header.protocol(IPPROTO_UDP);

    // adres źródłowy ("Filled in when zero")
    ip_header.saddr(header::address_to_binary(SOURCE_ADDRESS));

    // adres docelowy
    ip_header.daddr(header::address_to_binary(endpoint.address().to_string()));

    // wypełnienie pól nagłówka UDP

    // port źródłowy
    udp_header.source(boost::lexical_cast<unsigned short>(SOURCE_PORT));    //PERR
    // port docelowy
    udp_header.dest(boost::lexical_cast<unsigned short>(argv[2]));

    // rozmiar nagłówka UDP i danych - w tym przypadku tylko nagłówka
    udp_header.len(udp_header.length());

    udp_header.compute_checksum(ip_header.saddr(), ip_header.daddr()); 

    std::cout << "Sending UDP..." << std::endl;

    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << ip_header << udp_header;

    // wysyłanie datagramów co 1 sekunde
    while(true)
    {
        socket.send_to(request_buffer.data(), endpoint, 0, err);
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
