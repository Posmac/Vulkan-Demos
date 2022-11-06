#pragma once

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace vk
{
    class Log
    {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return logger; };

    private:
        static std::shared_ptr<spdlog::logger> logger;
    };
}

#define TRACE_LOG(...) ::vk::Log::GetLogger()->trace(__VA_ARGS__)
#define INFO_LOG(...) ::vk::Log::GetLogger()->info(__VA_ARGS__)
#define WARN_LOG(...) ::vk::Log::GetLogger()->warn(__VA_ARGS__)
#define ERROR_LOG(...) ::vk::Log::GetLogger()->error(__VA_ARGS__)
#define CIRITCAL_LOG(...) ::vk::Log::GetLogger()->critical(__VA_ARGS__)