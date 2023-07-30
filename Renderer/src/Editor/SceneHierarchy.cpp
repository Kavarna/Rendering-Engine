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

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (auto it = mSelectedEntities.find(entity); it != mSelectedEntities.end())
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
    if (!ImGui::Begin("Scene hierarchy"))
    {
        // return;
    }
    
    Common::Entity* selectedEntity = nullptr; 
    for (uint32_t i = 0; i < entities.size(); ++i)
    {
        Common::Entity* entity = RenderNode(entities[i]);
        if (entity != nullptr)
            selectedEntity = entity;
    }
    if (selectedEntity != nullptr)
    {
        SelectEntity(selectedEntity);
    }
    else
    {
        ClearSelection();
    }

    ImGui::End();
}

void SceneHierarchy::SelectEntity(Common::Entity* entity)
{
    if (ImGui::GetIO().KeyCtrl)
    {
        // CTRL+click to toggle
        if (auto it = mSelectedEntities.find(entity); it != mSelectedEntities.end())
        {
            mSelectedEntities.erase(it);
        }
        else
        {
            mSelectedEntities.insert(entity);
        }
    }
    else
    {
        mSelectedEntities.clear();
        mSelectedEntities.insert(entity);
        mObjectInspector->SetEntity(entity);
    }
    if (mSceneViewer != nullptr)
        mSceneViewer->SelectEntities(mSelectedEntities);
}

void SceneHierarchy::ClearSelection()
{
    auto& io = ImGui::GetIO();
    bool mouseClicked = io.MouseClicked[0];
    if (mouseClicked && ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
    {
        mSelectedEntities.clear();
        if (mSceneViewer != nullptr)
            mSceneViewer->ClearSelection();
        if (mObjectInspector != nullptr)
            mObjectInspector->ClearSelection();
    }
}



