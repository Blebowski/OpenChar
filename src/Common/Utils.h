#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

#include <signal.h>
#include <tcl.h>


namespace open_char {

#define DEBUG_MSG_ENABLED

inline const char*              format_arg(const std::string& s)         { return s.c_str(); }
inline const char*              format_arg(const char* s)                { return s; }
inline char*                    format_arg(char* s)                      { return s; }
inline int                      format_arg(int i)                        { return i; }
inline double                   format_arg(double d)                     { return d; }
inline long                     format_arg(long l)                       { return l; }
inline unsigned                 format_arg(unsigned u)                   { return u; }
inline size_t                   format_arg(size_t u)                     { return u; }
inline const char *             format_arg(std::filesystem::path wave)   { return wave.c_str(); }


template <typename... Args>
int printf(const char* format, Args&&... args) {
    return std::printf(format, format_arg(args)...);
}

template <typename... Args>
int printf(std::string &format, Args&&... args) {
    return std::printf(format.c_str(), format_arg(args)...);
}

template<typename... Args>
int fprintf(FILE *f, const char* format, Args&&... args) {
    return std::fprintf(f, format, format_arg(args)...);
}

template<typename... Args>
int fprintf(FILE *f, std::string &format, Args&&... args) {
    return std::fprintf(f, format.c_str(), format_arg(args)...);
}

template<typename... Args>
std::string sprintf(const char* format, Args&&... args) {
    int sz = std::snprintf(nullptr, 0, format, format_arg(args)...);
    char *buf = static_cast<char *>(calloc(1, sz + 1));
    int sz2 = std::sprintf(buf, format, format_arg(args)...);
    assert(sz == sz2);
    std::string rv = std::string(buf);
    free(buf);
    return rv;
}

template<typename... Args>
std::string sprintf(std::string format, Args&&... args) {
    int sz = std::snprintf(nullptr, 0, format.c_str(), format_arg(args)...);
    char *buf = static_cast<char *>(calloc(1, sz + 1));
    int sz2 = std::sprintf(buf, format.c_str(), format_arg(args)...);
    assert(sz == sz2);
    std::string rv = std::string(buf);
    free(buf);
    return rv;
}

template<typename... Args>
void fatal(const std::string &fmt, const Args&... args)
{
    std::printf("Fatal: ");
    std::printf(fmt.c_str(), format_arg(args)...);
    std::printf("\n");
    raise(SIGABRT);
}

template<typename... Args>
void error(const std::string &fmt, const Args&... args)
{
    std::printf("Error: ");
    std::printf(fmt.c_str(), format_arg(args)...);
    std::printf("\n");
}

template<typename... Args>
void warning(const std::string &fmt, const Args&... args)
{
    std::printf("Warning: ");
    std::printf(fmt.c_str(), format_arg(args)...);
    std::printf("\n");
}

template<typename... Args>
void info(const std::string &fmt, const Args&... args)
{
    std::printf("Info: ");
    std::printf(fmt.c_str(), format_arg(args)...);
    std::printf("\n");
}

template<typename... Args>
void debug(const std::string &fmt, const Args&... args)
{
#ifdef DEBUG_MSG_ENABLED
    std::printf("Debug: ");
    std::printf(fmt.c_str(), format_arg(args)...);
    std::printf("\n");
#endif
}

#define OPENCHAR_ENUM_ELEM(x) x,
#define OPENCHAR_ENUM_MAP(x) {x, #x},

#define OPENCHAR_ENUM(name, elements)                                                               \
    enum class name { elements(OPENCHAR_ENUM_ELEM) };                                               \
    inline std::string toString(name v) {                                                           \
        using enum name;                                                                            \
        std::unordered_map<name, std::string> map = {elements(OPENCHAR_ENUM_MAP)};                  \
        return map[v];                                                                              \
    }                                                                                               \

#define PRINT_LINE(len) printf("%s\n", std::string(len, '-'));

#define TAB_FPRINTF(size, f, ...) do {                          \
                fprintf(f, "%s", std::string(2 * size, ' '));   \
                fprintf(f, __VA_ARGS__);                        \
            } while (0)

#define IS_POWER_OF_2(x) (((x) > 0) && (((x) & ((x) - 1)) == 0))

#define COMPARE_THRESHOLD 0.000000000001
#define COMPARE_FLOATS(a, b) (((a - b) < COMPARE_THRESHOLD) && ((a - b) > - COMPARE_THRESHOLD))

}

#endif