#include "Logger.h"

namespace Gust 
{
    std::shared_ptr<spdlog::logger> Logger::_logger;

    void Logger::init() 
    {
        _logger = spdlog::stderr_color_mt("Gust");
        _logger->set_pattern("%^[%T] %n %l: %v%$");
        _logger->set_level(spdlog::level::info);
    }
}