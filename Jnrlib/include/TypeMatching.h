#pragma once


#include <vector>
#include <array>
#include <string>

namespace Jnrlib
{

#define ADD_TYPE_NAME(name) \
template <> struct GetTypeName<name>\
{\
    const char* value = #name;\
};

    template <typename T>
    struct GetTypeName
    {
    };

    template <>
    struct GetTypeName<std::string>
    {
        static constexpr const char* value = "std::string";
    };


    // ADD_TYPE_NAME(std::string);
    ADD_TYPE_NAME(int);

}

namespace Detail {
    template <typename T>
    auto IsIterableImpl(int) -> decltype (
        begin(std::declval<T&>()) != end(std::declval<T&>()), // begin/end and operator !=
        void(), // Handle evil operator ,
        ++std::declval<decltype(begin(std::declval<T&>()))&>(), // operator ++
        void(*begin(std::declval<T&>())), // operator*
        std::true_type{});

    template <typename T>
    std::false_type IsIterableImpl(...);
}

template <typename T>
using IsIterable = decltype(Detail::IsIterableImpl<T>(0));


template <typename T>
struct IsDefaultConstructible {
    template <typename U>
    static constexpr uint32_t SFINAE(decltype(U())*);
    template <typename U>
    static constexpr uint8_t SFINAE(...);

    static const bool value = sizeof(SFINAE<T>(nullptr)) == sizeof(int32_t);
};

template <typename T, typename U>
struct GetTypeInsideGenericList
{

};

// To be added as needed

