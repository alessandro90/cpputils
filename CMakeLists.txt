cmake_minimum_required (VERSION 3.10.0)

project(CPPUTILS)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wfloat-equal -Wconversion -Wsign-conversion)


add_definitions(${COMPILER_DEFINITIONS})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        devel
)

FetchContent_MakeAvailable(Catch2)

set(TEST_PATH tests)

include_directories(${PROJECT_SOURCE_DIR}/src/include)

set(TEST_FILES
${TEST_PATH}/dummy.cpp
)

add_executable(test_program ${TEST_FILES})
target_link_libraries(test_program PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(test_program)

enable_testing()
