#pragma once 

#include <Jnrlib.h>

#include "Ray.h"
#include "HitPoint.h"

namespace Common
{
    class Primitive
    {
    public:
        virtual std::optional<HitPoint> IntersectRay(Ray const&) = 0;
    };
}
