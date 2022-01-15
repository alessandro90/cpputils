cmake_minimum_required (VERSION 3.10.0)

project(CppUtils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

option(CPPUTILS_ENABLE_TESTING ON)

if (CPPUTILS_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()
