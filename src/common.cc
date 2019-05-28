#ifndef _CONFIG_H_
#define _CONFIG_H_
#include "common.h"
#include <fmt/core.h>
#include <loguru.hpp>
#include <stdlib.h>
#include <string>
#include <string_view>

std::ostream& operator<<(std::ostream& out, std::shared_ptr<options> obj)
{
    out << "Options:";
    out << "\n  Cmd: " << obj->cmd;
    out << "\n  Verbosity: " << obj->verbosity;
    out << "\n  Quiet: " << obj->quiet;
    out << "\n  Getline: " << obj->getline;
    out << '\n';
    return out;
}

/// Get runtime terminal size (lines & cols)
std::shared_ptr<termsize> getTermSize()
{
    auto tsize_t = std::make_shared<termsize>();
#if defined(TIOCGSIZE)
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    tsize_t->cols = ts.ts_cols;
    tsize_t->lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    tsize_t->cols = ts.ws_col;
    tsize_t->lines = ts.ws_row;
#endif
    return tsize_t;
}

std::string prettify(int size, char** data)
{
    std::vector<std::string> vec(data, data + size);
    return prettify(vec);
}

/// Get string value of env variable
/// @param key Read-only env var key
std::string get_env_var(std::string_view key)
{
    const char* val = getenv(key.data());
    if (val != nullptr) {
        return std::string(val);
    }
    LOG_F(WARNING, "Undefined env var '{}'", key);
    return std::string();
}

namespace Ansi {
    /// Set foreground color
    /// @param color Color from Ansi::Color enum
    const std::string setFg(Ansi::Color color)
    {
        if (std::string e = get_env_var("TERM"); e == "dumb") {
            return std::string();
        }
        return fmt::format("\033[38;5;{}m", static_cast<unsigned int>(color));
    }

    /// Set foreground color
    /// @param color Color from 256 color palette
    const std::string setFg(unsigned int color)
    {
        if (std::string e = get_env_var("TERM"); e == "dumb") {
            return std::string();
        }
        return fmt::format("\033[38;5;{}m", color);
    }

    /// Set background color
    /// @param color Color from Ansi::Color enum
    const std::string setBg(Ansi::Color color)
    {
        if (std::string e = get_env_var("TERM"); e == "dumb") {
            return std::string();
        }
        return fmt::format("\033[48;5;{}m", static_cast<unsigned int>(color));
    }

    /// Reset colors
    const std::string reset()
    {
        if (std::string e = get_env_var("TERM"); e == "dumb") {
            return std::string();
        }
        return "\033[0m";
    }
} // namespace Ansi
#endif // _CONFIG_H_
