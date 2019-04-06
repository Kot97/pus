#ifndef ABSTRACT_HEADER_HPP
#define ABSTRACT_HEADER_HPP

#include <iostream>
#include <string>
#include <boost/asio/ip/address.hpp>

namespace header
{
    class abstract 
    {
    public:
        abstract() {}
        virtual ~abstract() {}
        
        virtual int length() const = 0;
        virtual char *get_header() = 0;
        
        friend std::istream& operator>>(std::istream &is, abstract &header);
        friend std::ostream& operator<<(std::ostream &os, abstract &header);

    protected:
        virtual void prepare_to_read(std::istream&) {}
        virtual void prepare_to_write(std::ostream&) {}
    };

    static unsigned short checksum(unsigned short *buf, int bufsz) 
    {
        unsigned long sum = 0;
        while( bufsz > 1 ) 
        {
            sum += *buf++;
            bufsz -= 2;
        }
        if( bufsz == 1 ) sum += *(unsigned char *)buf;
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        return ~sum;
    }
        
    inline std::istream& operator>>(std::istream& is, abstract& header) 
    {
        header.prepare_to_read(is);
        return is.read(header.get_header(), header.length());
    }

    inline std::ostream& operator<<(std::ostream& os, abstract& header) 
    {
        header.prepare_to_write(os);
        return os.write(header.get_header(), header.length());
    }

    u_int32_t address_to_binary(const std::string &straddr) 
    {
        return boost::asio::ip::address_v4::from_string(straddr).to_ulong();
    } 
        
    std::string address_to_string(u_int32_t binaddr) 
    {
        return boost::asio::ip::address_v4(binaddr).to_string();
    }
}

#endif // abstract_HPP
