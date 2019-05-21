#include "config.h"
#include <algorithm>
#include <args.hxx>
#include <cstdlib>
// #include <cxxopts.hpp>
#include <ext/alloc_traits.h>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream> // IWYU pragma: keep
#include <iostream>
#include <loguru.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
// #include <algorithm>

constexpr bool DEBUG_MODE = true; // More verbose console logging

/// Output a text representation of vector to stream.
/// For pretty output, use prettify() to get string first.
/// @param `out` Stream to print to
/// @param `vec` Vector to print
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

/// Pretty print representation of vector.
/// For simple debug print, use << operator on vector directly.
/// @param `vec` Vector of <T> type
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
    /// Value on the Ansi 256 color spectrum
    enum class Color : unsigned int
    {
        // std colors
        black = 0,
        blue = 12,
        green = 2,
        cyan = 37,
        red = 124,
        yellow = 142,
        lime = 154,
        lightorange = 215,
        gray = 245,

        // bright colors
        brcyan = 51,
        brred = 196,
        bryellow = 226,
    };

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
    const std::string setBg(Color color)
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

/// Data structure for terminal cols and lines
struct termsize
{
    unsigned cols, lines;
};

struct options
{
    std::string cmd, verbosity;
    bool quiet, getline;
};

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

/// Initialize loguru with command line args
/// @param argc CLI argument count
/// @param argv CLI argument array
/// @param verbosity_flag Optional flag to parse for verbosity level.
/// Set to `nullptr` to skip parsing flag
///
/// | -v | Level  |
/// |----|--------|
/// | -2 |   ERROR|
/// | -1 | WARNING|
/// |  0 |    INFO|
/// |1-9 | VERBOSE|
void init_loguru(int& argc, char** argv, const char* verbosity_flag = "-v")
{
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

/// Get filesystem path of todo.txt file
std::filesystem::path get_todo_file_path()
{
    // TODO: check env vars
    std::filesystem::path fpath(getenv("HOME"));
    fpath.append("Dropbox").append("todo").append("todo.txt");
    LOG_F(INFO, "Todo file path: {}", fpath.c_str());
    return fpath;
}

/// Get entire file as a string
/// @param fpath Path to file
std::string get_file_contents(std::filesystem::path fpath)
{
    std::ifstream file{fpath};
    CHECK_F(file.is_open(), "Failed to open file '{}'", fpath.c_str());
    const auto fsize = std::filesystem::file_size(fpath);
    std::string result(fsize, ' ');
    file.read(result.data(), fsize);
    return result;
}

/// Get entire file as a vec of strings
/// @param fpath Path to file
std::vector<std::string> get_file_lines(std::filesystem::path fpath)
{
    std::ifstream file{fpath};
    CHECK_F(file.is_open(), "Failed to open file '{}'", fpath.c_str());
    std::vector<std::string> result;
    std::string line;
    line.reserve(256);
    while (getline(file, line)) {
        result.emplace_back(line);
    }
    file.close();
    return result;
}

/// Get a container of tokens from string
/// @param str String view (read-only) to tokenize
/// @param tokens Container ref to fill
/// @param delimiters Split using this string view
template <typename ContainerT>
void tokenize(std::string_view str, ContainerT& tokens, std::string_view delimiters = " ",
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

/// Get colorized output for printing to console
/// @param lines Reference to vector of lines
std::string format_lines(std::vector<std::string>& lines)
{
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i) {
        std::vector<std::string> words;
        tokenize(lines[i], words, " ", true);
        if (i > 0) out.push_back('\n');
        for (auto& word : words) {
            switch (word.at(0)) {
            case '@':
                out.append(fmt::format("{}{}{}", Ansi::setFg(Ansi::Color::lightorange), word,
                                       Ansi::reset()));
                break;
            case '+':
                out.append(
                    fmt::format("{}{}{}", Ansi::setFg(Ansi::Color::lime), word, Ansi::reset()));
                break;
            default:
                out.append(word);
            }
            out.push_back(' ');
        }
    }
    return out;
}


/// Remove trailing character of `str` if it matches `ch`
/// @param str String reference to modify
/// @param ch Character to chomp if last character in `str`
void chomp_trailing_char(std::string& str, const char ch)
{
    if (auto len = str.length(); len > 0 && str[len - 1] == ch) {
        str.resize(len - 1);
        return;
    }
    LOG_F(INFO, "Trailing character of string did not match '{}'", ch);
    return;
}

int parse_args(int argc, char* argv[], std::shared_ptr<options> opts)
{
    args::ArgumentParser parser(PACKAGE_DESCRIPTION);
    parser.helpParams.useValueNameOnce = true;
    parser.helpParams.proglinePreferShortFlags = true;
    parser.helpParams.width = 100;
    parser.helpParams.proglineOptions = "[OPTIONS]";
    parser.helpParams.shortSeparator = " ";
    parser.helpParams.longSeparator = " ";
    parser.helpParams.valueOpen = "";
    parser.helpParams.valueClose = "";
    args::HelpFlag help(parser, "help", "Display this help menu and exit", {'h', "help"});
    args::Flag version(parser, "version", "Display version info and exit", {'V', "version"});
    args::Flag quiet(parser, "quiet", "Silence debug messages", {'q', "quiet"});
    args::Flag getline(parser, "getline", "Use getline method to read file", {'g', "getline"});
    args::ValueFlag<std::string> verbosity(
        parser, "LEVEL", "Level of debug messages printed to console", {'v', "verbosity"});
    try {
        parser.ParseCLI(argc, argv);
        if (args::get(quiet)) opts->quiet = true;
        if (args::get(getline)) opts->getline = true;
        return 0;
    } catch (args::Help&) {
        auto help = parser.Help();
        chomp_trailing_char(help, '\n');
        std::cerr << help;
        return EXIT_FAILURE;
    } catch (args::Error& e) {
        // TODO: print usage here
        std::cerr << e.what();
        return EXIT_FAILURE;
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
        init_loguru(argc, argv); // init loguru with raw cli args
    }
    std::shared_ptr<options> opts = std::make_shared<options>();
    if (parse_args(argc, argv, opts) != 0) {
        exit(EXIT_FAILURE);
    }
    auto fpath = get_todo_file_path();
    if (opts->getline) {
        LOG_F(INFO, "Reading file lines into vector");
        auto lines = get_file_lines(fpath);
        std::string out(format_lines(lines));
        std::cout << out << std::endl;
    } else {
        LOG_F(INFO, "Reading contents of file into string");
        std::string raw(get_file_contents(fpath));
        std::vector<std::string> lines;
        tokenize(raw, lines, "\n", true);
        std::string out(format_lines(lines));
        std::cout << out << std::endl;
    }
}
