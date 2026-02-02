#ifndef UTILS_H
#define UTILS_H

#include <cstdio>
#include <string>
#include <string_view>
#include <filesystem>
#include <cassert>
#include <cstring>

namespace open_char {

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
    char *buf = static_cast<char *>(calloc(1, sz));
    int sz2 = std::sprintf(buf, format, format_arg(args)...);
    assert(sz == sz2);
    std::string rv = std::string(buf);
    free(buf);
    return rv;
}

template<typename... Args>
std::string sprintf(std::string format, Args&&... args) {
    int sz = std::snprintf(nullptr, 0, format, format_arg(args)...);
    char *buf = static_cast<char *>(calloc(1, sz));
    int sz2 = std::sprintf(buf, format.c_str(), format_arg(args)...);
    assert(sz == sz2);
    std::string rv = std::string(buf);
    free(buf);
    return rv;
}

template<typename... Args>
void error(std::string_view fmt, const Args&... args)
{
    printf("Error: ");
    printf(fmt, args...);
    printf("\n");
}

template<typename... Args>
void info(std::string_view fmt, const Args&... args)
{
    printf("Info: ");
    printf(fmt, args...);
    printf("\n");
}

template<typename... Args>
void warning(std::string_view fmt, const Args&... args)
{
    printf("Warning: ");
    printf(fmt, args...);
    printf("\n");
}

}

#endif