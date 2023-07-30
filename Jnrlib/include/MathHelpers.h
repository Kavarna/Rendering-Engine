#pragma once

#include <stdint.h>
#include <cmath>

#include "TypeHelpers.h"

namespace Jnrlib
{
    enum class Axis
    {
        X = 0,
        Y = 1,
        Z = 2,
        COUNT = 3
    };

    inline Axis operator+(Axis lhs, Axis rhs)
    {
        return Axis(((uint32_t)lhs + (uint32_t)rhs) % (uint32_t)Axis::COUNT);
    }

    // Normalize a value in the range [min - max]
    template<typename T, typename U>
    inline T NormalizeRange(U x, U min, U max)
    {
        return T(x - min) / T(max - min);
    }

    // Shift and bias a value into another range.
    template<typename T, typename U>
    inline T ShiftBias(U x, U shift, U bias)
    {
        return T(x * bias) + T(shift);
    }

    /***************************************************************************
    * These functions were taken from the MiniEngine.
    * Source code available here:
    * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
    * Retrieved: January 13, 2016
    **************************************************************************/
    template <typename T>
    inline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    template <typename T>
    inline bool IsAligned(T value, size_t alignment)
    {
        return 0 == ((size_t)value & (alignment - 1));
    }

    template <typename T>
    inline T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }
    /***************************************************************************/

    /**
    * Round up to the next highest power of 2.
    * @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    * @retrieved: January 16, 2016
    */
    inline uint32_t NextHighestPow2(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;

        return v;
    }

    /**
    * Round up to the next highest power of 2.
    * @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    * @retrieved: January 16, 2016
    */
    inline uint64_t NextHighestPow2(uint64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;

        return v;
    }

    inline bool Quadratic(Float a, Float b, Float c, Float* t1, Float* t2)
    {
        Float delta = b * b - 4 * a * c;
        if (delta < 0)
        {
            return false;
        }
        else if (delta == 0)
        {
            if (*t1) *t1 = -0.5f * (-b) / (a);
            if (*t2) *t2 = *t1;
            return true;
        }
        else
        {
            Float q = (b > 0) ?
                -0.5f * (b + sqrt(delta)) :
                -0.5f * (b - sqrt(delta));
            *t1 = q / a;
            *t2 = c / q;
            if (*t1 > *t2) std::swap(*t1, *t2);
        }
        return true;
    }
   
    inline Axis GetMaximumAxis(glm::vec2 v)
    {
        return v.x > v.y ? Axis::X : Axis::Y;
    }

    inline Axis GetMaximumAxis(glm::vec3 v)
    {
        return (v.x > v.y) ? ((v.x > v.z) ? Axis::X : Axis::Z) : ((v.y > v.z) ? Axis::Y : Axis::Z);
    }

    inline glm::vec2 Permute(glm::vec2 p, Axis x, Axis y)
    {
        return glm::vec2(p[(uint32_t)x], p[(uint32_t)y]);
    }

    inline glm::vec3 Permute(glm::vec3 p, Axis x, Axis y, Axis z)
    {
        return glm::vec3(p[(uint32_t)x], p[(uint32_t)y], p[(uint32_t)z]);
    }

    template <typename T>
    inline T RemapValueFromIntervalToInterval(T value, T from_start, T from_end, T to_start, T to_end)
    {
        CHECK(from_start < from_end);
        CHECK(to_start < to_end);

        value = std::max(from_start, std::min(from_end, value));

        // Calculate the remapped value
        double oldRange = from_end - from_start;
        double newRange = to_end - to_start;
        return T((value - from_start) / oldRange * newRange + to_start);
    }
}