#ifndef CPPUTILS_VARIANT_VISITOR_HPP
#define CPPUTILS_VARIANT_VISITOR_HPP

namespace cpputils {
template <typename... Vs>
struct visitor : Vs... {
    using Vs::operator()...;
};

// Just in case
template <typename... Vs>
visitor(Vs...) -> visitor<Vs...>;
}  // namespace cpputils

#endif
