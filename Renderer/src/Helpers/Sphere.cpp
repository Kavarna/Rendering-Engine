#include "Sphere.h"

using nlohmann::json;

Sphere::Sphere(std::string const& name, Jnrlib::Position const& position, Jnrlib::Float radius, std::string const& materialName) :
    mName(name), mPosition(position), mRadius(radius), mMaterialName(materialName)
{ }

Sphere::~Sphere()
{ }

bool Sphere::IntersectRay(Ray const&, HitPoint& hp)
{
    return false;
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
    j["material"] = s.mMaterialName;
    j["radius"] = s.mRadius;
    j["position"] = Jnrlib::to_string(s.mPosition);
}

void from_json(const nlohmann::json & j, Sphere & s)
{
    std::string positionString;
    j.at("name").get_to(s.mName);
    j.at("material").get_to(s.mMaterialName);
    j.at("radius").get_to(s.mRadius);
    j.at("position").get_to(positionString);
    s.mPosition = Jnrlib::to_type<Jnrlib::Position>(positionString);
}
