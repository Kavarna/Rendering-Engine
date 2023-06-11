#include "IntersectionSystem.h"

#include "Ray.h"
#include "HitPoint.h"
#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/SphereComponent.h"

#include "Material/Lambertian.h"

using namespace Common;
using namespace Components;
using namespace Systems;
using namespace Jnrlib;

/* Helpers */
inline Float gamma(int n)
{
    return (n * EPSILON) / (1 - n * EPSILON);
}

std::optional<HitPoint> RaySphereIntersection(Ray const& r, Base const& b, Sphere const& s)
{
    float radius = s.radius;
    Direction toCenter = b.position - r.origin;
    Float tca = glm::dot(toCenter, r.direction);
    if (tca < Zero)
        return std::nullopt;

    Float d2 = glm::dot(toCenter, toCenter) - tca * tca;
    if (d2 > radius * radius)
        return std::nullopt;

    Float thc = sqrt(radius * radius - d2);

    Float minPoint = Zero, maxPoint = Zero, intersectionPoint = Zero;
    /* Compute the intersection point */
    {
        Float firstPoint = tca - thc;
        Float secondPoint = tca + thc;

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
    Position hitPosition = r.At(intersectionPoint);
    Direction normal = hitPosition - b.position;

    if (fabs(intersectionPoint) < EPSILON || intersectionPoint > r.maxT)
        return std::nullopt;

    /* Fill the hitpoint */
    HitPoint hp = {};
    hp.SetIntersectionPoint(intersectionPoint);
    hp.SetMaterial(s.material);
    if (glm::dot(normal, r.direction) < 0)
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

bool RayAABBIntersectionSlow(Ray const& r, BoundingBox const& b, Float* hitt0 = nullptr, Float* hitt1 = nullptr)
{
    Float t0 = Zero, t1 = Infinity;
    {
        /* Check against X plane */
        Float invRay = One / r.direction.x;
        Float tNear = (b.pMin.x - r.origin.x) * invRay;
        Float tFar = (b.pMax.x - r.origin.x) * invRay;

        if (tNear > tFar) std::swap(tNear, tFar);

        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    {
        /* Check against Y plane */
        Float invRay = One / r.direction.y;
        Float tNear = (b.pMin.y - r.origin.y) * invRay;
        Float tFar = (b.pMax.y - r.origin.y) * invRay;

        if (tNear > tFar) std::swap(tNear, tFar);

        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    {
        /* Check against Z plane */
        Float invRay = One / r.direction.z;
        Float tNear = (b.pMin.z - r.origin.z) * invRay;
        Float tFar = (b.pMax.z - r.origin.z) * invRay;

        if (tNear > tFar) std::swap(tNear, tFar);

        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }

    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;

    return true;
}

bool RayAABBIntersectionFast(Ray const& r, BoundingBox const& b, const Direction& invDir, int const dirIsNeg[3])
{
    // Check for ray intersection against $x$ and $y$ slabs
    Float tMin = (b[dirIsNeg[0]].x - r.origin.x) * invDir.x;
    Float tMax = (b[1 - dirIsNeg[0]].x - r.origin.x) * invDir.x;
    Float tyMin = (b[dirIsNeg[1]].y - r.origin.y) * invDir.y;
    Float tyMax = (b[1 - dirIsNeg[1]].y - r.origin.y) * invDir.y;

    // Update _tMax_ and _tyMax_ to ensure robust bounds intersection
    tMax *= 1 + 2 * gamma(3);
    tyMax *= 1 + 2 * gamma(3);
    if (tMin > tyMax || tyMin > tMax) return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    // Check for ray intersection against $z$ slab
    Float tzMin = (b[dirIsNeg[2]].z - r.origin.z) * invDir.z;
    Float tzMax = (b[1 - dirIsNeg[2]].z - r.origin.z) * invDir.z;

    // Update _tzMax_ to ensure robust bounds intersection
    tzMax *= 1 + 2 * gamma(3);
    if (tMin > tzMax || tzMin > tMax) return false;
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;
    return (tMin < r.maxT) && (tMax > 0);
}

Intersection::Intersection()
{ }

Intersection::~Intersection()
{ }

std::optional<Common::HitPoint> Intersection::IntersectRay(Ray& r, entt::registry& objects)
{

    std::optional<HitPoint> finalHitPoint;
    Direction invDir = One / r.origin;
    {
        /* Perform ray-sphere intersections */
        auto view = objects.view<const Base, const Sphere>();
        for (auto const& [entity, base, sphere] : view.each())
        {
            std::optional<HitPoint> hp = RaySphereIntersection(r, base, sphere);

            if (!hp.has_value())
                continue;

            Float intersectionPoint = hp->GetIntersectionPoint();
            r.maxT = intersectionPoint;
            CHECK(base.entityPtr != nullptr) << "Base doesn't include an entity pointer";
            hp->SetEntity(base.entityPtr);

            finalHitPoint = hp.value();
        }
    }
    {
        /* Perform ray-aabb intersection */
        BoundingBox bb(Position(0, 0, 0), Position(2, 2, 2));
        Direction invDir = One / r.direction;
        int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
#if ENABLE_SLOW_VS_FAST
        CHECK(RayAABBIntersectionFast(r, bb, invDir, dirIsNeg) == RayAABBIntersectionSlow(r, bb)) << "Something went wrong checking the slow vs fast AABB instersection";
#endif
        if (RayAABBIntersectionFast(r, bb, invDir, dirIsNeg))
        {
            static std::shared_ptr<Lambertian> lamb = std::make_shared<Lambertian>(
                CreateInfo::Material{
                    .name = "default",
                    .attenuation = Cyan,
                    .fuziness = 0.0f,
                    .refractionIndex = 0.0f,
                    .type = CreateInfo::MaterialType::Lambertian,
                    .mask = CreateInfo::Material::FeaturesMask::Attenuation
                }, 69);
            HitPoint hp = {};
            hp.SetFrontFace(true);
            hp.SetIntersectionPoint(0.0f);
            hp.SetNormal(Up);
            hp.SetMaterial(lamb);
            finalHitPoint = hp;
        }
    }

    return finalHitPoint;
}
