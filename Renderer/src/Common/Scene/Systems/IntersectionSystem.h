#pragma once

#include <entt/entt.hpp>
#include <Jnrlib.h>

namespace Common
{
    class Ray;
    class HitPoint;
}

namespace Common::Components
{
    struct Base;
    struct Sphere;
}

namespace Common::Systems
{
    class Intersection : public Jnrlib::ISingletone<Intersection>
    {
        MAKE_SINGLETONE_CAPABLE(Intersection);
    private:
        Intersection();
        ~Intersection();

    public:
        std::optional<HitPoint> IntersectRay(Ray&, entt::registry &);

    };
}