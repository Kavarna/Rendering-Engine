#pragma once

#include <Jnrlib.h>
#include <Material/Material.h>

namespace Common
{
    class HitPoint
    {
    public:
        static constexpr const unsigned int MAX_INTERSECTION_POINTS = 2;

    public:
        HitPoint() = default;
        ~HitPoint() = default;

    public:
        void SetIntersectionPoint(Jnrlib::Float t);
        void SetNormal(Jnrlib::Direction const& normal);
        void SetMaterial(std::shared_ptr<IMaterial> material);
        void SetFrontFace(bool frontFace);

    public:
        Jnrlib::Direction const& GetNormal() const;
        Jnrlib::Float GetIntersectionPoint() const;
        std::shared_ptr<IMaterial> GetMaterial() const;
        bool GetFrontFace() const;

    private:
        Jnrlib::Direction mNormal;
        Jnrlib::Float mIntersectionPoint;

        std::shared_ptr<IMaterial> mMaterial;

        bool mFrontFace = false;

    };

}