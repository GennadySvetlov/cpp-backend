#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "json_loader.h"
#include "model.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
using namespace std::literals;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
                                  bool keep_alive,
                                  std::string_view content_type = ContentType::APPLICATION_JSON);

const std::string validMapPath = "/api/v1/maps/";

bool IsMapRequest(std::string request);

bool IsMapsRequest(std::string request);

bool IsApiRequest(std::string request);

std::string GetMapId(std::string request);

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        const auto text_response = [&req](http::status status, std::string_view text) {
            return MakeStringResponse(status, text, req.version(), req.keep_alive());
        };
        if (req.method_string() == "GET"){
            std::string request = {req.target().begin(), req.target().end()};
            std::string response;

//            std::vector<std::string> strs;
//            boost::split(strs,request,boost::is_any_of("/"));

            try {
                if (IsMapsRequest(request)){
                    response = json_loader::SerializeMaps(game_);
                } else if (IsMapRequest(request)){
                    response = json_loader::SerializeMap(game_, GetMapId(request));
                    if (response == "mapNotFound")
                        throw "mapNotFound";
                } else if (IsApiRequest(request)){
                        throw "badRequest";
                }
                send(std::move(text_response(http::status::ok, response)));
            }  catch (const char* err) {
                if (err == "mapNotFound"sv)
                    send(std::move(text_response(http::status::not_found, json_loader::SerializeError(err))));
                if (err == "badRequest"sv)
                    send(std::move(text_response(http::status::bad_request, json_loader::SerializeError(err))));
            }

        } else{
            send(std::move(text_response(http::status::method_not_allowed, "Invalid method")));
        }
    }

private:
    model::Game& game_;
};

}  // namespace http_handler
