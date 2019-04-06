/*
 * Data:                2019-03-27
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o ipaddr ipaddr.cpp
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio
 * Uruchamianie:        ./ipaddr <adres IP>
 */

#include <boost/asio/ip/address.hpp>
#include <iostream>

namespace asio = boost::asio;
namespace ip = asio::ip;
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
        std::cerr << "Invocation: " << argv[0] << " <IP ADDRESS>" << std::endl;
        return 1;
    }

    error_code err;

    // konwersja adresu IP do postaci zrozumiałej dla Boost.Asio
    auto address = ip::address::from_string(argv[1], err);

    if(err)
    {
        error("Conversion to ip address", err);
        return 1;
    }

    // konwersja adresu IP do std::string
    std::string string_address = address.to_string();

    std::string protocol = "unknown";       //unknown nigdy nie powinno się wypisać
    if(address.is_v4()) protocol = "IPv4";
    if(address.is_v6()) 
    {
        if(address.to_v6().is_v4_mapped()) protocol = "IPv4 mapped";
        else protocol = "IPv6";
    }

    std::cout << protocol << ": " << string_address << std::endl;

    return 0;
}