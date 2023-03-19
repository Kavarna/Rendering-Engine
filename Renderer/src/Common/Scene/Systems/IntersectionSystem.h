#pragma once

#include <entt/entt.hpp>
#include "Ray.h"
#include "HitPoint.h"
#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/SphereComponent.h"

namespace Common::Systems
{
    class Intersection : public Jnrlib::ISingletone<Intersection>
    {
        MAKE_SINGLETONE_CAPABLE(Intersection);
    private:
        Intersection();
        ~Intersection();

    public:
        std::optional<HitPoint> IntersectRay(Ray const&, entt::registry &);

    private:
        std::optional<HitPoint> RaySphereIntersection(
            Ray const& r, Components::Base const& b, Components::Sphere const& s);
    };
}