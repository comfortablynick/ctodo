#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include <regex>
#include <string>

inline std::string expand_env(std::string text)
{
    static const std::regex env_re{R"--(\$\{([^}]+)\})--"};
    std::smatch match;
    while (std::regex_search(text, match, env_re)) {
        auto const from = match[0];
        auto const var_name = match[1].str().c_str();
        text.replace(from.first, from.second, std::getenv(var_name));
    }
    return text;
}

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

int main(int argc, char** argv)
{
    // init loguru
    loguru::g_colorlogtostderr = true;
    loguru::g_flush_interval_ms = 100;
    if (auto tsize = getTermSize(); tsize->cols < 200) {
        LOG_F(1, "Log output adjusted for term size: %dx%d", tsize->cols, tsize->lines);
        loguru::g_preamble_thread = false;
        loguru::g_preamble_date = false;
        loguru::g_preamble_time = false;
        if (tsize->cols < 100) {
            loguru::g_preamble_file = false;
        }
    }
    loguru::init(argc, argv);

    auto fpath = expand_env("${HOME}/Dropbox/todo/todo.txt");
    VLOG_S(1) << "File path: " << fpath;
    std::ifstream file(fpath);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            std::cout << line << '\n';
        }
    } else {
        std::cerr << "File not found." << std::endl;
    }
}
