#include <filesystem>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
// #include <regex>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <vector>
#ifndef LOG_IS_ON
#define LOG_IS_ON(verbosity) ((loguru::Verbosity_##verbosity) <= loguru::current_verbosity_cutoff())
#endif
#define DEBUG_MODE 0 // Automatically output console logs by default

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
    VLOG_F(1, "Terminal size: %dx%d", tsize->cols, tsize->lines);
}

std::string get_file_contents(const char* fpath)
{
    std::ifstream file(fpath);
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    } else {
        LOG_F(ERROR, "File '%s' not found.", fpath);
        return "";
    }
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

std::string fmt_list(std::string& raw)
{
    std::vector<std::string> words;
    tokenize(raw, words, " \n");
    if (LOG_IS_ON(2)) {
        for (size_t i = 0; i < words.size(); ++i) {
            VLOG_S(2) << i << ' ' << words[i];
        }
    }
    return raw;
}

int main(int argc, char** argv)
{
#if DEBUG_MODE == 1
    // Automatically add logging to args for convenience
    std::vector<std::string> arguments(argv, argv + argc);
    arguments.emplace(arguments.begin() + 1, "-v");
    arguments.emplace(arguments.begin() + 2, "INFO");
    std::vector<char*> args; // convert vector back to char**
    for (auto& str : arguments) {
        args.push_back(&str.front());
    }
    int arg_ct = args.size();
    init_loguru(arg_ct, args.data());
#else
    // init loguru with raw cli args
    init_loguru(argc, argv);
#endif
    std::filesystem::path fpath(std::getenv("HOME"));
    fpath.append("Dropbox").append("todo").append("todo.txt");
    VLOG_S(1) << "File path: " << fpath;
    std::string raw(get_file_contents(fpath.c_str()));
    std::cout << fmt_list(raw);
}
