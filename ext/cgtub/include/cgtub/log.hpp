#pragma once

namespace cgtub
{

enum class LogLevel : unsigned int
{
    Trace = 0,
    Debug,
    Warn,
    Error,
    Unknown
};

char const* get_log_level_string(LogLevel level);

void set_log_level(LogLevel level);

void log_message(LogLevel level, char const* format, ...);

} // namespace cgtub