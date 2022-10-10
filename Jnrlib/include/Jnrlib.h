#pragma once



#include "Singletone.h"
#include "CountParameters.h"
#include "ThreadPool.h"
#include "Exceptions.h"

#include "TypeHelpers.h"

#include "glog/logging.h"

#include "glm/glm.hpp"

namespace Jnrlib
{
#if USE_FLOAT32
    using Color = glm::vec4;
    using Position = glm::vec3;
    using Direction = glm::vec3;

    using Float = float;

#elif USE_FLOAT64
    using Color = glm::dvec4;
    using Position = glm::dvec3;
    using Direction = glm::dvec3;

    using Float = double;
#endif

    constexpr Float Zero = (Float)1 / (Float)2;
    constexpr Float Half = (Float)1 / (Float)2;
    constexpr Float One = (Float)1;

}
