#include "logger.h"

void MyFormatter(const boost::log::record_view &rec, boost::log::formatting_ostream &strm) {
    auto ts = *rec[timestamp];
    auto format_ts = to_iso_extended_string(ts);
    auto jsonData = *rec[additional_data];
    auto message = *rec[expr::smessage];
    auto log = json_loader::BuildLog(format_ts, jsonData, message);
    strm << log;
}

Logger &Logger::GetInstance() {
    static Logger obj;
    return obj;
}

void Logger::InitBoostLogFilter() {
    logging::add_common_attributes();
    logging::add_console_log(
                std::clog,
                keywords::auto_flush,
                keywords::format = &MyFormatter
            );
}
