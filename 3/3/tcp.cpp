/*
 * Data:                2019-04-05
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o udp udp.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.LexicalCast
 * Uruchamianie:        ./tcp <adres IP lub nazwa domenowa> <numer portu>
 */

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "../header/tcp.hpp"
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
    asio::ip::tcp::resolver resolver(context);
    asio::ip::tcp::endpoint endpoint;

    // surowe gniazdo
    asio::generic::raw_protocol::socket socket(context);

    auto result = resolver.resolve(argv[1], nullptr, err);
    if(err)
    {
        error("resolve", err);
        return 1;
    }

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

    // utworzenie nagłówka tcp
    header::tcp tcp_header;

    // utworzenie nagłówka ip
    header::ipv4 ip_header;

    // wypełnienie pól nagłówka ip
    ip_header.version(4);
    ip_header.ihl(5);
    ip_header.tos(0);

    // długość (nagłówek ip + dane)
    ip_header.tot_len(ip_header.length() + tcp_header.length());

    // wypełniane przez jądro systemu, jeżeli podana wartość to zero
    ip_header.id(0);

    ip_header.frag_off(0);
    ip_header.ttl(64);

    // identyfikator enkapsulowanego protokolu
    ip_header.protocol(IPPROTO_TCP);

    // adres źródłowy ("Filled in when zero")
    ip_header.saddr(header::address_to_binary(SOURCE_ADDRESS));

    // adres docelowy
    ip_header.daddr(header::address_to_binary(endpoint.address().to_string()));

    // wypełnienie pól nagłówka TCP

    // port źródłowy
    tcp_header.source(boost::lexical_cast<unsigned short>(SOURCE_PORT));    //PERR
    // port docelowy
    tcp_header.dest(boost::lexical_cast<unsigned short>(argv[2]));

    tcp_header.seq(rand());
    tcp_header.doff(20/4);
    tcp_header.syn(true);
    tcp_header.window(header::tcp::DEFAULT_WINVAL);

    tcp_header.compute_checksum(ip_header.saddr(), ip_header.daddr()); 

    socket.connect(endpoint, err);
    if(err)
    {
        error("connect", err);
        return 1;
    }

    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << ip_header << tcp_header;

    std::cout << "Sending TCP..." << std::endl;

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
