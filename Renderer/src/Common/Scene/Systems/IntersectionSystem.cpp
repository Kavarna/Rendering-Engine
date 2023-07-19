#include "IntersectionSystem.h"

#include "Ray.h"
#include "HitPoint.h"
#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/SphereComponent.h"
#include "Scene/Components/MeshComponent.h"

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

std::optional<HitPoint> RaySphereIntersection(Ray const& r, Base const& base, Sphere const& s, Mesh const& m)
{
    /* sphere = (x-pos.x)^2 + (y-pos.y)^2 + (z-pos.z)^2 - radius^2 = 0
     * ray = o + t * d
     */
    Float radius = s.radius;
    Float px = base.position.x;
    Float py = base.position.y;
    Float pz = base.position.z;

    Float ox = r.origin.x;
    Float oy = r.origin.y;
    Float oz = r.origin.z;

    Float dx = r.direction.x;
    Float dy = r.direction.y;
    Float dz = r.direction.z;

    /* Extracing the components for the equation ax^2 + bx + c = 0 */
    Float lx = ox - px;
    Float ly = oy - py;
    Float lz = oz - pz;

    Float a = dx * dx + dy * dy + dz * dz;
    Float b = 2 * (dx * lx + dy * ly + dz * lz);
    Float c = lx * lx + ly * ly + lz * lz - s.radius * s.radius;

    Float t1, t2;
    if (!Quadratic(a, b, c, &t1, &t2))
        return std::nullopt;

    Float intersectionPoint;
    if (t1 >= 0.0)
    {
        intersectionPoint = t1;
    }
    else if (t2 >= 0.0) // && (t1 < 0)
    {
        intersectionPoint = t2;
    }
    else
    {
        return std::nullopt;
    }

    Position hitPosition = r.At(t1);
    Direction normal = hitPosition - base.position;

    if (fabs(intersectionPoint) < EPSILON || intersectionPoint > r.maxT)
        return std::nullopt;

    HitPoint hp{};
    hp.SetIntersectionPoint(intersectionPoint);
    hp.SetMaterial(m.material);
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
    /* Check intersection with the 6 planes that define the AABB
     * Important note: the intersection has to be always between t0 and t1 (eg. planes Y t1 and t2 should be between t1 and t2 for the plane X)
     */
    Float t0 = Zero, t1 = Infinity;
    {
        /* Check against X planes */
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
        /* Check against Y planes */
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
        /* Check against Z planes */
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
    // Check for ray intersection against x and y slabs
    Float tMin = (b[dirIsNeg[0]].x - r.origin.x) * invDir.x;
    Float tMax = (b[1 - dirIsNeg[0]].x - r.origin.x) * invDir.x;
    Float tyMin = (b[dirIsNeg[1]].y - r.origin.y) * invDir.y;
    Float tyMax = (b[1 - dirIsNeg[1]].y - r.origin.y) * invDir.y;

    // Update tMax and tyMax to ensure robust bounds intersection
    tMax *= 1 + 2 * gamma(3);
    tyMax *= 1 + 2 * gamma(3);
    if (tMin > tyMax || tyMin > tMax) return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    // Check for ray intersection against z slab
    Float tzMin = (b[dirIsNeg[2]].z - r.origin.z) * invDir.z;
    Float tzMax = (b[1 - dirIsNeg[2]].z - r.origin.z) * invDir.z;

    // Update tzMax to ensure robust bounds intersection
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
        auto view = objects.view<const Base, const Sphere, const Mesh>();
        for (auto const& [entity, base, sphere, mesh] : view.each())
        {
            std::optional<HitPoint> hp = RaySphereIntersection(r, base, sphere, mesh);

            if (!hp.has_value())
                continue;

            Float intersectionPoint = hp->GetIntersectionPoint();
            r.maxT = intersectionPoint;
            CHECK(base.entityPtr != nullptr) << "Base doesn't include an entity pointer";
            hp->SetEntity(base.entityPtr);

            finalHitPoint = hp.value();
        }
    }

    return finalHitPoint;
}
