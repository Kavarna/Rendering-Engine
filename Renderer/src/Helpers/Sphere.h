#pragma once


#include <Jnrlib.h>
#include "Primitive.h"
#include "MaterialManager.h"

class Sphere : public Primitive
{
public:
    Sphere() = default;
    Sphere(std::string const& name, Jnrlib::Position const& position, Jnrlib::Float radius, std::string const& materialName);
    ~Sphere();

    virtual std::optional<HitPoint> IntersectRay(Ray const&) override;

    friend std::ostream& operator << (std::ostream& stream, Sphere const& s);
    friend std::istream& operator >> (std::istream& stream, Sphere& s);
    friend void to_json(nlohmann::json& j, const Sphere& s);
    friend void from_json(const nlohmann::json& j, Sphere& s);

private:
    Jnrlib::Position mPosition;
    Jnrlib::Float mRadius;

    std::string mName;
    std::shared_ptr<IMaterial> mMaterial;
};

