#ifndef BOOST_ASIO_BONUS_USER_SOCKOPT
#define BOOST_ASIO_BONUS_USER_SOCKOPT

#include <netinet/in.h>
#include <sys/socket.h>

#include <boost/asio/detail/push_options.hpp>

namespace boost 
{
namespace asio 
{
namespace ip 
{
    class ip_hdrincl 
    {
    public:
        ip_hdrincl() : optval(1) {}
        ip_hdrincl(bool ov) : optval(ov ? 1 : 0) {}

        template<typename Protocol>
        int level(const Protocol &p) const { return IPPROTO_IP; }

        template<typename Protocol>
        int name(const Protocol &p)  const { return IP_HDRINCL; }

        template<typename Protocol>
        const void *data(const Protocol &p) const { return reinterpret_cast<const void*>(&optval); }

        template<typename Protocol>
        int size(const Protocol &p) const { return sizeof(optval); }
        
    private:
        int optval;
    };

    class ipv6_hdrincl 
    {
    public:
        ipv6_hdrincl() : optval(1) {}
        ipv6_hdrincl(bool ov) : optval(ov ? 1 : 0) {}

        template<typename Protocol>
        int level(const Protocol &p) const { return IPPROTO_IPV6; }

        template<typename Protocol>
        int name(const Protocol &p)  const { return IPV6_HDRINCL; }

        template<typename Protocol>
        const void *data(const Protocol &p) const { return reinterpret_cast<const void*>(&optval); }

        template<typename Protocol>
        int size(const Protocol &p) const { return sizeof(optval); }
        
    private:
        int optval;
    };

    class ssrr 
    {
    public:
        ssrr(unsigned char *ov) : optval(ov) {}

        template<typename Protocol>
        int level(const Protocol &p) const { return IPPROTO_IP; }

        template<typename Protocol>
        int name(const Protocol &p)  const { return IP_OPTIONS; }

        template<typename Protocol>
        const void *data(const Protocol &p) const { return reinterpret_cast<const void*>(optval); }

        template<typename Protocol>
        int size(const Protocol &p) const { return sizeof(unsigned char) * 16; }
        
    private:
        unsigned char *optval;
    };

    class ipv6_checksum 
    {
    public:
        // domyślnie offset dla UDP (6 bajtów)
        ipv6_checksum() : optval(6) {}
        ipv6_checksum(int offset) : optval(offset) {}

        template<typename Protocol>
        int level(const Protocol &p) const { return IPPROTO_IPV6; }

        template<typename Protocol>
        int name(const Protocol &p)  const { return IPV6_CHECKSUM; }

        template<typename Protocol>
        const void *data(const Protocol &p) const { return reinterpret_cast<const void*>(&optval); }

        template<typename Protocol>
        int size(const Protocol &p) const { return sizeof(optval); }
        
    private:
        int optval;
    };

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_HDRINCL_SOCKOPT
