#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace vk
{
    std::shared_ptr<spdlog::logger> Log::logger;

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        logger = spdlog::stderr_color_mt("Vulkan APP");
        logger->set_level(spdlog::level::trace);
    }
}