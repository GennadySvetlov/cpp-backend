#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "json_loader.h"
#include "model.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "logger.h"
#include "json_loader.h"

namespace json = boost::json;
//#include <variant>
//#include <boost/filesystem.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;

using namespace std::literals;

class SyncWriteOStreamAdapter {
public:
    explicit SyncWriteOStreamAdapter(std::ostream& os)
        : os_{os} {}

    template <typename ConstBufferSequence>
    size_t write_some(const ConstBufferSequence& cbs, sys::error_code& ec) {
        const size_t total_size = net::buffer_size(cbs);
        if (total_size == 0) {
            ec = {};
            return 0;
        }
        size_t bytes_written = 0;
        for (const auto& cb : cbs) {
            const size_t size = cb.size();
            const char* const data = reinterpret_cast<const char*>(cb.data());
            if (size > 0) {
                if (!os_.write(reinterpret_cast<const char*>(data), size)) {
                    ec = make_error_code(boost::system::errc::io_error);
                    return bytes_written;
                }
                bytes_written += size;
            }
        }
        ec = {};
        return bytes_written;
    }

    template <typename ConstBufferSequence>
    size_t write_some(const ConstBufferSequence& cbs) {
        sys::error_code ec;
        const size_t bytes_written = write_some(cbs, ec);
        if (ec) {
            throw std::runtime_error("Failed to write");
        }
        return bytes_written;
    }

private:
    std::ostream& os_;
};

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
// Ответ, тело которого представлено в виде файла
using FileResponse = http::response<http::file_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JAVASCRIPT = "text/javascript"sv;
    constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
    constexpr static std::string_view APPLICATION_XML = "application/xml"sv;
    constexpr static std::string_view APPLICATION_OCTET = "application/octet-stream"sv;
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;
    constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
    // При необходимости внутрь ContentType можно добавить и другие типы контента
};

enum FileExtension {
    default_,
    htm,
    html,
    css,
    txt,
    js,
    json,
    xml,
    png,
    jpg,
    jpe,
    jpeg,
    gif,
    bmp,
    ico,
    tiff,
    tif,
    svg,
    svgz,
    mp3
};

static std::map<std::string, FileExtension> s_mapStringValues{
    {"", FileExtension::default_},
    {".htm", FileExtension::htm},
    {".html", FileExtension::html},
    {".css", FileExtension::css},
    {".txt", FileExtension::txt},
    {".js", FileExtension::js},
    {".json", FileExtension::json},
    {".xml", FileExtension::xml},
    {".png", FileExtension::png},
    {".jpg", FileExtension::jpg},
    {".jpe", FileExtension::jpe},
    {".jpeg", FileExtension::jpeg},
    {".gif", FileExtension::gif},
    {".bmp", FileExtension::bmp},
    {".ico", FileExtension::ico},
    {".tiff", FileExtension::tiff},
    {".tif", FileExtension::tif},
    {".svg", FileExtension::svg},
    {".svgz", FileExtension::svgz},
    {".mp3", FileExtension::mp3},

};

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body,
                                  unsigned http_version, bool keep_alive,
                                  std::string_view content_type);

FileResponse MakeFileResponse(http::status status, http::file_body::value_type& body,
                              unsigned http_version, bool keep_alive,
                              std::string_view content_type);

const std::string validMapPath = "/api/v1/maps/";

bool IsMapRequest(std::string request);

bool IsMapsRequest(std::string request);

bool IsApiRequest(std::string request);

std::string GetMapId(std::string request);

std::string UrlDecode(const std::string& value);

std::string_view GetContentType(std::string fileExtension);

// Возвращает true, если каталог p содержится внутри base_path.
bool IsSubPath(fs::path path, fs::path base);

template<class SomeRequestHandler>
class LoggingRequestHandler {
    static void LogRequest(const StringRequest& r){
        std::string request = {r.target().begin(), r.target().end()};
        json::value custom_data{{"data"s, request}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "get StringRequest"sv;
    };
    template<typename Send>
    static void LogResponse(const Send& r){
//        std::string_view resp = r.base().at(http::field::content_type);
//        json::value custom_data{{"data"s, resp}};
//        BOOST_LOG_TRIVIAL(info) << r << "get StringResponse"sv;
    };
public:
    LoggingRequestHandler(SomeRequestHandler& requestHandler)
        : decorated_(requestHandler){
    };

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
//        LogRequest(req);

//        decorated_(std::move(req), std::forward<decltype(send)>(send));
        decorated_(std::move(req), std::move(send));

        //LogRequest(req);

        //LogResponse(send);
    }

private:
    SomeRequestHandler& decorated_;
};


class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, fs::path staticPath)
        : game_{game}, staticPath_{staticPath} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        const auto text_response = [&req](http::status status, std::string_view text, std::string_view content_type) {
            return MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
        };
        const auto file_response = [&req](http::status status, http::file_body::value_type& file, std::string_view content_type) {
            return MakeFileResponse(status, file, req.version(), req.keep_alive(), content_type);
        };

        if (req.method_string() == "GET" || req.method_string() == "HEAD"){
            std::string request = {req.target().begin(), req.target().end()};
            request = UrlDecode(request);
            std::string response;
            //std::cout << request << std::endl;
            if(IsApiRequest(request)){
                try {
                    if (IsMapsRequest(request)){
                        response = json_loader::SerializeMaps(game_);
                    } else if (IsMapRequest(request)){
                        response = json_loader::SerializeMap(game_, GetMapId(request));
                        if (response == "mapNotFound")
                            throw "mapNotFound";
                    } else {
                        throw "badRequest";
                    }
                    send(std::move(text_response(http::status::ok, response, ContentType::APPLICATION_JSON)));
                }  catch (const char* err) {
                    if (err == "mapNotFound"sv)
                        send(std::move(text_response(http::status::not_found, json_loader::SerializeError(err), ContentType::APPLICATION_JSON)));
                    if (err == "badRequest"sv)
                        send(std::move(text_response(http::status::bad_request, json_loader::SerializeError(err), ContentType::APPLICATION_JSON)));
                }
            } else {
                request = request.substr(1, request.size()-1);
                if(request == "")
                    request = "index.html";
                fs::path request_file = fs::weakly_canonical(staticPath_ / request);
                if (IsSubPath(request_file, staticPath_)){
                    std::string extension = request_file.extension();
                    boost::algorithm::to_lower(extension);

                    http::file_body::value_type file;
                    if (sys::error_code ec; file.open(request_file.c_str(), beast::file_mode::read, ec), ec) {
                        std::cout << "Failed to open file "sv << extension << std::endl;
                        send(std::move(text_response(http::status::not_found, "Not Found", ContentType::TEXT_PLAIN)));
                    } else {
                        send(std::move(file_response(http::status::ok, file, GetContentType(extension))));
                    }

                } else {
                    send(std::move(text_response(http::status::bad_request, "Bad request", ContentType::TEXT_PLAIN)));
                }
            }

        } else {
            send(std::move(text_response(http::status::method_not_allowed, "Invalid method", ContentType::TEXT_HTML)));
        }
    }

private:
    model::Game& game_;
    fs::path staticPath_;
};

}  // namespace http_handler
