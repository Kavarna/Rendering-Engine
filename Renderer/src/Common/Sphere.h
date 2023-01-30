#pragma once


#include <Jnrlib.h>
#include "Primitive.h"
#include "HitPoint.h"

namespace Common
{

    class Sphere : public Primitive
    {
    public:
        Sphere() = default;
        Sphere(std::string const& name, Jnrlib::Position const& position, Jnrlib::Float radius, std::string const& materialName);
        ~Sphere();

        virtual std::optional<Common::HitPoint> IntersectRay(Ray const&) override;

    private:
        Jnrlib::Position mPosition;
        Jnrlib::Float mRadius;

        std::string mName;
        std::shared_ptr<IMaterial> mMaterial;
    };

}