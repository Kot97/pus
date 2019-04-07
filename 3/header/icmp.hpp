#ifndef ICMP_HEADER_HPP
#define ICMP_HEADER_HPP

#include "abstract.hpp"
#include <cstdint>
#include <netdb.h>
#include <netinet/ip_icmp.h>

/*
    struct icmphdr
    {
    uint8_t type;		
    uint8_t code;		
    uint16_t checksum;
    union
    {
        struct
        {
        uint16_t	id;
        uint16_t	sequence;
        } echo;		
        uint32_t	gateway;
        struct
        {
        uint16_t	__glibc_reserved;
        uint16_t	mtu;
        } frag;	
    } un;
    };
*/

//
// 0               8               16                             31
// +-------------------------------+------------------------------+     
// |     type      |     code      |           checksum           |
// +-------------------------------+------------------------------+
// |                             data                             |
// +-------------------------------+------------------------------+
//

namespace header
{
//TODO : reszta pól nagłówka
    class icmp : public abstract 
    {
    public:
        typedef struct icmphdr header_type;

        icmp() : _header{0} {}
        explicit icmp(const header_type &icmph) : _header(icmph) {}

        uint16_t type() const { return ntohs(_header.type); }
        uint16_t code() const { return ntohs(_header.code); }
        uint16_t check() const { return ntohs(_header.checksum); }
        uint16_t id() const { return ntohs(_header.un.echo.id); }
        uint16_t sequence() const { return ntohs(_header.un.echo.sequence); }
        uint16_t gateway() const { return ntohs(_header.un.gateway); }

        void type(uint16_t type) { _header.type = type; }
        void code(uint16_t code) { _header.code = code; }
        void check(uint16_t checksum) { _header.checksum = checksum; }
        void id(uint16_t id) { _header.un.echo.id = id; }
        void sequence(uint16_t sequence) { _header.un.echo.sequence = sequence; }
        void gateway(uint16_t gateway) { _header.un.gateway = gateway; }

        void compute_checksum() { check(checksum(reinterpret_cast<uint16_t*>(&_header), sizeof(icmphdr))); }

        int length() const { return sizeof(_header); }
        char* get_header() { return reinterpret_cast<char*>(&_header); }
        const struct icmphdr& get() const { return _header; }

    private:
        header_type _header;
    };
}

#endif  // TCP_HEADER_HPP