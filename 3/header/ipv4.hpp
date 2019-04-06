#ifndef IPV4_HEADER_HPP
#define IPV4_HEADER_HPP

#include <cstdint>
#include <netdb.h>
#include <netinet/ip.h>
#include "abstract.hpp"

/*
struct iphdr
  {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int version:4;
    unsigned int ihl:4;
#else
# error	"Please fix <bits/endian.h>"
#endif
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
    // The options start here.
  };
*/

namespace header
{
    class ipv4 : public abstract 
    {
    public:
        typedef struct iphdr header_type;

        ipv4() : _header{0} {}
        explicit ipv4(const header_type &iph) : _header(iph) {}
        
        uint8_t  version()  const { return _header.version;         }
        uint8_t  ihl()      const { return _header.ihl;             }
        uint8_t  tos()      const { return _header.tos;             }
        uint16_t tot_len()  const { return ntohs(_header.tot_len);  }
        uint16_t id()       const { return ntohs(_header.id);       }
        uint16_t frag_off() const { return ntohs(_header.frag_off); }
        uint8_t  ttl()      const { return _header.ttl;             }
        uint8_t  protocol() const { return _header.protocol;        }
        uint16_t check()    const { return ntohs(_header.check);    }
        uint32_t saddr()    const { return ntohl(_header.saddr);    }
        uint32_t daddr()    const { return ntohl(_header.daddr);    }
        
        void version(uint8_t version)    { _header.version = version;             }
        void ihl(uint8_t ihl)            { _header.ihl = ihl;    }
        void tos(uint8_t tos)            { _header.tos = tos; }
        void tot_len(uint16_t tot_len)   { _header.tot_len = htons(tot_len); }
        void id(uint16_t id)             { _header.id = htons(id); }
        void frag_off(uint16_t frag_off) { _header.frag_off = htons(frag_off); }
        void ttl(uint8_t ttl)            { _header.ttl = ttl; }
        void protocol(uint8_t protocol)  { _header.protocol = protocol; }
        void check(uint16_t check)       { _header.check = htons(check); }
        void saddr(uint32_t saddr)       { _header.saddr = htonl(saddr); }
        void daddr(uint32_t daddr)       { _header.daddr = htonl(daddr); }

        void compute_checksum() { check( checksum(reinterpret_cast<uint16_t*>(&_header), sizeof(iphdr))); }

        int length() const { return sizeof(_header); }
        char *get_header() { return reinterpret_cast<char*>(&_header); }
        const struct iphdr get() const { return _header; }
        
    private:
        header_type _header;
    }; 

}

 
#endif // IP_HEADER_HPP