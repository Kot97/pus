/*
 * Data:                2019-04-08
 * Autor:               Jakub Markowski
 * Kompilacja:          g++ ping.cpp -o ping.run -std=c++14 -lpthread
 *                      Zamiast g++ można użyć clang++
 *                      Do skompilowaniu programu potrzebna jest biblioteka Boost.Asio oraz Boost.LexicalCast
 * Uruchamianie:        ./ping.run <adres IP lub nazwa domenowa>
 */


#include <boost/asio/generic/raw_protocol.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include "../header/icmp.hpp"
#include "../header/ipv4.hpp"
#include "../header/ipv6.hpp"
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/ip/unicast.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffer.hpp>

// #define SOURCE_PORT 12344
// #define SOURCE_ADDRESS "2a02:a31a:a03d:7580:7c97:5950:50f0:b1f0"
// należy zmienić interface na ten, którego się używa
// #define INTERFACE "wlp2s0"


namespace asio = boost::asio;
using boost::system::error_code;

void error(const std::string& what, error_code err)
{
    if(err == asio::error::operation_aborted) return;

    std::cerr << what << ": " << err.message() << std::endl;
}

//funkcja generująca losowe alfanumeryczne stringi
//https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

asio::io_context context;
asio::ip::icmp::socket receiving_socket(context);


// std::array<char, 256> buff;
asio::streambuf buff(256);
std::istream istr(&buff);
// asio::streambuf::mutable_buffers_type buffs = buff.prepare(256);
// std::ostream ostr(&buff);
asio::ip::icmp::endpoint remote;

void async_receive_from_handler(const error_code &err, size_t bytes)
{
    if(err)
    {
        error("receive_from", err);
        return;
    }

    // std::cout << "rcvfrom success" << std::flush << std::endl;
    std::cout << "Pong received: ";
    /* for(const auto& s: buff)
        std::cout << s << ' '; */

    buff.commit(256);
    if(remote.address().is_v6())
    {
        header::ipv6 hipv6;
        istr >> hipv6;
        in6_addr temp_daddr = hipv6.daddr();
        char daddr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &temp_daddr, daddr, INET6_ADDRSTRLEN);
        std::cout << "v6 " << "hop limit:" << hipv6.hlim() << " length:" << hipv6.length() << " destination address:" <<  daddr << " ";

    }
    else
    {
        header::ipv4 hipv4;
        
        // buff.consume(256);
        
        istr >> hipv4;
        // const struct iphdr hdr = hipv4.get();
        // uint8_t test = hipv4.ttl();
        // fprintf(stderr, "%u", test);
        std::cout << "v4 " << "hop limit:" << int(hipv4.ttl()) << " length:" << hipv4.length() << " destination address:" <<  header::address_to_string(hipv4.daddr()) << " ";

        
    }

    header::icmp hicmp;
    istr >> hicmp;

    std::cout << "type:" << hicmp.type() << " code:" << hicmp.code() << " id:" << hicmp.id() << " sequence:" << hicmp.sequence();
    
    buff.consume(256);
    std::cout << std::endl << std::flush;

    receiving_socket.async_receive_from(buff.prepare(256), remote, async_receive_from_handler);
}

//TODO: Dopisać akceptowanie wracających pongów
int main(int argc, char** argv) 
{
    if (argc != 2 && argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << "<HOSTNAME OR IP ADDRESS> <INTERFACE>(optional for link-local addresses" << std::endl;
        return 1;
    }

    header::icmp icmp_header;

    /* asio::io_context context; */
    error_code err;
    asio::ip::icmp::resolver resolver(context);
    asio::ip::icmp::endpoint endpoint;

    asio::ip::icmp::socket socket(context);
    /* asio::ip::icmp::socket receiving_socket(context); */

    


    
    



    auto result = resolver.resolve(argv[1], nullptr, err);
    if(err)
    {
        error("resolve", err);
        return 1;
    }

    for(auto&& i : result)
        /* if(i.endpoint().address().is_v6())  */endpoint = i.endpoint();

    socket.open(endpoint.protocol());

    asio::ip::unicast::hops ttl(64);
    socket.set_option(ttl);

    
    asio::ip::icmp::endpoint final_endpoint;
    if(endpoint.address().is_v6())
    {
        if(endpoint.address().to_v6().is_link_local())
        {
            final_endpoint = asio::ip::icmp::endpoint(asio::ip::make_address_v6(endpoint.address().to_v6().to_string()+"%"+argv[2]), boost::lexical_cast<unsigned short>(argv[2]));
        }
        else
        {
            final_endpoint = endpoint;
        }
        receiving_socket.open(asio::ip::icmp::v6());
        receiving_socket.bind(asio::ip::icmp::endpoint(asio::ip::icmp::v6(), 0));
    }
    else
    {
        final_endpoint = endpoint;
        receiving_socket.open(asio::ip::icmp::v4());
        receiving_socket.bind(asio::ip::icmp::endpoint(asio::ip::icmp::v4(), 0));
    }

    /* std::array<char, 256> buff;
    asio::ip::icmp::endpoint remote;
    receiving_socket.async_receive_from(asio::buffer(buff), remote, [&](const error_code &err, size_t bytes)
    {
        if(err)
        {
            error("async_receive_from", err);
            return;
        }

        receiving_socket.async_receive_from(asio::buffer(buff), remote, )
    }); */
    receiving_socket.async_receive_from(buff.prepare(256), remote, async_receive_from_handler);

    socket.connect(final_endpoint);
    

    
    for(int i = 1; i<5; i++)
    {
        header::icmp icmp_header;
        icmp_header.type(8);
        icmp_header.code(0);

        uint32_t pid = getpid();
        uint16_t lower_pid = (uint16_t) pid & 0x0000FFFF;

        icmp_header.id(htons(lower_pid));
        icmp_header.sequence(htons(i));

        std::string data = random_string(22);
        // std::string data("");

        icmp_header.compute_checksum(data.data(), data.length());
        // std::cerr << sizeof(data) << std::endl;



        asio::streambuf request_buffer;
        std::ostream os(&request_buffer);



        os << icmp_header << data;
        


        socket.send(request_buffer.data(), 0, err);
        if(err)
        {
            error("send_to", err);
            return 1;
        }
        
        
    }

    socket.close();

    context.run();

    receiving_socket.close();

    return 1;
}