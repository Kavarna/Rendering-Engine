#pragma once

#include <entt/entt.hpp>
#include <Jnrlib.h>

namespace Common
{
    class Ray;
    class HitPoint;
    class Scene;
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
        /* TODO: Make this system as part of the scene */
        std::optional<HitPoint> IntersectRay(Ray&, entt::registry &, Common::Scene const* scene);

    };
}