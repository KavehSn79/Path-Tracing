#include "cgtub/log.hpp"

#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace cgtub
{

struct LogState
{
    LogLevel minimum_level = LogLevel::Debug;
    FILE*    stream        = stdout;
};

LogState& get_log_state()
{
    static LogState state;
    return state;
}

char const* get_log_level_string(LogLevel level)
{
    static char const* strings[] = {"Trace",
                                    "Debug",
                                    "Warn",
                                    "Error"};
    
    static_assert(std::size(strings) == static_cast<unsigned int>(LogLevel::Unknown), "Number of LogLevel enum values does not match number of strings.");

    return strings[static_cast<unsigned int>(level)];
}

void set_log_level(LogLevel level)
{
    // TODO: Not thread-safe
    get_log_state().minimum_level = level;
}

void log_message(LogLevel level, char const* format, ...)
{
    LogState const& state = get_log_state();

    // Do not emit the message if it's level is below the minimum
    if (static_cast<unsigned int>(level) < static_cast<unsigned int>(state.minimum_level))
        return;

    std::va_list args;
    va_start(args, format);
    std::fprintf(state.stream, "[%s] ", get_log_level_string(level));
    std::vfprintf(state.stream, format, args);
    std::putc('\n', state.stream);
    va_end(args);
}

} // namespace cgtub
