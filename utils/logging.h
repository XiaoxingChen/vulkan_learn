#if !defined(_LOGGING_H_)
#define _LOGGING_H_

#ifdef __ANDROID__
#include <spdlog/sinks/android_sink.h>
#endif
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/spdlog.h>
#include <vector>

#define LOGT(...) spdlog::trace(__VA_ARGS__)
#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error(__VA_ARGS__);
#define LOGD(...) spdlog::debug(__VA_ARGS__);

inline void initLogger()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#ifdef __ANDROID__
	sinks.push_back(std::make_shared<spdlog::sinks::android_sink_mt>("vk_learn"));
#endif
    auto logger = std::make_shared<spdlog::logger>("vk_learn", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);
    LOGI("Logger initialized");
}

inline void destroyLogger()
{
    spdlog::drop_all();
}

#endif // _LOGGING_H_
