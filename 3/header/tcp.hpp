#ifndef TCP_HEADER_HPP
#define TCP_HEADER_HPP

#include <cstdint>
#include <netdb.h>
#include <netinet/tcp.h>
#include "abstract.hpp"

/*
   struct tcphdr 
   {
    u_int16_t source;
    u_int16_t dest;
    u_int32_t seq;
    u_int32_t ack_seq;
    u_int16_t res1:4;
    u_int16_t doff:4;
    u_int16_t fin:1;
    u_int16_t syn:1;
    u_int16_t rst:1;
    u_int16_t psh:1;
    u_int16_t ack:1;
    u_int16_t urg:1;
    u_int16_t res2:2;
    u_int16_t window;
    u_int16_t check;
    u_int16_t urg_ptr;
   };
*/

//
//                      1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |          Source Port          |       Destination Port        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                        Sequence Number                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                    Acknowledgment Number                      |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |  Data |           |U|A|P|R|S|F|                               |
//  | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
//  |       |           |G|K|H|T|N|N|                               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |           Checksum            |         Urgent Pointer        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                    Options                    |    Padding    |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                             data                              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//

namespace header
{
    class tcp : public abstract 
    {
    public:
        enum { DEFAULT_WINVAL = 4096 };
        typedef struct tcphdr header_type;

        tcp() : _header{0} {}
        explicit tcp(const header_type &tcph) : _header(tcph) {}

        uint16_t source()  const { return ntohs(_header.source);  }
        uint16_t dest()    const { return ntohs(_header.dest);    }
        uint32_t seq()     const { return ntohl(_header.seq);     }
        uint32_t ack_seq() const { return ntohl(_header.ack_seq); }
        uint16_t res1()    const { return ntohs(_header.res1);    }
        uint16_t doff()    const { return ntohs(_header.doff);    }
        uint16_t fin()     const { return ntohs(_header.fin);     }
        uint16_t syn()     const { return ntohs(_header.syn);     }
        uint16_t rst()     const { return ntohs(_header.rst);     }
        uint16_t psh()     const { return ntohs(_header.psh);     }
        uint16_t ack()     const { return ntohs(_header.ack);     }
        uint16_t urg()     const { return ntohs(_header.urg);     }
        uint16_t res2()    const { return ntohs(_header.res2);    }
        uint16_t window()  const { return ntohs(_header.window);  }
        uint16_t check()   const { return ntohs(_header.check);   }
        uint16_t urg_ptr() const { return ntohs(_header.urg_ptr); }

        void source(uint16_t source)   { _header.source = htons(source);   }
        void dest(uint16_t dest)       { _header.dest = htons(dest);       }
        void seq(uint32_t seq)         { _header.seq = htonl(seq);         }
        void ack_seq(uint32_t ack_seq) { _header.ack_seq = htonl(ack_seq); }
        void res1(uint16_t res1)       { _header.res1 = htons(res1);       }
        void doff(uint16_t doff)       { _header.doff = htons(doff);       }
        void fin(bool fin)             { _header.fin = (fin) ? 1 : 0;      }
        void syn(bool syn)             { _header.syn = (syn) ? 1 : 0;      }
        void rst(bool rst)             { _header.rst = (rst) ? 1 : 0;      }
        void psh(bool psh)             { _header.psh = (psh) ? 1 : 0;      }
        void ack(bool ack)             { _header.ack = (ack) ? 1 : 0;      }
        void urg(uint16_t urg)         { _header.urg = htons(urg);         }
        void res2(uint16_t res2)       { _header.res2 = htons(res2);       }
        void window(uint16_t window)   { _header.window = htons(window);   }
        void check(uint16_t check)     { _header.check = htons(check);     }
        void urg_ptr(uint16_t urg_ptr) { _header.urg_ptr = htons(urg_ptr); }

        int length() const { return sizeof(_header); }
        char* get_header() { return reinterpret_cast<char*>(&_header); }
        const struct tcphdr& get() const { return _header; }

        void compute_checksum(uint32_t srcaddr, uint32_t destaddr) 
        {
            check(0);
            tcp_checksum tc = {{0}, {0}};
            tc.pseudo.ip_src   = htonl(srcaddr);
            tc.pseudo.ip_dst   = htonl(destaddr);
            tc.pseudo.unused   = 0;
            tc.pseudo.protocol = IPPROTO_TCP;
            tc.pseudo.length   = htons(sizeof(tcphdr));
            tc.tcphdr = _header;
            _header.check = ((checksum(reinterpret_cast<uint16_t*>(&tc), sizeof(struct tcp_checksum))));
        }

        void compute_checksum(const std::string &srcaddr, const std::string &destaddr) 
        {
            compute_checksum(address_to_binary(srcaddr), address_to_binary(destaddr));
        }

    private:
        struct phdr    
        {    
                uint32_t ip_src;
                uint32_t ip_dst;
                uint8_t unused;
                uint8_t  protocol;
                uint16_t length;
        };

        struct tcp_checksum 
        {
            phdr pseudo;
            header_type tcphdr;
        };
        
        header_type _header;
    };
}

#endif  // TCP_HEADER_HPP