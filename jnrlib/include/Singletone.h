#pragma once

#include <memory>
#include <mutex>
#include "CountParameters.h"
#include "TypeMatching.h"

namespace Jnrlib
{
    template <typename type> class ISingletone
    {
    protected:
        ISingletone()
        { };
        virtual ~ISingletone()
        {
            Destroy();
        };

    public:
        template <typename... Args> constexpr static type* Get(Args... args)
        {
            if constexpr (CountParameters<Args...>::value == 0)
            {
                if constexpr (IsDefaultConstructible<type>::value)
                {
                    std::call_once(m_singletoneFlags,
                                   [&]
                    {
                        m_singletoneInstance = new type(args...);
                    });
                    return m_singletoneInstance;
                }
                else
                {
                    if (!m_singletoneInstance)
                    {
                        throw std::exception("Can't use a singletone that has not been created");
                    }
                    return m_singletoneInstance;
                }
            }
            else
            {
                if (m_singletoneInstance)
                {
                    throw std::exception("Singletone should be created only once");
                }
                std::call_once(m_singletoneFlags,
                               [&]
                {
                    m_singletoneInstance = new type(args...);
                });
                return m_singletoneInstance;
            }
        }

        static void Destroy()
        {
            if (m_singletoneInstance)
            {
                auto ptr = m_singletoneInstance; // If we do it this way, we can safely
                                                 // call reset() from destructor too
                m_singletoneInstance = nullptr;
                delete ptr;
            }
        };

    private:
        static type* m_singletoneInstance;
        static std::once_flag m_singletoneFlags;
    };
}

#define MAKE_SINGLETONE_CAPABLE(name) \
friend class Jnrlib::ISingletone<name>;\
friend struct IsDefaultConstructible<name>;\

template <typename type>
type* Jnrlib::ISingletone<type>::m_singletoneInstance = nullptr;
template <typename type>
std::once_flag Jnrlib::ISingletone<type>::m_singletoneFlags;