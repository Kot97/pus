#ifndef IPV6_HEADER_HPP
#define IPV6_HEADER_HPP

#include <cstdint>
#include <netdb.h>
#include <netinet/ip6.h>
#include "abstract.hpp"

/*
struct ip6_hdr
  {
    union
      {
	struct ip6_hdrctl
	  {
	    uint32_t ip6_un1_flow;   
	    uint16_t ip6_un1_plen;  
	    uint8_t  ip6_un1_nxt;  
	    uint8_t  ip6_un1_hlim;
	  } ip6_un1;
	uint8_t ip6_un2_vfc;     
      } ip6_ctlun;
    struct in6_addr ip6_src;
    struct in6_addr ip6_dst;
  };
*/

namespace header
{
// TODO : extension headers
    class ipv6 : public abstract 
    {
    public:
        typedef struct ip6_hdr header_type;

        ipv6() : _header{0} {}
        explicit ipv6(const header_type &iph) : _header(iph) {}

        // flow = 4 bits version, 8 bits TC, 20 bits flow-ID
        //iphdr.ip6_flow = htonl ((6 << 28) | (0 << 20) | 0);

        uint8_t  version() const { return ((_header.ip6_ctlun.ip6_un1.ip6_un1_flow >> 28) & 0x04);    } // PERR
        uint8_t  tc()      const { return ((_header.ip6_ctlun.ip6_un1.ip6_un1_flow >> 20) & 0x0ff);   } // PERR
        uint32_t flow_id() const { return ntohl(_header.ip6_ctlun.ip6_un1.ip6_un1_flow & 0x000fffff); } // PERR

        uint32_t flow()    const { return ntohl(_header.ip6_ctlun.ip6_un1.ip6_un1_flow); }
        uint16_t plen()    const { return ntohs(_header.ip6_ctlun.ip6_un1.ip6_un1_plen); }
        uint8_t  next()    const { return _header.ip6_ctlun.ip6_un1.ip6_un1_nxt;         }
        uint8_t  hlim()    const { return _header.ip6_ctlun.ip6_un1.ip6_un1_hlim;        }
        in6_addr saddr()   const { return _header.ip6_src;                               }
        in6_addr daddr()   const { return _header.ip6_dst;                               }

        void flow(uint8_t version, uint8_t tc, uint32_t flow_id)
        {
            _header.ip6_ctlun.ip6_un1.ip6_un1_flow = htonl((version << 28) | (tc << 20) | flow_id);
        }

        void flow(uint32_t flow) { _header.ip6_ctlun.ip6_un1.ip6_un1_flow = htonl(flow); }
        void plen(uint16_t plen) { _header.ip6_ctlun.ip6_un1.ip6_un1_plen = htons(plen); }
        void next(uint8_t next)  { _header.ip6_ctlun.ip6_un1.ip6_un1_nxt = next;         }
        void hlim(uint8_t hlim)  { _header.ip6_ctlun.ip6_un1.ip6_un1_hlim = hlim;        }

        void saddr(const in6_addr& saddr) { _header.ip6_src = saddr; }
        void daddr(const in6_addr& daddr) { _header.ip6_dst = daddr; }
        
        int saddr(const char* saddr) { return inet_pton(AF_INET6, saddr, &_header.ip6_src); }
        int daddr(const char* daddr) { return inet_pton(AF_INET6, daddr, &_header.ip6_dst); }

        int length() const { return sizeof(_header); }
        char *get_header() { return reinterpret_cast<char*>(&_header); }
        const struct ip6_hdr get() const { return _header; }
        
    private:
        header_type _header;
    }; 
}

#endif