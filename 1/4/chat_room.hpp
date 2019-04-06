#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <boost/asio/io_context.hpp>
#include <unordered_set>
#include <array>
#include "session.hpp"

// aktualnie uwzględniłem tylko jeden chat_room w obrębie serwera
// choć przy takiej implementacji rozszerzenie tego na więcej chat_room'ów jest banalnie proste

template<std::size_t message_size>
class chat_room
{
    // zbiór haszowany, w którym trzymam shared_ptr na aktywne sesje
    std::unordered_set< std::shared_ptr<session<message_size> > > _sessions;

public:
    // nakazuje kompilatorowi wygenerować domyślny konstruktor
    chat_room() = default;

    void join(const std::shared_ptr<session<message_size> >& user_session) { _sessions.insert(user_session); }
    void leave(const std::shared_ptr<session<message_size> >& user_session) { _sessions.erase(user_session); }

    // wysyła wiadomość do wszystkich klientów, poza tym, którego ID jest argumentem funkcji
    void send(const std::array<char, message_size>& message, std::size_t bytes, unsigned long id)
    {
        for(auto i : _sessions) if(i->get_id() != id) i->send(message, bytes);
    }

};


#endif // !CHAT_ROOM_HPP
