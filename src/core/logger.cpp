#include "core/logger.h"
#include <iostream>

using namespace tri;

Logger::Logger()
{
}

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

void Logger::Log(LogLevel level, const std::string& tag, const std::string& message)
{
    LogEntry entry;
    entry.level = level;
    entry.tag = tag;
    entry.message = message;
    entry.timestamp = std::chrono::system_clock::now();

    std::vector<Listener> listeners_copy;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (entries_.size() >= max_entries_)
        {
            // Remove oldest 10% when capacity reached
            size_t remove_count = max_entries_ / 10;
            if (remove_count == 0) remove_count = 1;
            entries_.erase(entries_.begin(), entries_.begin() + remove_count);
        }
        entries_.push_back(entry);
        listeners_copy = listeners_;
    }

    // Console output
    std::string prefix;
    switch (level)
    {
        case LogLevel::Info:    prefix = "[INFO]"; break;
        case LogLevel::Warning: prefix = "[WARN]"; break;
        case LogLevel::Error:   prefix = "[ERROR]"; break;
    }

    if (!tag.empty())
    {
        prefix += " [" + tag + "] ";
    }
    else
    {
        prefix += " ";
    }

    if (level == LogLevel::Error)
    {
        std::cerr << prefix << message << std::endl;
    }
    else
    {
        std::cout << prefix << message << std::endl;
    }

    for (const auto& listener : listeners_copy)
    {
        if (listener)
        {
            listener(entry);
        }
    }
}

void Logger::LogInfo(const std::string& tag, const std::string& message)
{
    Log(LogLevel::Info, tag, message);
}

void Logger::LogWarning(const std::string& tag, const std::string& message)
{
    Log(LogLevel::Warning, tag, message);
}

void Logger::LogError(const std::string& tag, const std::string& message)
{
    Log(LogLevel::Error, tag, message);
}

std::vector<LogEntry> Logger::GetEntries()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

void Logger::Clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
}

void Logger::SetMaxEntries(size_t max_entries)
{
    std::lock_guard<std::mutex> lock(mutex_);
    max_entries_ = max_entries;
}

size_t Logger::GetEntryCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

void Logger::AddListener(Listener listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(std::move(listener));
}
