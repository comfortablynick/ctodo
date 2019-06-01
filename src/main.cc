#include "CLI11.hpp"
#include "common.h"
#include "config.h"
#include <algorithm>
#include <bits/getopt_core.h>
#include <cstdlib>
#include <ext/alloc_traits.h>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ostream.h> // IWYU pragma: keep
#include <fstream>       // IWYU pragma: keep
#include <iostream>
#include <loguru.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

constexpr bool DEBUG_MODE = false; // More verbose console logging

/**
 * Initialize loguru with command line args
 *
 * | -v | Level  |
 * |----|--------|
 * | -2 |   ERROR|
 * | -1 | WARNING|
 * |  0 |    INFO|
 * |1-9 | VERBOSE|
 *
 * @param argc CLI argument count
 * @param argv CLI argument array
 * @param verbosity_flag Optional flag to parse for verbosity level.
 * Set to `nullptr` to skip parsing flag
 *
 * @return void
 */
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

/**
 * Get path to file
 *
 * @param void
 *
 * @return std::filesystem::path
 */
std::filesystem::path get_todo_file_path()
{
    // TODO: check env vars
    std::filesystem::path fpath(getenv("HOME"));
    fpath.append("Dropbox").append("todo").append("todo.txt");
    LOG_F(INFO, "Todo file path: {}", fpath.c_str());
    return fpath;
}

/**
 * Get entire file as a vec of strings
 *
 * @param fpath Path to file
 *
 * @return std::string File contents
 */
std::string get_file_contents(std::filesystem::path fpath)
{
    std::ifstream file{fpath};
    CHECK_F(file.is_open(), "Failed to open file '{}'", fpath.c_str());
    const auto fsize = std::filesystem::file_size(fpath);
    std::string result(fsize, ' ');
    file.read(result.data(), fsize);
    return result;
}

/**
 * Get entire file as a vec of strings
 *
 * @param fpath Path to file
 *
 * @return std::vector<std::string> File lines
 */
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

/**
 * Get a container of tokens from string
 *
 * @param str String view (read-only) to tokenize
 * @param tokens Container ref to fill
 * @param delimiters Split by this string (default: " ")
 * @param trimEmptyfalse Trim empty lines
 *
 * @return void
 */
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

/**
 * Format lines of todo.txt file
 *
 * @param lines
 *
 * @return std::string Formatted lines joined together
 */
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


/**
 * Remove trailing character of `str` if it matches `ch`
 *
 * @param str String reference to modify
 * @param ch Character to chomp if last character in `str`
 *
 * @return void
 */
void chomp_trailing_char(std::string& str, const char ch)
{
    if (auto len = str.length(); len > 0 && str[len - 1] == ch) {
        str.resize(len - 1);
        return;
    }
    LOG_F(INFO, "Trailing character of string did not match '{}'", ch);
    return;
}

/**
 * Use getopt() to parse specified command-line flags
 *
 * @param argc Argument count
 * @param argv Arguments
 * @param opts Options object
 *
 * @return int Non-zero value if an error is encountered
 */
int parse_opts(int argc, char* argv[], std::shared_ptr<options> opts)
{
    int opt;
    extern char* optarg;
    extern int optind, optopt, opterr;
    opterr = 0; // tell getopt not to print errors to stderr

    while ((opt = getopt(argc, argv, "hVqgv:t:")) != -1) {
        LOG_F(3, "Opt index {}: {}", optind, char(optopt));
        switch (opt) {
        case 'h':
            fmt::print("Usage: {}\n", argv[0]);
            break;
        case 'q':
            opts->quiet = true;
            break;
        case ':':
            LOG_F(WARNING, "Option '{}' requires an argument", optopt);
            break;
        case 'v':
            break;
        case '?':
            LOG_F(WARNING, "Unknown option '{}'", char(optopt));
            break;
        default:
            LOG_F(1, "Found: {}", optarg);
        }
    }

    std::vector<std::string> cmds{"add", "list"};

    if (LOG_IS_ON(2) && argc > 0) {
        LOG_SCOPE_F(2, "Argv after parsing:");
        for (int i = 0; i < argc; ++i) {
            LOG_F(2, "{}: {}", i, argv[i]);
            if (std::find(cmds.begin(), cmds.end(), argv[i]) != cmds.end()) {
                LOG_F(2, "--> Found command '{}'", argv[i]);
                opts->cmd = argv[i];
            }
        }
    }
    return 0;
}

/**
 * Parse command-line arguments and commands.
 *
 * @param argc Argument count
 * @param argv Array of arguments
 * @param opts Shared pointer of global options object
 *
 * @return int Success or failure of parsing
 */
int parse_args(int argc, char** argv, std::shared_ptr<options> opts)
{
    CLI::App app{PACKAGE_DESCRIPTION};

    bool quiet, version;
    std::string verbosity;

    app.add_flag("-q,--quiet", quiet, "Silence debug output");
    app.add_flag("-V,--version", version, "Print version info and exit");
    app.add_option("-v,--verbosity", verbosity, "Print debug logs to console");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        if (e.get_name() == "CallForHelp") {
            // manually handle help and retval
            // build in function returns 0 for help
            std::cerr << app.help();
            return 1;
        }
        LOG_F(ERROR, "ExitCode: {}, Name: {}, What: {}", e.get_exit_code(), e.get_name(), e.what());
        return app.exit(e);
    }
    // set opts object variables
    opts->quiet = quiet;
    opts->verbosity = verbosity;

    return 0;
}

/**
 * Entry point to program
 *
 * @param argc Count of CLI arguments
 * @param argv CLI arguments
 *
 * @return int Status returned to calling program
 */
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
        // loguru::g_internal_verbosity = 1;
        init_loguru(argc, argv); // init loguru with raw cli args
    }
    std::shared_ptr<options> opts = std::make_shared<options>();
    // if (parse_opts(argc, argv, opts) != 0) {
    //     exit(EXIT_FAILURE);
    // }
    if (int p = parse_args(argc, argv, opts); p != 0) {
        exit(p);
    }

    loguru::g_stderr_verbosity = loguru::get_verbosity_from_name(opts->verbosity.data());
    if (opts->quiet) loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    LOG_F(2, "{}", opts);
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
