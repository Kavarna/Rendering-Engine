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

    virtual bool IntersectRay(Ray const&, HitPoint& hp) override;

    friend std::ostream& operator << (std::ostream& stream, Sphere const& s);
    friend std::istream& operator >> (std::istream& stream, Sphere& s);
    friend void to_json(nlohmann::json& j, const Sphere& s);
    friend void from_json(const nlohmann::json& j, Sphere& s);

private:
    Jnrlib::Position mPosition;
    Jnrlib::Float mRadius;

    std::string mName;
    std::string mMaterialName;

};

