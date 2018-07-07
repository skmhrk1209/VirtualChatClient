#pragma once

#include <initializer_list>
#include <iostream>
#include <tuple>
#include <utility>

template <typename T>
using Invoke = typename T::Type;

template <std::size_t...>
struct IndexSequence {
    using Type = IndexSequence;
};

template <typename S1, typename S2>
struct Concat;

template <std::size_t... I1, std::size_t... I2>
struct Concat<IndexSequence<I1...>, IndexSequence<I2...>>
    : IndexSequence<I1..., (sizeof...(I1) + I2)...> {};

template <std::size_t N>
struct MakeIndexSequence
    : Invoke<Concat<Invoke<MakeIndexSequence<(N >> 1)>>,
                    Invoke<MakeIndexSequence<N - (N >> 1)>>>> {};

template <>
struct MakeIndexSequence<0> : IndexSequence<> {};

template <>
struct MakeIndexSequence<1> : IndexSequence<0> {};

// Apply a function to each element of a tuple.
template <typename Tuple, typename Function, std::size_t... Indices>
void forEachImpl(Tuple &&tuple, Function &&function,
                 IndexSequence<Indices...>) {
    // Don't unpack in function parameters because it's not guaranteed each
    // parameter is evaluated from left to right.
    // [](auto&&...)
    // {}((function(std::get<Indices>(std::forward<Tuple>(tuple))), 0)...);

    // So, unpack in initializer list.
    static_cast<void>(std::initializer_list<int>{
        (function(std::get<Indices>(std::forward<Tuple>(tuple))), 0)...});
}

// Apply a function to each element of a tuple.
template <typename Tuple, typename Function>
void forEach(Tuple &&tuple, Function &&function) {
    constexpr std::size_t Size(
        std::tuple_size<typename std::remove_reference<Tuple>::type>::value);
    forEachImpl(std::forward<Tuple>(tuple), std::forward<Function>(function),
                MakeIndexSequence<Size>());
}

struct Scaner {
    Scaner(std::istream &is) : istream(is) {}

    template <typename T>
    void operator()(T &object) {
        istream >> object;
    }

    std::istream &istream;
};

struct Printer {
    Printer(std::ostream &os) : ostream(os) {}

    template <typename T>
    void operator()(const T &object) {
        ostream << object << " ";
    }

    std::ostream &ostream;
};

template <typename... Types>
std::istream &operator>>(std::istream &istream, std::tuple<Types...> &tuple) {
    forEach(tuple, Scaner(istream));

    return istream;
}

template <typename... Types>
std::ostream &operator<<(std::ostream &ostream,
                         const std::tuple<Types...> &tuple) {
    forEach(tuple, Printer(ostream));

    return ostream;
}