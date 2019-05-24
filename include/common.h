#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>

/// @brief Data structure to hold common program options
struct options
{
    std::string cmd, verbosity;
    bool quiet, getline;
};

/// Data structure for terminal cols and lines
struct termsize
{
    unsigned cols, lines;
};
std::shared_ptr<termsize> getTermSize();

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
std::string prettify(int, char**);

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
    const std::string setFg(Ansi::Color);
    const std::string setFg(unsigned int);
    const std::string setBg(Ansi::Color);
    const std::string reset();
} // namespace Ansi
