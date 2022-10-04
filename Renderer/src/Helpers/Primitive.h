#pragma once 

#include <Jnrlib.h>

#include "Ray.h"
#include "HitPoint.h"


class Primitive
{
public:
    virtual bool IntersectRay(Ray const&, HitPoint& hp) = 0;
};
