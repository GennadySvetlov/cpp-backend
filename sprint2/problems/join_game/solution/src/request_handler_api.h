#ifndef REQUESTHANDLERAPI_H
#define REQUESTHANDLERAPI_H

#include "request_handler.h"

//namespace beast = boost::beast;
//namespace http = beast::http;

//using StringResponse = http::response<http::string_body>;

//const std::string validMapPath = "/api/v1/maps/";

//bool IsMapRequest(std::string request)
//{
//    return request.starts_with(validMapPath) && (request.size() > validMapPath.size());
//}

//std::string GetMapId(std::string request)
//{
//    return request.substr(validMapPath.size(), request.size()-validMapPath.size());
//}

//bool IsMapsRequest(std::string request)
//{
//    return (request == validMapPath.substr(0, validMapPath.size()-1));
//}

//bool IsApiRequest(std::string request)
//{
//    return (request.starts_with(validMapPath.substr(0, 5))); // request == "/api/..."
//}

//class RequestHandlerApi
//{
//public:
//    explicit RequestHandlerApi(model::Game& game)
//        : game_{game}{
//    }
//    template <typename Body, typename Allocator, typename Send>
//    Send operator()(http::request<Body, http::basic_fields<Allocator>>&& req, std::string request){
//        const auto text_response = [&req](http::status status, std::string_view text, std::string_view content_type) {
//            return MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
//        };
//        try {
//            std::string response;
//            if (IsMapsRequest(request)){
//                response = json_loader::SerializeMaps(game_);
//            } else if (IsMapRequest(request)){
//                response = json_loader::SerializeMap(game_, GetMapId(request));
//                if (response == "mapNotFound")
//                    throw "mapNotFound";
//            } else {
//                throw "badRequest";
//            }
//            return text_response(http::status::ok, response, http_handler::ContentType::APPLICATION_JSON);
//        }  catch (const char* err) {
//            if (err == "mapNotFound"sv)
//                return text_response(http::status::not_found, json_loader::SerializeError(err), http_handler::ContentType::APPLICATION_JSON);
//            if (err == "badRequest"sv)
//                return text_response(http::status::bad_request, json_loader::SerializeError(err), http_handler::ContentType::APPLICATION_JSON);
//        }
//    }
//private:
//    model::Game& game_;
//};

//class RequestHandlerAPI {
//public:
//    explicit RequestHandlerAPI(model::Game& game) : game_(game) {}

//    std::string HandleRequest(const std::string& request) {
//        std::string response;
//        try {
//            if (IsMapsRequest(request)) {
//                response = json_loader::SerializeMaps(game_);
//            } else if (IsMapRequest(request)) {
//                response = json_loader::SerializeMap(game_, GetMapId(request));
//                if (response == "mapNotFound")
//                    throw "mapNotFound";
//            } else {
//                throw "badRequest";
//            }
//        } catch (const char* err) {
//            response = json_loader::SerializeError(err);
//        }
//        return response;
//    }

//private:
//    model::Game& game_;
//};

//class RequestHandler {
//public:
//    explicit RequestHandler(model::Game& game, fs::path staticPath)
//        : game_{game}, staticPath_{staticPath}, apiHandler_{game_} {
//    }

//    RequestHandler(const RequestHandler&) = delete;
//    RequestHandler& operator=(const RequestHandler&) = delete;

//    template <typename Body, typename Allocator, typename Send>
//    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
//        // Обработать запрос request и отправить ответ, используя send
//        const auto text_response = [&req](http::status status, std::string_view text, std::string_view content_type) {
//            return MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
//        };
//        const auto file_response = [&req](http::status status, http::file_body::value_type& file, std::string_view content_type) {
//            return MakeFileResponse(status, file, req.version(), req.keep_alive(), content_type);
//        };

//        if (req.method_string() == "GET" || req.method_string() == "HEAD"){
//            std::string request = {req.target().begin(), req.target().end()};
//            request = UrlDecode(request);
//            std::string response;
//            if(IsApiRequest(request)){
//                response = apiHandler_.HandleRequest(request);
//                send(std::move(text_response(http::status::ok, response, ContentType::APPLICATION_JSON)));
//            } else {
//                request = request.substr(1, request.size()-1);
//                if(request == "")
//                    request = "index.html";
//                fs::path request_file = fs::weakly_canonical(staticPath_ / request);
//                if (IsSubPath(request_file, staticPath_)){
//                    std::string extension = request_file.extension();
//                    boost::algorithm::to_lower(extension);

//                    http::file_body::value_type file;
//                    if (sys::error_code ec; file.open(request_file.c_str(), beast::file_mode::read, ec), ec) {
//                        std::cout << "Failed to open file "sv << extension << std::endl;
//                        send(std::move(text_response(http::status::not_found, "Not Found", ContentType::TEXT_PLAIN)));
//                    } else {
//                        send(std::move(file_response(http::status::ok, file, GetContentType(extension))));
//                    }

//                } else {
//                    send(std::move(text_response(http::status::bad_request, "Bad request", ContentType::TEXT_PLAIN)));
//                }
//            }

//        } else {
//            send(std::move(text_response(http::status::method_not_allowed, "Invalid method", ContentType::TEXT_HTML)));
//        }
//    }

//private:
//    model::Game& game_;
//    fs::path staticPath_;
//    RequestHandlerAPI apiHandler_;
//};

#endif // REQUESTHANDLERAPI_H
