#ifndef HTTP_SESSION_HPP
#define HTTP_SESSION_HPP

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include "util.hpp"

namespace fs = boost::filesystem;

class http_session : public std::enable_shared_from_this<http_session>
{
    ip::tcp::socket _socket;
    beast::flat_buffer _buffer;
    http::request<http::empty_body> _request;

    void read_handler(error_code err, std::size_t bytes)
    {
        // zamknięcie połączenia
        if(err == http::error::end_of_stream)
        {
            _socket.shutdown(ip::tcp::socket::shutdown_send, err);
            return;
        }

        if(err) return error("http_session::read", err);

        error_code err2;
        // metoda get
        if(_request.method() == http::verb::get)
        {
            auto target = _request.target();
            auto type = mime_type(target);
            std::string path = path_cat(fs::current_path().string(), target);
            std::cout << "Path: " << path << std::endl;
            if(beast::iequals(type, "image/png") || beast::iequals(type, "image/jpeg")
                                                 || beast::iequals(type, "image/gif"))
            { // zapytanie dotyczy konkretnego pliku graficznego - prześlij ten plik
                if(!fs::exists(path))
                { // taki plik nie istnieje
                    std::cout << "File not exist" << std::endl;
                    // utworzenie odpowiedzi HTTP o pustym body
                    http::response<http::empty_body> response;
                    response.version(_request.version());
                    response.result(http::status::not_found);
                    response.set(http::field::server, "Beast");
                    // uzupełnienie reszty niezbędnych pól
                    response.prepare_payload();
                    // wysłanie odpowiedzi do klienta - funkcja synchroniczna, blokowanie do momentu skończenia wysyłania
                    http::write(_socket, response, err2);
                }
                else
                { // wysłanie żądanego pliku
                    std::cout << "Sending file" << std::endl;
                    // utworzenie odpowiedzi HTTP zawierającej plik
                    http::response<http::file_body> response;
                    response.version(_request.version());
                    response.result(http::status::ok);
                    response.set(http::field::server, "Beast");
                    // otwarcie pliku, który będzie wysłany jako odpowiedź
                    response.body().open(path.c_str(), beast::file_mode::read, err2);
                    if(err2) error("http_session::read_handler", err2);
                    // uzupełnienie reszty niezbędnych pól
                    response.prepare_payload();
                    // wysłanie odpowiedzi do klienta - funkcja synchroniczna, blokowanie do momentu skończenia wysyłania
                    http::write(_socket, response, err2);
                }
            }
            else
            { // zapytanie nie dotyczy konkretnego pliku graficznego
                if(!fs::exists(path))
                { // taki plik nie istnieje
                    std::cout << "File not exist" << std::endl;
                    // utworzenie odpowiedzi HTTP o pustym body
                    http::response<http::empty_body> response;
                    response.version(_request.version());
                    response.result(http::status::not_found);
                    response.set(http::field::server, "Beast");
                    // uzupełnienie reszty niezbędnych pól
                    response.prepare_payload();
                    // wysłanie odpowiedzi do klienta - funkcja synchroniczna, blokowanie do momentu skończenia wysyłania
                    http::write(_socket, response, err2);
                }
                else
                { // przesyłam index.html
                    std::cout << "Sending index.html" << std::endl;
                    // utworzenie odpowiedzi HTTP zawierającej index
                    http::response<http::file_body> response;
                    response.version(_request.version());
                    response.result(http::status::ok);
                    response.set(http::field::server, "Beast");
                    // otwarcie index.html, który zostanie wysłany jako odpowiedź
                    response.body().open("index.html", beast::file_mode::read, err2);
                    if(err2) error("http_session::read_handler", err2);
                    // uzupełnienie reszty niezbędnych pól
                    response.prepare_payload();
                    // wysłanie odpowiedzi do klienta - funkcja synchroniczna, blokowanie do momentu skończenia wysyłania
                    http::write(_socket, response, err2);
                }
            }
        }
        else
        { // serwer nie obsługuje innych metod
                    http::response<http::empty_body> response;
                    response.version(_request.version());
                    response.result(http::status::unknown);
                    response.set(http::field::server, "Beast");
                    // uzupełnienie reszty niezbędnych pól
                    response.prepare_payload();
                    // wysłanie odpowiedzi do klienta - funkcja synchroniczna, blokowanie do momentu skończenia wysyłania
                    http::write(_socket, response, err2);
        }

        http::async_read(_socket, _buffer, _request, 
        [self = this->shared_from_this()](error_code err, std::size_t bytes)
        {
            self->read_handler(err, bytes);
        });
    }

public:
    http_session(ip::tcp::socket socket) : _socket(std::move(socket)) {}
    void run()
    {
        http::async_read(_socket, _buffer, _request, 
        [self = this->shared_from_this()](error_code err, std::size_t bytes)
        {
            self->read_handler(err, bytes);
        });
    }
};

#endif