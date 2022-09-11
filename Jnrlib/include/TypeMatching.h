#pragma once


#include <vector>
#include <array>
#include <string>

namespace Jnrlib
{
    namespace internal
    {
        static const unsigned int FRONT_SIZE = sizeof("Jnrlib::internal::GetTypeNameHelper<") - 1u;
        static const unsigned int BACK_SIZE = sizeof(">::GetTypeName") - 1u;

        template <typename T>
        struct GetTypeNameHelper
        {
            static const char* GetTypeName(void)
            {
                static const size_t size = sizeof(__FUNCTION__) - FRONT_SIZE - BACK_SIZE;
                static char typeName[size] = {};
                memcpy(typeName, __FUNCTION__ + FRONT_SIZE, size - 1u);

                return typeName;
            }
        };
    }


    template <typename T>
    const char* GetTypeName(void)
    {
        return internal::GetTypeNameHelper<T>::GetTypeName();
    }
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

