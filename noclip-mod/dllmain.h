#pragma once
#include <string>
#include <format>

void LogMessage(const char* message);

inline void LogMessage(const std::string& message) {
    LogMessage(message.c_str());
}

void LogMessage(const std::string& format, auto&&... args) {
    LogMessage(std::vformat(format, std::make_format_args(args...)));
}
