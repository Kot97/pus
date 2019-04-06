/*
 * Data:                2019-03-27
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -std=c++14 -o hostname2ip hostname2ip.cpp -lpthread
 *                      Zamiast clang++ można użyć g++, ale nie jest to zalecane
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.Lexical_Cast
 * Uruchamianie:        ./hostname2ip <nazwa hosta>
 */

#include <iostream>
#include <string>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

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
        std::cerr << "Invocation: " << argv[0] << " <HOSTNAME>" << std::endl;
        return 1;
    }

    asio::io_context context;
    error_code err;
    ip::tcp::endpoint endpoint;
    std::string protocol;

    // zamiana nazwy hosta na adres ip
    ip::tcp::resolver resolver(context);
    auto result = resolver.resolve(argv[1], nullptr, err);
    if(err) 
    {
        error("resolve", err);
        return 1;    
    }
    
    for(auto&& i : result)
    {
        endpoint = i.endpoint();
        switch(endpoint.protocol().family())
        {
            case 2:
                protocol = "IPv4";
                break;
            case 10:
                protocol = "IPv6";
                break;
            default:
                protocol = "unknown";
        }
        std::cout << protocol << ": " << endpoint.address() << std::endl;
    }

    return 0;
}