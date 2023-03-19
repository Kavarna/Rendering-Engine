#pragma once

#include "ImguiWindow.h"
#include "Common/Scene/Entity.h"


namespace Editor
{

    class ObjectInspector : public ImguiWindow
    {
    public:
        ObjectInspector() = default;
        ~ObjectInspector() = default;

        virtual void OnRender() override;

    public:
        void SetEntity(Common::Entity* entity);

    private:
        Common::Entity* mActiveEntity;

    };
}
