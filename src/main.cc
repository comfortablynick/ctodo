#include "config.h"
#include <filesystem>
#include <fmt/ostream.h>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <vector>
#ifndef LOG_IS_ON
#define LOG_IS_ON(verbosity) ((loguru::Verbosity_##verbosity) <= loguru::current_verbosity_cutoff())
#endif

#ifndef DEBUG_MODE
#define DEBUG_MODE 1 // Automatically output console logs by default
#endif

// Output a text representation of vector to stream.
// For pretty output, use prettify() to get string first.
// @param out Stream to print to
// @param vec Vector to print
template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
    out << '[';
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) out << ", ";
        out << vec[i];
    }
    out << ']';
    return out;
}

// Pretty print representation of vector.
// For simple debug print, use << operator on vector directly.
// @param vec Vector of <T> type
template <class T>
std::string prettify(const std::vector<T>& vec)
{
    fmt::memory_buffer out;
    fmt::format_to(out, "[\n");
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) fmt::format_to(out, ",\n");
        fmt::format_to(out, "{:4}: {}", i, vec[i]);
    }
    fmt::format_to(out, "\n]");
    return fmt::to_string(out);
}
// namespace Ansi { {{{
//     // Value on the Ansi 256 color spectrum
//     enum class Color : unsigned int
//     {
//         // std colors
//         black = 0,
//         blue = 12,
//         green = 2,
//         cyan = 37,
//         red = 124,
//         yellow = 142,
//         gray = 245,
//
//         // bright colors
//         brcyan = 51,
//         brred = 196,
//         bryellow = 226,
//     };
//
//     // Set foreground color
//     //
//     // @param color Color from Ansi::Color enum
//     const std::string setFg(Color color)
//     {
//         const auto env_term = getenv("TERM");
//         if (env_term == nullptr || strcmp(env_term, "dumb") == 0) {
//             return "";
//         }
//         return fmt::format("\033[38;5;{}m", static_cast<unsigned int>(color));
//     }
//
//     // Set background color
//     //
//     // @param color Color from Ansi::Color enum
//     const std::string setBg(Ansi::Color color)
//     {
//         const auto env_term = getenv("TERM");
//         if (env_term == nullptr || strcmp(env_term, "dumb") == 0) {
//             return "";
//         }
//         return fmt::format("\033[48;5;{}m", static_cast<unsigned int>(color));
//     }
//
//     // Reset colors
//     const std::string reset()
//     {
//         const auto env_term = getenv("TERM");
//         if (env_term == nullptr || strcmp(env_term, "dumb") == 0) {
//             return "";
//         }
//         return "\033[0m";
//     }
// } // namespace Ansi
// }}}
struct termsize
{
    unsigned cols, lines;
};

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

void init_loguru(int& argc, char** argv, const char* verbosity_flag = "-v")
{
    // init loguru
    //
    // VERBOSITY LEVELS
    //  -2 |   ERROR
    //  -1 | WARNING
    //   0 |    INFO
    // 1-9 | VERBOSE
    loguru::g_stderr_verbosity = -2;
    loguru::g_colorlogtostderr = true;
    loguru::g_flush_interval_ms = 100;
    auto tsize = getTermSize();
    if (tsize->cols < 200) {
        loguru::g_preamble_thread = false;
        loguru::g_preamble_date = false;
        loguru::g_preamble_time = false;
        if (tsize->cols < 100) {
            loguru::g_preamble_file = false;
        }
    }
    loguru::init(argc, argv, verbosity_flag);
    VLOG_F(1, "Terminal size: {}x{}", tsize->cols, tsize->lines);
}

std::string get_file_contents(const char* fpath)
{
    std::ifstream file(fpath);
    CHECK_F(file.is_open(), "Failed to open file '{}'", fpath);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

template <typename ContainerT>
void tokenize(const std::string& str, ContainerT& tokens, const std::string& delimiters = " ",
              bool trimEmpty = false)
{
    std::string::size_type pos, lastPos = 0, length = str.length();

    using value_type = typename ContainerT::value_type;
    using size_type = typename ContainerT::size_type;

    while (lastPos < length + 1) {
        pos = str.find_first_of(delimiters, lastPos);
        if (pos == std::string::npos) {
            pos = length;
        }

        if (pos != lastPos || !trimEmpty)
            tokens.push_back(value_type(str.data() + lastPos, (size_type)pos - lastPos));

        lastPos = pos + 1;
    }
}

int main(int argc, char** argv)
{
    if constexpr (DEBUG_MODE) {
        // Generate test arguments
        std::vector<std::string> arguments(argv, argv + argc);
        arguments.emplace(arguments.begin() + 1, "-v");
        arguments.emplace(arguments.begin() + 2, "INFO");
        std::vector<char*> args; // convert vector back to char**
        for (auto& str : arguments) {
            args.push_back(&str.front());
        }
        int arg_ct = args.size();
        init_loguru(arg_ct, args.data());
    } else {
        loguru::g_internal_verbosity = 1;
        // init loguru with raw cli args
        init_loguru(argc, argv);
    }
    std::filesystem::path fpath(std::getenv("HOME"));
    fpath.append("Dropbox").append("todo").append("todo.txt");
    LOG_F(INFO, "Todo file path: {}", fpath);
    std::string raw(get_file_contents(fpath.c_str()));
    std::vector<std::string> lines;
    tokenize(raw, lines, "\n", true);
    fmt::print(std::cout, "{}\n", prettify(lines));

    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::string> words;
        tokenize(lines[i], words, " ", true);
        LOG_F(2, "Line {}\n{}\n{}", i, lines[i], words);
    }
}
