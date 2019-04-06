#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <boost/asio/io_context.hpp>
#include "chat_room.hpp"

// poniższe dziedziczenie jest niezbędne do zliczania referencji,
// dzięki któremu obiekty klasy będą istnieć tak długo, jak długo będzie praca do wykonania

template<std::size_t message_size>
class listener : public std::enable_shared_from_this< listener<message_size> >
{
    ip::tcp::acceptor _acceptor;
    ip::tcp::socket _socket;
    std::shared_ptr< chat_room<message_size> > _chat_room;

    void error(const std::string& what, error_code err)
    {
        // operacja przerwana nie zgłasza błędu, kończy akceptacje nowych połączeń
        if(err == asio::error::operation_aborted) return;

        std::cerr << what << ": " << err.message() << std::endl;
    }

    void accept_handler(error_code err)
    {
        if(err) error("accept", err);
        // odpala dla połączenia nową sesję
        else std::make_shared< session<message_size> >(std::move(_socket), _chat_room)->run();

        // akceptuje nowe połączenie
        _acceptor.async_accept(_socket, 
        [self = this->shared_from_this()](error_code err)
        {
            self->accept_handler(err);
        });
    }

public:
    listener(asio::io_context& context, ip::tcp::endpoint& endpoint, const std::shared_ptr< chat_room<message_size> >& room)
        : _socket(context), _acceptor(context, endpoint), _chat_room(room) {}

    void run()
    {
        _acceptor.async_accept(_socket, 
        [self = this->shared_from_this()](error_code err)
        {
            self->accept_handler(err);
        });
    }
};

#endif