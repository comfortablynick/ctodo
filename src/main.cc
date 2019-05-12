#include <fstream>
#include <iostream>
// #include <loguru.hpp>
#include "easylogging++.h"
#include <regex>
#include <string>
INITIALIZE_EASYLOGGINGPP

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

int main(int argc, char** argv)
{
    // loguru::init(argc, argv);
    START_EASYLOGGINGPP(argc, argv);
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Verbose, el::ConfigurationType::Format,
                    "%level%vlevel|%fbase:%line|%msg");
    el::Loggers::reconfigureLogger("default", defaultConf);
    auto fpath = expand_env("${HOME}/Dropbox/todo/todo.txt");
    VLOG(1) << "File path: " << fpath;
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
