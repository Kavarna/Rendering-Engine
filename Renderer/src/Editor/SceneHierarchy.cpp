#include "SceneHierarchy.h"
#include "SceneViewer.h"
#include "ObjectInspector.h"

#include "Scene/Components/BaseComponent.h"

#include "imgui.h"

using namespace Editor;

SceneHierarchy::SceneHierarchy(Common::Scene* scene, SceneViewer* sceneViewer, ObjectInspector* objInspector) :
    mScene(scene), mSceneViewer(sceneViewer), mObjectInspector(objInspector)
{
}

Common::Entity* SceneHierarchy::RenderNode(Common::Entity* entity)
{
    Common::Entity* selectedEntity = nullptr;

    auto const& base = entity->GetComponent<Common::Components::Base>();
    auto entityId = entity->GetEntityId();

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (auto it = mSelectedNodes.find(entityId); it != mSelectedNodes.end())
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!entity->HasChildren())
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool isNodeOpen = ImGui::TreeNodeEx(base.name.c_str(), nodeFlags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        selectedEntity = entity;
    }


    if (isNodeOpen)
    {
        if (entity->HasChildren())
        {
            auto& children = entity->GetChildren();
            for (uint32_t i = 0; i < children.size(); ++i)
            {
                auto entity = RenderNode(children[i]);
                if (entity != nullptr)
                    selectedEntity = entity;

            }
        }
        ImGui::TreePop();
    }

    return selectedEntity;
}

void SceneHierarchy::OnRender()
{
    auto& entities = mScene->GetRootEntities();
    ImGui::Begin("Scene hierarchy");
    
    Common::Entity* selectedEntity = nullptr;
    for (uint32_t i = 0; i < entities.size(); ++i)
    {
        Common::Entity* entity = RenderNode(entities[i]);
        if (entity != nullptr)
            selectedEntity = entity;
    }
    if (selectedEntity != nullptr)
    {
        if (ImGui::GetIO().KeyCtrl)
        {
            // CTRL+click to toggle
            if (auto it = mSelectedNodes.find(selectedEntity->GetEntityId()); it != mSelectedNodes.end())
            {
                mSelectedNodes.erase(it);
            }
            else
            {
                mSelectedNodes.insert(selectedEntity->GetEntityId());
            }
        }
        else
        {
            mSelectedNodes.clear();
            mSelectedNodes.insert(selectedEntity->GetEntityId());
            mObjectInspector->SetEntity(selectedEntity);
        }
        if (mSceneViewer != nullptr)
            mSceneViewer->SelectIndices(mSelectedNodes);
    }
    else
    {
        auto& io = ImGui::GetIO();
        bool mouseClicked = io.MouseClicked[0];
        if (mouseClicked && ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
        {
            mSelectedNodes.clear();
            if (mSceneViewer != nullptr)
                mSceneViewer->ClearSelection();
        }
    }

    ImGui::End();
}



