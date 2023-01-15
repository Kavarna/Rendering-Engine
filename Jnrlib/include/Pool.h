#pragma once


#include <list>
#include <utility>
#include <glog/logging.h>


namespace Jnrlib
{
    template <class type>
    class Pool
    {
    public:
        template<typename ... Args>
        Pool(uint32_t maxSize, Args&&... args) :
            mMaxSize(maxSize)
        {
            CHECK(maxSize > 1) << "A pool must have more than one object";
            // Construct enough objects
            for (uint32_t i = 0; i < maxSize; ++i)
            {
                if constexpr (sizeof...(Args) == 0)
                {
                    mPool.emplace_back();
                }
                else
                {
                    mPool.emplace_back(std::forward<Args>(args)...);
                }
            }
            mCurrentObject = mPool.begin();
            mNextObject = mPool.begin()++;
        }
        ~Pool() = default;

        type& GetCurrent()
        {
            return *mCurrentObject;
        }

        void Advance()
        {
            mCurrentObject = mNextObject;
            mNextObject++;
            if (mNextObject == mPool.end())
                mNextObject = mPool.begin();
        }

        void Clear()
        {
            mPool.clear();
        }

    private:
        uint32_t mMaxSize;
        std::list<type> mPool;

        std::list<type>::iterator mCurrentObject, mNextObject;

    };
}