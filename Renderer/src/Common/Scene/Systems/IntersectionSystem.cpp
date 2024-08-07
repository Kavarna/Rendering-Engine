#include "IntersectionSystem.h"

#include "Ray.h"
#include "HitPoint.h"
#include "Scene/Components/Base.h"
#include "Scene/Components/Sphere.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Components/AccelerationStructure.h"
#include "Scene/Scene.h"

#include "Material/Lambertian.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace Common;
using namespace Components;
using namespace Systems;
using namespace Jnrlib;

/* Helpers */
inline Float gamma(int n)
{
    return (n * EPSILON) / (1 - n * EPSILON);
}

static std::optional<HitPoint> RaySphereIntersection(Ray& r, Base const& base, Sphere const& s)
{
    /* sphere = (x-pos.x)^2 + (y-pos.y)^2 + (z-pos.z)^2 - radius^2 = 0
     * ray = o + t * d
     */
    Float radius = s.radius;
    Float ox = r.origin.x;
    Float oy = r.origin.y;
    Float oz = r.origin.z;

    Float dx = r.direction.x;
    Float dy = r.direction.y;
    Float dz = r.direction.z;

    /* Extracing the components for the equation ax^2 + bx + c = 0 */
    Float lx = ox;
    Float ly = oy;
    Float lz = oz;

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

    Jnrlib::Vec3 scale;
    Jnrlib::Quaternion rotation;
    Jnrlib::Position translation;
    Jnrlib::Vec3 skew;
    Jnrlib::Vec4 perspective;
    glm::decompose(base.world, scale, rotation, translation, skew, perspective);

    Position hitPosition = r.At(t1);
    Direction normal = glm::normalize(hitPosition - translation);

    if (fabs(intersectionPoint) < EPSILON || intersectionPoint > r.maxT)
        return std::nullopt;

    r.maxT = intersectionPoint;

    HitPoint hp{};
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

static bool RayAABBIntersectionSlow(Ray const& r, BoundingBox const& b, Float* hitt0 = nullptr, Float* hitt1 = nullptr)
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

static bool RayAABBIntersectionFast(Ray const& r, BoundingBox const& b, const Direction& invDir, int const dirIsNeg[3])
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

static bool RayTriangleIntersection(Ray const& r, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float* t, Float barycentrics[3])
{
    // Translate vertices based on ray origin
    auto p0t = p0 - r.origin;
    auto p1t = p1 - r.origin;
    auto p2t = p2 - r.origin;

    // Permute components of triangle vertices and ray direction
    Axis kz = GetMaximumAxis(abs(r.direction));
    Axis kx = kz + (Axis)1;
    Axis ky = kx + (Axis)1;
    glm::vec3 d = Permute(r.direction, kx, ky, kz);
    p0t = Permute(p0t, kx, ky, kz);
    p1t = Permute(p1t, kx, ky, kz);
    p2t = Permute(p2t, kx, ky, kz);

    // Apply shear transformation to translated vertex positions
    Float Sx = -d.x / d.z;
    Float Sy = -d.y / d.z;
    Float Sz = 1.f / d.z;
    p0t.x += Sx * p0t.z;
    p0t.y += Sy * p0t.z;
    p1t.x += Sx * p1t.z;
    p1t.y += Sy * p1t.z;
    p2t.x += Sx * p2t.z;
    p2t.y += Sy * p2t.z;

    // Compute edge function coefficients _e0_, _e1_, and _e2_
    Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
    Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
    Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

    // Fall back to double precision test at triangle edges
    if (sizeof(Float) == sizeof(float) &&
        (e0 == 0.0f || e1 == 0.0f || e2 == 0.0f))
    {
        double p2txp1ty = (double)p2t.x * (double)p1t.y;
        double p2typ1tx = (double)p2t.y * (double)p1t.x;
        e0 = (float)(p2typ1tx - p2txp1ty);
        double p0txp2ty = (double)p0t.x * (double)p2t.y;
        double p0typ2tx = (double)p0t.y * (double)p2t.x;
        e1 = (float)(p0typ2tx - p0txp2ty);
        double p1txp0ty = (double)p1t.x * (double)p0t.y;
        double p1typ0tx = (double)p1t.y * (double)p0t.x;
        e2 = (float)(p1typ0tx - p1txp0ty);
    }

    // Perform triangle edge and determinant tests
    if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
        return false;
    Float det = e0 + e1 + e2;
    if (det == 0) return false;

    // Compute scaled hit distance to triangle and test against ray $t$ range
    p0t.z *= Sz;
    p1t.z *= Sz;
    p2t.z *= Sz;
    Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
    if (det < 0 && (tScaled >= 0 || tScaled < r.maxT * det))
        return false;
    else if (det > 0 && (tScaled <= 0 || tScaled > r.maxT * det))
        return false;

    // Compute barycentric coordinates and $t$ value for triangle intersection
    Float invDet = 1 / det;
    barycentrics[0] = e0 * invDet;
    barycentrics[1] = e1 * invDet;
    barycentrics[2] = e2 * invDet;
    *t = tScaled * invDet;

    return true;
}

static std::optional<HitPoint> RayMeshIntersectionSlow(Ray& r, Base const& base, Mesh const& mesh, Scene const* scene)
{
    auto const& indices = scene->GetIndices();
    auto const& vertices = scene->GetVertices();
    float t = -1.0f;
    glm::vec3 normal(Zero);

    uint32_t triangleCount = mesh.indices.indexCount / 3;
    for (uint32_t i = 0; i < triangleCount; ++i)
    {
        uint32_t index0 = indices[mesh.indices.firstIndex + i * 3 + 0];
        uint32_t index1 = indices[mesh.indices.firstIndex + i * 3 + 1];
        uint32_t index2 = indices[mesh.indices.firstIndex + i * 3 + 2];

        glm::vec3 p0 = vertices[mesh.indices.firstVertex + index0].position;
        glm::vec3 p1 = vertices[mesh.indices.firstVertex + index1].position;
        glm::vec3 p2 = vertices[mesh.indices.firstVertex + index2].position;

        float barycentrics[3];
        if (RayTriangleIntersection(r, p0, p1, p2, &t, barycentrics))
        {
            r.maxT = t;
            normal = vertices[mesh.indices.firstVertex + index0].normal * barycentrics[0] +
                vertices[mesh.indices.firstVertex + index1].normal * barycentrics[1] +
                vertices[mesh.indices.firstVertex + index2].normal * barycentrics[2];
        }
    }
    
    if (t == -1)
        return std::nullopt;

    HitPoint hp{};
    hp.SetEntity(base.entityPtr);
    hp.SetIntersectionPoint(t);
    hp.SetMaterial(mesh.material);
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

static std::optional<HitPoint> RayMeshIntersectionFast(Ray &r, Base const& base, Mesh const& mesh, AccelerationStructure const& accelStructure, Scene const* scene)
{
    if (accelStructure.nodes.empty())
        return std::nullopt;

    auto const& indices = scene->GetIndices();
    auto const& vertices = scene->GetVertices();

    Direction invDir = One / r.direction;
    int isDirNeg[3] = {r.direction.x < 0, r.direction.y < 0, r.direction.z < 0};

    int toVisitOffset = 0;
    int currentNodeIndex = 0;
    int nodesToVisit[64] = {};
    const Common::Components::LinearBVHNode* hitNode = nullptr;
    uint32_t hitPrimitve = -1;
    Float hitBarycentrics[3]{};
    while (true)
    {
        const Common::Components::LinearBVHNode* node = &accelStructure.nodes[currentNodeIndex];
        auto bounds = node->bounds;
        if (RayAABBIntersectionFast(r, bounds, invDir, isDirNeg))
        {
            if (node->primitiveCount)
            {
                /* Should check against each primitive */
                for (uint32_t i = 0; i < node->primitiveCount; ++i)
                {
                    auto index0 = mesh.indices.firstIndex + (node->primitiveOffset + i) * 3 + 0;
                    auto index1 = mesh.indices.firstIndex + (node->primitiveOffset + i) * 3 + 1;
                    auto index2 = mesh.indices.firstIndex + (node->primitiveOffset + i) * 3 + 2;

                    glm::vec3 p0 = vertices[mesh.indices.firstVertex + indices[index0]].position;
                    glm::vec3 p1 = vertices[mesh.indices.firstVertex + indices[index1]].position;
                    glm::vec3 p2 = vertices[mesh.indices.firstVertex + indices[index2]].position;

                    Float barycentrics[3]{};
                    float t;
                    if (RayTriangleIntersection(r, p0, p1, p2, &t, barycentrics))
                    {
                        r.maxT = t;
                        hitNode = node;
                        hitPrimitve = i;
                        memcpy_s(hitBarycentrics, sizeof(hitBarycentrics), barycentrics, sizeof(barycentrics));
                    }
                }
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                if (isDirNeg[static_cast<uint32_t>(node->axis)])
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else
        {
            if (toVisitOffset == 0)
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }

    if (hitNode)
    {
        HitPoint hp{};
        hp.SetEntity(base.entityPtr);
        hp.SetIntersectionPoint(r.maxT);
        hp.SetMaterial(mesh.material);

        auto index0 = mesh.indices.firstIndex + (hitNode->primitiveOffset + hitPrimitve) * 3 + 0;
        auto index1 = mesh.indices.firstIndex + (hitNode->primitiveOffset + hitPrimitve) * 3 + 1;
        auto index2 = mesh.indices.firstIndex + (hitNode->primitiveOffset + hitPrimitve) * 3 + 2;

        auto& v0 = vertices[mesh.indices.firstVertex + indices[index0]];
        auto& v1 = vertices[mesh.indices.firstVertex + indices[index1]];
        auto& v2 = vertices[mesh.indices.firstVertex + indices[index2]];


        auto normal = v0.normal * hitBarycentrics[0] + v1.normal * hitBarycentrics[1] + v2.normal * hitBarycentrics[2];
        {
            /* We're hitting the sphere in the front */
            hp.SetNormal(normal);
            hp.SetFrontFace(true);
        }
        return hp;
    }
    return std::nullopt;
}

Intersection::Intersection()
{ }

Intersection::~Intersection()
{ }

std::optional<Common::HitPoint> Intersection::IntersectRay(Ray& r, entt::registry& objects, Common::Scene const* scene)
{
    std::optional<HitPoint> finalHitPoint;
    Direction invDir = One / r.origin;
    {
        /* Perform ray-sphere intersections */
        auto view = objects.view<const Base, const Sphere>();
        for (auto const& [entity, base, sphere] : view.each())
        {
            /* TODO: This code is duplicated; Ideally we should not do this during raytracing => bake all world matrices */
            Jnrlib::Matrix4x4 world = base.world;
            auto parent = base.entityPtr->GetParent();
            while (parent != nullptr)
            {
                auto &parentBase = parent->GetComponent<Base>();
                world = world * parentBase.world;
                parent = parent->GetParent();
            }
            Jnrlib::Matrix4x4 inverseWorld = glm::inverse(world);

            auto localSpaceRay = r.TransformedRay(inverseWorld);
            std::optional<HitPoint> hp = RaySphereIntersection(localSpaceRay, base, sphere);

            if (!hp.has_value())
                continue;

            Float intersectionPoint = hp->GetIntersectionPoint();
            CHECK(base.entityPtr != nullptr) << "Base doesn't include an entity pointer";
            hp->SetEntity(base.entityPtr);

            finalHitPoint = hp.value();
            r.maxT = localSpaceRay.maxT;
        }
    }

    {
        /* Perform ray-mesh intersections */
        auto view = objects.view<const Base, const Mesh, const AccelerationStructure>();
        for (auto const& [entity, base, mesh, accel] : view.each())
        {
            if (auto ptr = base.entityPtr->TryGetComponent<Sphere>(); ptr != nullptr) [[unlikely]]
                continue;

            /* TODO: This code is duplicated; Ideally we should not do this during raytracing => bake all world matrices */
            Jnrlib::Matrix4x4 world = base.world;
            auto parent = base.entityPtr->GetParent();
            while (parent != nullptr)
            {
                auto &parentBase = parent->GetComponent<Base>();
                world = world * parentBase.world;
                parent = parent->GetParent();
            }
            Jnrlib::Matrix4x4 inverseWorld = glm::inverse(world);

            auto localSpaceRay = r.TransformedRay(inverseWorld);
            std::optional<HitPoint> hp = RayMeshIntersectionFast(localSpaceRay, base, mesh, accel, scene);

            if (!hp.has_value())
                continue;

            Float intersectionPoint = hp->GetIntersectionPoint();
            CHECK(base.entityPtr != nullptr) << "Base doesn't include an entity pointer";
            hp->SetEntity(base.entityPtr);

            finalHitPoint = hp.value();
            r.maxT = localSpaceRay.maxT;
        }
    }
    return finalHitPoint;
}
