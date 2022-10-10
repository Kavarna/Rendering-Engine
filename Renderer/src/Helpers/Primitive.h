#pragma once 

#include <Jnrlib.h>

#include "Ray.h"
#include "HitPoint.h"


class Primitive
{
public:
    virtual std::optional<HitPoint> IntersectRay(Ray const&) = 0;
};
