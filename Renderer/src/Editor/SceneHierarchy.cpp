#include "SceneHierarchy.h"
#include "SceneViewer.h"

#include "Scene/Components/BaseComponent.h"

#include "imgui.h"

using namespace Editor;

SceneHierarchy::SceneHierarchy(Common::Scene* scene, SceneViewer* sceneViewer) : 
    mScene(scene), mSceneViewer(sceneViewer)
{
}

uint32_t SceneHierarchy::RenderNode(Entity* entity)
{
    uint32_t clickedEntity = -1;

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
        clickedEntity = entityId;
    }


    if (isNodeOpen)
    {
        if (entity->HasChildren())
        {
            auto& children = entity->GetChildren();
            for (uint32_t i = 0; i < children.size(); ++i)
            {
                uint32_t entity = RenderNode(children[i]);
                if (entity != -1)
                    clickedEntity = entity;

            }
        }
        ImGui::TreePop();
    }

    return clickedEntity;
}

void SceneHierarchy::OnRender()
{
    auto& entities = mScene->GetRootEntities();
    ImGui::Begin("Scene hierarchy");
    
    uint32_t clickedEntity = -1;
    for (uint32_t i = 0; i < entities.size(); ++i)
    {
        uint32_t entity = RenderNode(entities[i]);
        if (entity != -1)
            clickedEntity = entity;
    }
    if (clickedEntity != -1)
    {
        if (ImGui::GetIO().KeyCtrl)
        {
            // CTRL+click to toggle
            if (auto it = mSelectedNodes.find(clickedEntity); it != mSelectedNodes.end())
            {
                mSelectedNodes.erase(it);
            }
            else
            {
                mSelectedNodes.insert(clickedEntity);
            }
        }
        else
        {
            mSelectedNodes.clear();
            mSelectedNodes.insert(clickedEntity);
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



