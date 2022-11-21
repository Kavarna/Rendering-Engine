#pragma once 

#include "Jnrlib.h"

namespace Jnrlib
{
    Jnrlib::Direction GetRandomPointInUnitSphere();
    Jnrlib::Direction GetRandomDirectionInUnitSphere();
    Jnrlib::Direction GetRandomDirectionInHemisphere(Jnrlib::Direction const& normal);
    Jnrlib::Direction GetRandomPointInHemisphere(Jnrlib::Direction const& normal);

    Jnrlib::Direction GetRandomInUnitDisk();
}
