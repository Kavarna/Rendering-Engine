#include "Entity.h"
#include "Components/Update.h"
#include "Constants.h"

using namespace Common;

void Entity::UpdateBase()
{
    auto& u = GetComponent<Components::Update>();
    u.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT;
}

void Entity::SetParent(Entity* parentEntity)
{
    if (parentEntity == mParentEntity)
        return;

    for (uint32_t i = 0; i < parentEntity->mChildEntities.size(); ++i)
    {
        if (parentEntity->mChildEntities[i] == this)
        {
            mParentEntity = parentEntity;
            return;
        }
    }
    mParentEntity = parentEntity;
    parentEntity->AddChildUnsafe(this);
}

void Entity::AddChild(Entity* childEntity)
{
    if (childEntity->mParentEntity == this)
        return;

    for (uint32_t i = 0; i < mChildEntities.size(); ++i)
    {
        if (mChildEntities[i] == childEntity)
        {
            return;
        }
    }

    childEntity->mParentEntity = this;
    AddChildUnsafe(childEntity);
}

uint32_t Entity::GetEntityId() const
{
    return (size_t)mEntity;
}

bool Entity::HasChildren() const
{
    return mChildEntities.size();
}

std::vector<Entity*> const& Entity::GetChildren() const
{
    return mChildEntities;
}

void Entity::AddChildUnsafe(Entity* childEntity)
{
    mChildEntities.push_back(childEntity);
}
