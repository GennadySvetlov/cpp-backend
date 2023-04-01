#include "request_handler.h"

namespace http_handler {

StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type)
{
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    //    SyncWriteOStreamAdapter adapter{std::cout};
    //    http::write(adapter, response);

    return response;
}

FileResponse MakeFileResponse(http::status status, http::file_body::value_type &body, unsigned http_version, bool keep_alive, std::string_view content_type)
{
    FileResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = std::move(body);
    response.prepare_payload();
    response.keep_alive(keep_alive);

    //    SyncWriteOStreamAdapter adapter{std::cout};
    //    http::write(adapter, response);

    return response;
}

bool IsMapRequest(std::string request)
{
    return request.starts_with(validMapPath) && (request.size() > validMapPath.size());
}

std::string GetMapId(std::string request)
{
    return request.substr(validMapPath.size(), request.size()-validMapPath.size());
}

bool IsMapsRequest(std::string request)
{
    return (request == validMapPath.substr(0, validMapPath.size()-1));
}

bool IsApiRequest(std::string request)
{
    return (request.starts_with(validMapPath.substr(0, 5))); // request == "/api/..."
}

bool IsJoinRequest(std::string request)
{
    return request == "/api/v1/game/join";
}

std::string UrlDecode(const std::string &value)
{
    std::string result;
    result.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        auto ch = value[i];

        if (ch == '%' && (i + 2) < value.size())
        {
            auto hex = value.substr(i + 1, 2);
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(ch);
        }
    }
    return result;
}

bool IsSubPath(fs::path path, fs::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);
    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}

std::string_view GetContentType(std::string fileExtension)
{
    switch (s_mapStringValues[fileExtension]) {
    case FileExtension::htm:
    case FileExtension::html:
        return ContentType::TEXT_HTML;
    case FileExtension::css:
        return ContentType::TEXT_CSS;
    case FileExtension::txt:
        return ContentType::TEXT_PLAIN;
    case FileExtension::js:
        return ContentType::TEXT_JAVASCRIPT;
    case FileExtension::json:
        return ContentType::APPLICATION_JSON;
    case FileExtension::xml:
        return ContentType::APPLICATION_XML;
    case FileExtension::png:
        return ContentType::IMAGE_PNG;
    case FileExtension::jpg:
    case FileExtension::jpe:
    case FileExtension::jpeg:
        return ContentType::IMAGE_JPEG;
    case FileExtension::gif:
        return ContentType::IMAGE_GIF;
    case FileExtension::bmp:
        return ContentType::IMAGE_BMP;
    case FileExtension::ico:
        return ContentType::IMAGE_ICO;
    case FileExtension::tiff:
    case FileExtension::tif:
        return ContentType::IMAGE_TIFF;
    case FileExtension::svg:
    case FileExtension::svgz:
        return ContentType::IMAGE_SVG;
    case FileExtension::mp3:
        return ContentType::AUDIO_MPEG;
    default:
        return ContentType::APPLICATION_OCTET;
    }

}



}  // namespace http_handler
