#pragma once

#include <Jnrlib.h>
#include <Material/Material.h>

namespace Common
{
    class Entity;

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
        void SetEntity(Entity* entity);

    public:
        Jnrlib::Direction const& GetNormal() const;
        Jnrlib::Float GetIntersectionPoint() const;
        std::shared_ptr<IMaterial> GetMaterial() const;
        bool GetFrontFace() const;
        Entity* GetEntity() const;

    private:
        Jnrlib::Direction mNormal;
        Jnrlib::Float mIntersectionPoint;

        Entity* mEntity = nullptr;
        std::shared_ptr<IMaterial> mMaterial;

        bool mFrontFace = false;

    };

}