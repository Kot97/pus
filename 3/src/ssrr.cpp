/*
 * Data:                2019-04-05
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o ssrr ssrr.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio
 * Uruchamianie:        ./ssrr <nazwa hosta lub adres IP>
 */

#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/asio/streambuf.hpp>
#include <iostream>
#include <string>
#include "../header/icmp.hpp"
#include "../header/options.hpp"

namespace asio = boost::asio;
using boost::system::error_code;

void error(const std::string& what, error_code err)
{
    if(err == asio::error::operation_aborted) return;

    std::cerr << what << ": " << err.message() << std::endl;
}

int main(int argc, char** argv) 
{
    if (argc != 2) 
    {
        std::cerr << "Invocation: " << argv[0] << "<HOSTNAME OR IP ADDRESS>" << std::endl;
        return 1;
    }


    // Opcje IP: NOP, SSRR, len, ptr, adr_IP_1, adr_IP_2, IP_docelowe
    unsigned char ip_options[16] = 
    {
        1, 0x89, 15, 4,
        192,0,2,1,
        195,136,186,1,
        213,172,178,41
    };

    asio::io_context context;
    error_code err;
    // ważne jest, by użyć resolvera dla protokołu icmp, w przeciwnym wypadku endpoint będzie niewłaściwy
    asio::ip::icmp::resolver resolver(context);
    asio::ip::icmp::endpoint endpoint;
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
    // ustawienie opcji ssrr
    socket.set_option(asio::ip::ssrr(ip_options), err);
    if(err)
    {
        error("set_option", err);
        return 1;
    }

    // utworzenie nagłówka icmp
    header::icmp icmp_header;

    // wypełnienie nagłówka icmp 
    icmp_header.type(ICMP_ECHO);
    icmp_header.code(0);
    icmp_header.id(htons(getpid()));
    icmp_header.sequence(htons((unsigned short)rand()));
    icmp_header.compute_checksum();

    // utworzenie strumienia z nagłówkiem do wysłania
    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << icmp_header;

    std::cout << "Sending ICMP Echo..." << std::endl;

    // wysłanie komunikatu ICMP Echo
    socket.send_to(request_buffer.data(), endpoint, 0, err);
    if(err)
    {
        error("send_to", err);
        return 1;
    }

    socket.close();
    return 0;
}
