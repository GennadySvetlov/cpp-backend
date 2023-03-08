#include "request_handler.h"

namespace http_handler {

StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive, std::string_view content_type) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
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

}  // namespace http_handler
