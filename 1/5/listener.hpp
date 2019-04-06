#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <thread>
#include "http_session.hpp"

class listener : public std::enable_shared_from_this<listener>
{
    ip::tcp::socket _socket;
    ip::tcp::acceptor _acceptor;

    void accept_handler(error_code err)
    {
        if(err) error("accept", err);
        // jeśli nie ma błędu odpalam na nowym wątku obsługę żądania HTTP
        else std::thread([this]()
        {
            std::make_shared<http_session>(std::move(_socket)) -> run();
        }).detach();

        _acceptor.async_accept(_socket, 
        [self = this->shared_from_this()](error_code err)
        {
            self->accept_handler(err);
        });
    }

public:
    listener(asio::io_context& context, ip::tcp::endpoint& endpoint)
        : _socket(context), _acceptor(context, endpoint) {}

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