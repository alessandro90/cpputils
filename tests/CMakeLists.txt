cmake_minimum_required (VERSION 3.10.0)

set(CPPUTILS_ENABLE_COVERAGE FALSE CACHE BOOL "Enable coverage")
if (CPPUTILS_ENABLE_COVERAGE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage -O0 -g")
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wfloat-equal -Wconversion -Wsign-conversion)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        devel
)

FetchContent_MakeAvailable(Catch2)

set(TEST_PATH ${PROJECT_SOURCE_DIR}/tests)
include_directories(${PROJECT_SOURCE_DIR}/src/include)

set(TEST_FILES
${TEST_PATH}/number_is_error_result_test.cpp
${TEST_PATH}/number_test.cpp
${TEST_PATH}/opt_ext_test.cpp
${TEST_PATH}/enumerate_test.cpp
${TEST_PATH}/zip_test.cpp
${TEST_PATH}/zip_with_test.cpp
${TEST_PATH}/tovector_test.cpp
${TEST_PATH}/typelist_test.cpp
)

add_executable(cpputils_test ${TEST_FILES})
target_link_libraries(cpputils_test PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(cpputils_test)

add_test(NAME cpputils_test COMMAND cpputils_test)
