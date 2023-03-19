#include "IntersectionSystem.h"

using namespace Common;
using namespace Components;
using namespace Systems;

Intersection::Intersection()
{ }

Intersection::~Intersection()
{ }

std::optional<Common::HitPoint> Intersection::IntersectRay(Ray const& r, entt::registry& objects)
{

    std::optional<HitPoint> finalHitPoint;
    Jnrlib::Float closestHitSoFar = std::numeric_limits<Jnrlib::Float>::max();
    {
        /* Perform ray-sphere intersections */
        auto view = objects.group<const Base>(entt::get<const Sphere>);
        for (auto const& [entity, base, sphere] : view.each())
        {
            std::optional<HitPoint> hp = RaySphereIntersection(r, base, sphere);

            if (!hp.has_value())
                continue;

            Jnrlib::Float intersectionPoint = hp->GetIntersectionPoint();
            if (intersectionPoint < closestHitSoFar)
            {
                CHECK(base.entityPtr != nullptr) << "Base doesn't include an entity pointer";
                hp->SetEntity(base.entityPtr);

                closestHitSoFar = intersectionPoint;
                finalHitPoint = hp.value();
            }
        }
    }
    return finalHitPoint;
}

std::optional<HitPoint> Intersection::RaySphereIntersection(Ray const& r, Base const& b, Sphere const& s)
{
    CHECK(b.scaling.x == b.scaling.y && b.scaling.y == b.scaling.z) << "A non-sphere was specified to this function";

    float radius = b.scaling.x;
    Jnrlib::Direction toCenter = b.position - r.GetStartPosition();
    Jnrlib::Float tca = glm::dot(toCenter, r.GetDirection());
    if (tca < Jnrlib::Zero)
        return std::nullopt;

    Jnrlib::Float d2 = glm::dot(toCenter, toCenter) - tca * tca;
    if (d2 > radius * radius)
        return std::nullopt;

    Jnrlib::Float thc = sqrt(radius * radius - d2);

    Jnrlib::Float minPoint = Jnrlib::Zero, maxPoint = Jnrlib::Zero, intersectionPoint = Jnrlib::Zero;
    /* Compute the intersection point */
    {
        Jnrlib::Float firstPoint = tca - thc;
        Jnrlib::Float secondPoint = tca + thc;

        if (firstPoint > secondPoint)
        {
            minPoint = secondPoint;
            maxPoint = firstPoint;
        }
        else
        {
            minPoint = firstPoint;
            maxPoint = secondPoint;
        }
        if (minPoint >= 0.0)
        {
            intersectionPoint = minPoint;
        }
        else if (maxPoint > 0.0)
        {
            intersectionPoint = maxPoint;
        }
        else
        {
            return std::nullopt;
        }
    }

    /* Compute the normal */
    Jnrlib::Position hitPosition = r.At(intersectionPoint);
    Jnrlib::Direction normal = hitPosition - b.position;

    if (fabs(intersectionPoint) < Jnrlib::EPSILON)
        return std::nullopt;

    /* Fill the hitpoint */
    HitPoint hp = {};
    hp.SetIntersectionPoint(intersectionPoint);
    hp.SetMaterial(s.material);
    if (glm::dot(normal, r.GetDirection()) < 0)
    {
        /* We're hitting the sphere in the front */
        hp.SetNormal(normal);
        hp.SetFrontFace(true);
    }
    else
    {
        /* We're hitting the sphere in the back, so the normal has to be reversed */
        hp.SetNormal(-normal);
        hp.SetFrontFace(false);
    }

    return hp;
}
