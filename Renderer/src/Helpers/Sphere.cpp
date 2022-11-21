#include "Sphere.h"

using nlohmann::json;

Sphere::Sphere(std::string const& name, Jnrlib::Position const& position, Jnrlib::Float radius, std::string const& materialName) :
    mName(name), mPosition(position), mRadius(radius)
{
    mMaterial = MaterialManager::Get()->GetMaterial(materialName);
    CHECK(mMaterial != nullptr) << "A sphere must have a material";
}

Sphere::~Sphere()
{ }

std::optional<HitPoint> Sphere::IntersectRay(Ray const& r)
{
#undef min
#undef max
    Jnrlib::Direction toCenter = mPosition - r.GetStartPosition();
    Jnrlib::Float tca = glm::dot(toCenter, r.GetDirection());
    if (tca < Jnrlib::Zero)
        return std::nullopt;

    Jnrlib::Float d2 = glm::dot(toCenter, toCenter) - tca * tca;
    if (d2 > mRadius * mRadius)
        return std::nullopt;

    Jnrlib::Float thc = sqrt(mRadius * mRadius - d2);

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
    Jnrlib::Direction normal = hitPosition - mPosition;

    if (fabs(intersectionPoint) < Jnrlib::EPSILON)
        return std::nullopt;

    /* Fill the hitpoint */
    HitPoint hp = {};
    hp.SetIntersectionPoint(intersectionPoint);
    hp.SetMaterial(mMaterial);
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

std::ostream& operator<<(std::ostream& stream, Sphere const& s)
{
    json j;

    to_json(j, s);

    stream << j;
    return stream;
}

std::istream& operator>>(std::istream& stream, Sphere& s)
{
    json j;

    stream >> j;
    from_json(j, s);

    return stream;
}

void to_json(nlohmann::json& j, const Sphere& s)
{
    j["name"] = s.mName;
    j["radius"] = s.mRadius;
    j["position"] = Jnrlib::to_string(s.mPosition);
    j["material"] = s.mMaterial->GetName();
}

void from_json(const nlohmann::json & j, Sphere & s)
{
    std::string positionString, materialName;
    j.at("name").get_to(s.mName);
    j.at("radius").get_to(s.mRadius);
    j.at("position").get_to(positionString);
    j.at("material").get_to(materialName);

    s.mMaterial = MaterialManager::Get()->GetMaterial(materialName);
    CHECK(s.mMaterial != nullptr) << "A sphere must have a material";
    s.mPosition = Jnrlib::to_type<Jnrlib::Position>(positionString);
}
