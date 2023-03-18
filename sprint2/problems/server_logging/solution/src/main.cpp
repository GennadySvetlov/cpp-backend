#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include <iostream>
#include <thread>

#include "logger.h"
#include "json_loader.h"
#include "request_handler.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace json = boost::json;

namespace {

// Запускает функцию fn на threadCount потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned threadCount, const Fn& fn) {
    threadCount = std::max(1u, threadCount);
    std::vector<std::jthread> workers;
    workers.reserve(threadCount - 1);
    // Запускаем threadCount-1 рабочих потоков, выполняющих функцию fn
    while (--threadCount) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {
    Logger::GetInstance().InitBoostLogFilter();
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <front-filepath>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                json::value custom_data{{"code"s, 0}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                                        << "server exited"sv;
                ioc.stop();
            }
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его
        // с моделью игры и с путём к статическим файлам
        http_handler::RequestHandler handler{game, argv[2]};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;

        net::ip::tcp::endpoint endpoint{address, port};
        http_handler::LoggingRequestHandler logging_handler{handler};

        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        //        std::cout << "Server has started..."sv << std::endl;
        //        std::cout << "Hello! Server is starting at port " << port << std::endl;
        json::value log_data{{"port"s, port}, {"address"s, address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, log_data)
                                << "server started"sv;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        //std::cerr << ex.what() << std::endl;
        json::value custom_data{{"code"s, EXIT_FAILURE}, {"exception"s, ex.what()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                                << "error"sv;
        return EXIT_FAILURE;
    }
}
