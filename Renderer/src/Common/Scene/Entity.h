#pragma once

#include <entt/entt.hpp>

namespace Common
{
    class Scene;


    class Entity
    {
        friend class Common::Scene;
        static constexpr const uint32_t INITIAL_RESERVED_NUMBER_OF_CHILDREN = 8;
    public:
        Entity(entt::entity entity, entt::registry& entities, Entity* parent = nullptr) :
            mEntity(entity), mEntities(entities), mParentEntity(parent)
        {
            mChildEntities.reserve(INITIAL_RESERVED_NUMBER_OF_CHILDREN);
        }
        ~Entity() = default;

        Entity(Entity const&) = delete;
        Entity(Entity&&) = delete;
        Entity& operator=(Entity const&) = delete;
        Entity& operator=(Entity&&) = delete;

        void UpdateBase();

        template <typename ComponentType>
        ComponentType& AddComponent(ComponentType&& component);

        template <typename T>
        T const& GetComponent() const;

        template <typename T>
        T* TryGetComponent() const;

        template <typename T>
        T& GetComponent();

        template <typename T, typename Callable>
        void PatchComponent(Callable&&);

        void SetParent(Entity* parentEntity);
        void AddChild(Entity* childEntity);
        uint32_t GetEntityId() const;
        bool HasChildren() const;
        std::vector<Entity*>const& GetChildren() const;

        operator bool() const
        {
            return mEntity != entt::null;
        }
        operator entt::entity() const
        {
            return mEntity;
        }

    private:
        void AddChildUnsafe(Entity* childEntity);

    private:
        entt::entity mEntity;
        entt::registry& mEntities;

        std::vector<Entity*> mChildEntities;
        Entity* mParentEntity = nullptr;

    };

    template <typename ComponentType>
    inline ComponentType& Entity::AddComponent(ComponentType&& component)
    {
        using DecayedComponentType = std::decay_t<ComponentType>;
        return mEntities.emplace<DecayedComponentType>(mEntity, std::forward<ComponentType>(component));
    }

    template<typename T>
    inline T const& Entity::GetComponent() const
    {
        return mEntities.get<T>(mEntity);
    }

    template<typename T>
    inline T* Entity::TryGetComponent() const
    {
        return mEntities.try_get<T>(mEntity);
    }

    template<typename T>
    inline T& Entity::GetComponent()
    {
        return mEntities.get<T>(mEntity);
    }
    template<typename T, typename Callable>
    inline void Entity::PatchComponent(Callable&& callable)
    {
        mEntities.patch<T>(mEntity, callable);
    }
}
