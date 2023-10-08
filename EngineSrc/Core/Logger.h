#ifndef LOGGER_HDR
#define LOGGER_HDR

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Gust 
{
class Logger 
{
public:
    static void init();

    static std::shared_ptr<spdlog::logger>& getLogger() { return _logger; }

private:
    static std::shared_ptr<spdlog::logger> _logger;
};

}

#define GUST_CRITICAL(...) Gust::Logger::getLogger()->critical(__VA_ARGS__)
#define GUST_ERROR(...) Gust::Logger::getLogger()->error(__VA_ARGS__)
#define GUST_WARN(...) Gust::Logger::getLogger()->warn(__VA_ARGS__)
#define GUST_INFO(...) Gust::Logger::getLogger()->info(__VA_ARGS__)
#define GUST_TRACE(...) Gust::Logger::getLogger()->trace(__VA_ARGS__)

#define GUST_DEBUG 1
#ifdef GUST_DEBUG
    #define GUST_CORE_ASSERT(condition, ...) { if((condition)) { GUST_CRITICAL("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define GUST_CORE_ASSERT(condition, ...)
#endif

#endif // !LOGGER_HDR
