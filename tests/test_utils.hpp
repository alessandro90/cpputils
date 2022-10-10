#ifndef CPPUTILS_TEST_UTILS_HPP
#define CPPUTILS_TEST_UTILS_HPP

namespace cpputils::test_utils {
struct move_detector {
    move_detector() = default;
    move_detector(move_detector const &) = default;
    move_detector &operator=(move_detector const &) = default;
    move_detector(move_detector &&other) noexcept {
        other.moved = true;
    };
    move_detector &operator=(move_detector &&other) noexcept {
        other.moved = true;
        return *this;
    }
    ~move_detector() = default;

    bool moved{false};
};
}  // namespace cpputils::test_utils

#endif
