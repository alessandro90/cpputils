#ifndef CPPUTILS_SYSTEM_MACROS_HPP
#define CPPUTILS_SYSTEM_MACROS_HPP

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define CPPUTILS_WINDOWS_PLATFORM
#endif

#endif