cmake_minimum_required (VERSION 3.10.0)

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
${TEST_PATH}/number_test.cpp
${TEST_PATH}/opt_ext_test.cpp
${TEST_PATH}/enumerate_test.cpp
${TEST_PATH}/zip_test.cpp
${TEST_PATH}/zip_with_test.cpp
${TEST_PATH}/tovector_test.cpp
)

add_executable(cpputils_test ${TEST_FILES})
target_link_libraries(cpputils_test PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(cpputils_test)
