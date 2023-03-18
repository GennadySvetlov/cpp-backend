#include "http_server.h"

#include <boost/asio/dispatch.hpp>
#include <iostream>



namespace http_server {

void ReportError(beast::error_code ec, std::string_view what) {
    json::value custom_data{{"code"s, ec.value()}, {"text"s, ec.message()}, {"where"s, what}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                            << "error"sv;
    //std::cerr << what << ": "sv << ec.message() << std::endl;
}

void SessionBase::Run() {
    // Вызываем метод Read, используя executor объекта stream_.
    // Таким образом вся работа со stream_ будет выполняться, используя его executor
    net::dispatch(stream_.get_executor(),
                  beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

void SessionBase::Read() {
    // Очищаем запрос от прежнего значения (метод Read может быть вызван несколько раз)
    request_ = {};
    stream_.expires_after(30s);
    // Считываем request_ из stream_, используя buffer_ для хранения считанных данных
    http::async_read(stream_, buffer_, request_,
                     // По окончании операции будет вызван метод OnRead
                     beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(beast::error_code ec, std::size_t bytes_read) {
    if (ec == http::error::end_of_stream) {
        // Нормальная ситуация - клиент закрыл соединение
        return Close();
    }
    if (ec) {
        return ReportError(ec, "read"sv);
    }
    json::value custom_data{{"ip"s, stream_.socket().local_endpoint().address().to_string()},
                            {"URI"s, std::string{request_.target().begin(), request_.target().end()}},
                            {"method"s, request_.method_string()}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "request received"sv;
    start_ts_ = std::chrono::system_clock::now();
    HandleRequest(std::move(request_));
}

void SessionBase::Close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}

void SessionBase::OnWrite(bool close, beast::error_code ec, std::size_t bytes_written) {
    if (ec) {
        return ReportError(ec, "write"sv);
    }
    if (close) {
        // Семантика ответа требует закрыть соединение
        return Close();
    }
    // Считываем следующий запрос
    Read();
}

}  // namespace http_server
