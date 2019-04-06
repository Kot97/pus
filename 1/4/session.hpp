#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <memory>
#include <iostream>
#include "chat_room.hpp"

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using boost::system::error_code;

template<std::size_t n> class chat_room;

// poniższe dziedziczenie jest niezbędne do zliczania referencji,
// dzięki któremu obiekty klasy będą istnieć tak długo, jak długo będzie praca do wykonania

template<std::size_t message_size>
class session : public std::enable_shared_from_this< session<message_size> >
{
    ip::tcp::socket _socket;
    std::array<char, message_size> _buffer;
    std::shared_ptr< chat_room<message_size> > _chat_room;
    unsigned long ID;
    static unsigned long free_ID;

    void error(const std::string& what, error_code err)
    {
        // operacja przerwana nie zgłasza błędu, kończy akceptacje nowych połączeń
        if(err == asio::error::operation_aborted) return;

        std::cerr << what << ": " << err.message() << std::endl;
    }

    void read_handler(error_code err, std::size_t size)
    {   
        // koniec połączenia
        if(err == asio::error::operation_aborted)
        {
            _socket.shutdown(ip::tcp::socket::shutdown_send, err);
            return;
        }

        if(err) error("read", err);

        // wysłanie informacji do wszystkich pozostałych podłączonych klientów
        _chat_room->send(_buffer, size, ID);

        // kontynuujemy odczytywanie danych wprowadzonych przez klienta
        _socket.async_read_some(asio::buffer(_buffer), 
        [self = this->shared_from_this()](error_code err, std::size_t bytes)
        {
            self->read_handler(err, bytes);
        });
    }

public:
    session(ip::tcp::socket socket, const std::shared_ptr< chat_room<message_size> >& room)
        : _socket(std::move(socket)), _chat_room(room) { ID = free_ID; free_ID++; }

    // destruktor wywołuje się, gdy licznik referencji shared_ptr osiągnie 0
    // w tym momencie klient opuszcza pokój
    ~session() { _chat_room->leave(this->shared_from_this()); }

    unsigned long get_id() const noexcept { return ID; }

    void run()
    {
        // klient dołącza do pokoju
        _chat_room->join(this->shared_from_this());

        // rozpoczęcie odczytywania danych od klienta
        // ważnym elementem jest this->shared_from_this(), ponieważ wywołanie to zwiększa licznik referencji o 1 
        _socket.async_read_some(asio::buffer(_buffer), 
        [self = this->shared_from_this()](error_code err, std::size_t bytes)
        {
            self->read_handler(err, bytes);
        });
    }

    void send(const std::array<char, message_size>& message, std::size_t bytes)
    {
        // wysłanie wiadomości do konkretnego klienta z pomocą socketu
        asio::async_write(_socket, asio::buffer(message, bytes), 
        [self = this->shared_from_this(), this, &message](error_code err, std::size_t bytes)
        {
            // operacja przerwana - koniec komunikacji
            if(err == asio::error::operation_aborted)
            {
                _socket.shutdown(ip::tcp::socket::shutdown_send, err);
                return;
            }

            // jeśli jest jakiś błąd to go wypisuje
            if(err) error("read", err);
        });
    }
};

// inicjalizacja statycznej zmiennej, której używam do generowania unikalnych numerów ID
template<std::size_t message_size> unsigned long session<message_size>::free_ID = 1;

#endif