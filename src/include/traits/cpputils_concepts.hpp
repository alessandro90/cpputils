#ifndef CPPUTILS_CONCEPTS_HPP
#define CPPUTILS_CONCEPTS_HPP

#include "is_specialization_of.hpp"
#include <type_traits>


namespace cpputils {
template <typename T, template <typename...> typename Target>
concept an = is<std::remove_cvref_t<T>>::template specialization_of_v<Target>;
}  // namespace cpputils


#endif
