# CPPUTILS

![CI@master](https://github.com/alessandro90/cpputils/actions/workflows/ci.yml/badge.svg)

This is a repo in which I put stuff I think it could be usefull or interesting to some degree. It hasn't a precise goal or scope.
For now it is a header-only library. Its components are

## [number](src/include/cpputils/types/number.hpp)

A non-promoting number-like class. It is a wrapper around the underlying number.
In debug build, all operations are checked against under and overflow. In release build no checks are performed.
All operations are non promoting.
Bit operations are allowed only on unsigned types.
All casts are explicit.

The class signature is

```cpp
template<typename I>
class number;
```

Available aliases are

```cpp
u8
u16
u32
u64
usize
i8
i16
i32
i64
isize
f32
f64
```

Literal (consteval) operators are defined inside the nested namespace `cpputils::literals` and can be used like so

```cpp
1_u8
10_usize
etc...
```

Also a global `True` and `False` are available in the same namespace for convenience (these are simply `number<bool>{true}` and `number<bool>{false}` respectively).

`std::numeric_limits<T>` is specialized for all possible class templates.

The internal value is availbale via the `val` member function.
Casts can be performed via `static_cast` or the `as<T>` member function.

#### **Example**

```cpp
using namespace cpputils::literals;
constexpr auto i = 11_u16;
assert(i.as<std::int16_t>() == 11);
assert(static_cast<std::uint16_t>(i) == 11);
assert(True);
assert(!False);
```

## Range compatible stuff

### [zip](src/include/cpputils/functional/zip.hpp)/[zip_with](src/include/cpputils/functional/zip_with.hpp)

`zip` takes an arbitrary values of containers (lvalues) and return a view of the zipped values. The shortest container is the limiting one.
`zip` is a (lazy) range, and therefore support the transform syntax only as initial value (just like std::vector)

#### **Example**

```cpp
    using namespace cpputils;
    std::array arr{1, 2, 3};
    std::vector v{10, 20, 30, 40};
    int x[] = {4, 5, 6};
    for (auto [i, j, k] : zip(arr, v, x) {
        std::cout << i << "; " << j << "; " << << k '\n';
    }
    // Will print
    // 1; 10; 4
    // 2; 20; 5
    // 3; 30; 6
    // This is valid

    zip(arr, v, x)
        | std::ranges::transform([](int i) { return i * 2; });
```

`zip_with` is similar to `zip` but it first takes a callable to apply to each of the zipped elements.

#### **Example**

```cpp
    using namespace cpputils;
    auto const sum = [](int a, int b) { return a + b };
    std::array arr{1, 2, 3};
    std::vector v{10, 20, 30, 40};
    for (auto i : zip_with(sum, arr, v)) {
        std::cout << i << '\n';
    }
    // Will print
    // 11
    // 22
    // 33
    // This is valid
    zip_with(sum, arr, v)
        | std::ranges::transform([](int i) { return i * 2; });
```

### [enumerate](src/include/cpputils/functional/enumerate.hpp)

Compatible with transform syntax. Custom types are supported as index (see example).

#### **Example**

```cpp
using namespace cpputils;
using namespace cpputils::literals;

int x[] = {10, 20, 30};
for (auto [index, value] : x | enumerate()) {
    std::cout << index << "; " << value << '\n';
}
// Will print
// 0; 10
// 1; 20
// 2; 30
for (auto [index, value] : x | enumerate(-1)) {
    std::cout << index << "; " << value << '\n';
}
// Will print
// -1; 10
// 0; 20
// 1; 30
for (auto [index, value] : x | enumerate(10_u32)) {
    std::cout << index << "; " << value << '\n';
}
// Will print
// 10; 10
// 11; 20
// 12; 30

auto const divide = [](int i) { return i / 2; };
// This is valid
x | std::ranges::views::transform(divide) | enumerate();
```

### [to_vector](src/include/cpputils/functional/tovector.hpp)

Converts a range to a std::vector.

#### **Example**

```cpp
using namespace cpputils;
assert(std::ranges::iota(5) | to_vector() == std::vector{0, 1, 2, 3, 4});
```

## [Utilities that extend the use of std::optional](src/include/cpputils/functional/opt_ext.hpp)

C++23 will add [some monadic operations to optional](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0798r6.html), so probably some of these utilities will be less interesting in the future.

### map

Takes a callable and an arbitrary number of optionals. If all optionals have values call the callable with all the contained values, otherwise return an empty optional. The callable can return itself an optional or not.

#### **Example**

```cpp
using namespace cpputils;

std::optional a{1};
std::optional b{2};
map([](int i) { return i + 1 }, a); // == std::optional{2}
map([](int i, int j) { return i + j }, a, b); // == std::optional{3}
map([](int i, int j) { return std::optional{i + j} }, a, b); // == std::optional{3}
map([](int i, int j) { return std::optional{i + j} }, a, std::optional<int>{}); // == std::optional{}
map([](int i, int j) { return std::optional{i + j} }, std::optional<int>{}, std::optional<int>{}); // == std::optional{}
```

### transform

Takes an optional and a callable and apply the callable to the optional's contained element, if any. Otherwise return an empty optional. The callable can itself return an optional (just like for map), the result is flattened. For a more readable syntax use the `>>` operator.

#### **Example**

```cpp
using namespace cpputils;

std::optional{1}
    >> transform([](int i) { return i + 1; })
    >> transform([](int i) { return i * 2; }); // == std::optional{4}

std::optional{1}
    >> transform([](int i) { return i + 1; })
    >> transform([](int i) { return std::optional{i * 2}; }); // == std::optional{4}
```

### if_value/or_else

`if_value` takes and optional and a callable and apply the callable to the optional's contained element if any. Return a reference to the optional.
`or_else` takes and optional and a callable and call the callable if the optional is empty. Return a reference to the optional.

#### **Example**

```cpp
using namespace cpputils;

auto const print = [](int i) { std::printf("%d\n", i); };
auto const panic = []() { throw std::invalid_argument{"Should not be empty"}; };

std::optional{}
    >> if_value(print)
    >> or_else(panic);
```

### unwrap/unwrap_or/unwrap_or_else

`unwrap` calls `value` on the optional.
`unwrap_or` calls `value_or` on the optional.
`unwrap_or_else` calls `value` on the optional if it has a value, otherwise calls the provided fallback.

#### **Example**

```cpp
using namespace cpputils;

auto const get_good_default = []() { return 42; };

std::optional{1} >> unwrap(); // == 1
std::optional{} >> unwrap_or(10); // == 10
std::optional{} >> unwrap_or_else(get_good_default); // == 42
```

## [typelist](src/include/cpputils/traits/typelist.hpp)

An exercise in template metaprogramming. I tried to mimic some of the most common functional features applied to types.
The basic type is the typelist which is just en empty variadic struct.

The "functions" implemented are (in progress, for the full list see the [test file](tests/typelist_test.cpp))

| Function | Signature                                                                      | Description | Usage                                        | 'Closure' form                 |
| -------- | ------------------------------------------------------------------------------ | ----------- | -------------------------------------------- | ------------------------------ |
| `inner`  | `inner :: typelist<T> -> T`                                                    |             | `inner<typelist<T>>::type`                   |                                |
| `head`   | `head :: typelist<T...> -> typelist<T>`                                        |             | `head<typelist<T...>>::list`                 | `head<>::list<typelist<T...>>` |
| `tail`   | `tail :: typelist<T...> -> typelist<T>`                                        |             | `tail<typelist<T...>>::list`                 | `tail<>::list<typelist<T...>>` |
| `join`   | `join :: typelist<T...> -> typelist<E...> -> ... -> typelist<T..., E..., ...>` |             | `join<typelist<T...>, typelist<E...>>::list` |                                |

## Utility traits and concepts

### is_specialization_of/is_specialization_of_v/an

To be used for template classes. Does not work for non-type template parameters.
`an` is just the concept version of is_specialization_of.

#### **Example**

```cpp
using namespace cpputils;

static_assert(is_specialization_of_v<std::optional<int>, std::optional>);

auto f(an<std::optional> auto opt) { /* ... */ }

```

### minimal_incrementable

A very relaxed incrementable requirement.

### tl::specializable/tl::predicate/tl::transformation

Concepts for template metaprogramming.
`tl::predicate` is satisfied for every type `T` such that exists `T::value` and it is convertible to bool.
`tl::transformation` is satisfied for every type `T` such that `typename T::type` exists.

#### **Example**

```cpp
using namespace cpputils;

static_assert(tl::specializable<std::tuple, int, char>); // success
static_assert(tl::specializable<std::optional, int, char>); // fail
```

## Details

The tests are downloaded automatically in the build folder and are the only buildable thing. So doing `make` will build them. All typelist tests are compile-time checks, so if a test fail you get a compile-time error.

For now only gcc-11 on Ubuntu is used by the CI.
