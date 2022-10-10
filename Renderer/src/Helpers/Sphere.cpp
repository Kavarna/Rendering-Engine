#include "Sphere.h"

using nlohmann::json;

Sphere::Sphere(std::string const& name, Jnrlib::Position const& position, Jnrlib::Float radius, std::string const& materialName) :
    mName(name), mPosition(position), mRadius(radius), mMaterial(MaterialManager::Get()->GetMaterial(materialName))
{
    CHECK((mMaterial.mask & Material::FeaturesMask::Color) != 0) << "Sphere material must have a color";
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

    Jnrlib::Float firstPoint = tca - thc;
    Jnrlib::Float secondPoint = tca + thc;

    HitPoint hp = {};
    hp.SetColor(mMaterial.color);
    hp.SetIntersectionPoint(std::min(firstPoint, secondPoint), 0);
    hp.SetIntersectionPoint(std::max(firstPoint, secondPoint), 1);

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
    j["material"] = s.mMaterial.name;
    j["radius"] = s.mRadius;
    j["position"] = Jnrlib::to_string(s.mPosition);
}

void from_json(const nlohmann::json & j, Sphere & s)
{
    std::string positionString;
    j.at("name").get_to(s.mName);
    j.at("material").get_to(s.mMaterial.name);
    j.at("radius").get_to(s.mRadius);
    j.at("position").get_to(positionString);
    s.mPosition = Jnrlib::to_type<Jnrlib::Position>(positionString);
    s.mMaterial = MaterialManager::Get()->GetMaterial(s.mMaterial.name);
}
