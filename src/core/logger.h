#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <functional>

namespace tri
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error
    };

    struct LogEntry
    {
        LogLevel level;
        std::string tag;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
    };

    class Logger
    {
    public:
        static Logger& Instance();

        void Log(LogLevel level, const std::string& tag, const std::string& message);
        void LogInfo(const std::string& tag, const std::string& message);
        void LogWarning(const std::string& tag, const std::string& message);
        void LogError(const std::string& tag, const std::string& message);

        std::vector<LogEntry> GetEntries();
        void Clear();

        void SetMaxEntries(size_t max_entries);
        size_t GetEntryCount();

        using Listener = std::function<void(const LogEntry&)>;
        void AddListener(Listener listener);

    private:
        Logger();
        ~Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::mutex mutex_;
        std::vector<LogEntry> entries_;
        std::vector<Listener> listeners_;
        size_t max_entries_{ 5000 };
    };

    #define TRI_LOG_INFO(tag, msg) ::tri::Logger::Instance().LogInfo(tag, msg)
    #define TRI_LOG_WARN(tag, msg) ::tri::Logger::Instance().LogWarning(tag, msg)
    #define TRI_LOG_ERROR(tag, msg) ::tri::Logger::Instance().LogError(tag, msg)
} // namespace tri
