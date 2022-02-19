# CPPUTILS

This is a repo in which I put stuff I think it could be usefull or interesting to some degree. It hasn't a precise goal or scope.
For now it is a header-only library. Its components are

## [number](src/include/cpputils/types/number.hpp)

## Range compatible stuff

### [zip](src/include/cpputils/functional/zip.hpp)/[zip_with](src/include/cpputils/functional/zip_with.hpp)

### [enumerate](src/include/cpputils/functional/enumerate.hpp) (pipe syntax compatible)

### [to_vector](src/include/cpputils/functional/tovector.hpp)

Converts a range to a std::vector.

## [Utilities that extend the use of std::optional](src/include/cpputils/functional/opt_ext.hpp)

C++23 will add [some monadic operations to optional](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0798r6.html), so probably some of these utilities will be less interesting in the future.

## [typelist](src/include/cpputils/traits/typelist.hpp)

An exercise in template metaprogramming. I tried to mimic some of the most common functional features applied to types.
The basic type is the typelist which is just en empty variadic struct.

The "functions" implemented are

| Function | Signature                                                                      | Description | Usage                                        | 'Closure' form                 |
| -------- | ------------------------------------------------------------------------------ | ----------- | -------------------------------------------- | ------------------------------ |
| `inner`  | `inner :: typelist<T> -> T`                                                    |             | `inner<typelist<T>>::type`                   |                                |
| `head`   | `head :: typelist<T...> -> typelist<T>`                                        |             | `head<typelist<T...>>::list`                 | `head<>::list<typelist<T...>>` |
| `tail`   | `tail :: typelist<T...> -> typelist<T>`                                        |             | `tail<typelist<T...>>::list`                 | `tail<>::list<typelist<T...>>` |
| `join`   | `join :: typelist<T...> -> typelist<E...> -> ... -> typelist<T..., E..., ...>` |             | `join<typelist<T...>, typelist<E...>>::list` |                                |

## Utility traits and concepts

## Details

The tests are downloaded automatically in the build folder and are the only buildable thing. So doing `make` will build them. All typelist tests are compile-time checks, so if a test fail you get a compile-time error.
