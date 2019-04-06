#ifndef UDP_HEADER_HPP
#define UDP_HEADER_HPP

#include <cstdint>
#include <netdb.h>
#include <netinet/udp.h>
#include "abstract.hpp"

/*
   struct udphdr 
   {
      uint16_t source;
      uint16_t dest;
      uint16_t len;
      uint16_t check;
   };
*/

//
// 0               8               16                             31
// +-------------------------------+------------------------------+     
// |          source port          |       destination port       |
// +-------------------------------+------------------------------+
// |            length             |           checksum           |
// +-------------------------------+------------------------------+
//

namespace header
{
    class udp : public abstract 
    {
    public:
        typedef struct udphdr header_type;

        udp() : _header{0} {}
        explicit udp(const header_type &udph) : _header(udph) {}

        uint16_t source() const { return ntohs(_header.source); }
        uint16_t dest() const { return ntohs(_header.dest); }
        uint16_t check() const { return ntohs(_header.check); }
        uint16_t len() const { return ntohs(_header.len); }

        void source(uint16_t source) { _header.source = htons(source); }
        void dest(uint16_t dest) { _header.dest = htons(dest); }
        void check(uint16_t check) { _header.check = htons(check); }
        void len(uint16_t len) { _header.len = htons(len); }

        int length() const { return sizeof(_header); }
        char* get_header() { return reinterpret_cast<char*>(&_header); }
        const struct udphdr& get() const { return _header; }

        void compute_checksum(uint32_t srcaddr, uint32_t destaddr)
        {
            udp_checksum uc = {{0}, {0}};
            uc.pseudo.ip_src = htonl(srcaddr);
            uc.pseudo.ip_dest = htonl(destaddr);
            uc.pseudo.unused = 0;
            uc.pseudo.protocol = IPPROTO_UDP;
            uc.pseudo.length = htons(sizeof(udphdr));
            uc.udphdr = _header;
            _header.check = ((checksum(reinterpret_cast<uint16_t*>(&uc), sizeof(udp_checksum))));
        }

        void compute_checksum(in6_addr srcaddr, in6_addr dstaddr)
        {
            phdr6 pseudo = 
            {
                { srcaddr.__in6_u.__u6_addr32[0], srcaddr.__in6_u.__u6_addr32[1], srcaddr.__in6_u.__u6_addr32[2], srcaddr.__in6_u.__u6_addr32[3]}, 
                { dstaddr.__in6_u.__u6_addr32[0], dstaddr.__in6_u.__u6_addr32[1], dstaddr.__in6_u.__u6_addr32[2], dstaddr.__in6_u.__u6_addr32[3]},
                0, IPPROTO_UDP, htons(sizeof(udphdr)) 
            };
            udp_checksum6 uc = {pseudo, _header};
            _header.check = ((checksum(reinterpret_cast<uint16_t*>(&uc), sizeof(udp_checksum6))));
        }

        void compute_checksum(const std::string &srcaddr, const std::string &destaddr) 
        {
            compute_checksum(address_to_binary(srcaddr), address_to_binary(destaddr));
        }

    private:
        struct phdr 
        {
            uint32_t ip_src; 
            uint32_t ip_dest;
            uint8_t unused;
            uint8_t protocol;
            uint16_t length;
        };

        struct phdr6 
        {
            uint32_t ip_src[4]; 
            uint32_t ip_dest[4];
            uint8_t unused;
            uint8_t protocol;
            uint16_t length;
        };

        struct udp_checksum
        {
            phdr pseudo;
            header_type udphdr;
        };

        struct udp_checksum6
        {
            phdr6 pseudo;
            header_type udphdr;
        };

        header_type _header;
    };
}

#endif  // TCP_HEADER_HPP