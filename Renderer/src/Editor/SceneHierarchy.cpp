#include "SceneHierarchy.h"
#include "SceneViewer.h"
#include "ObjectInspector.h"

#include "Scene/Components/Base.h"
#include "Common/CreateInfo/SceneCreateInfo.h"

#include "imgui.h"

using namespace Editor;

SceneHierarchy::SceneHierarchy(Common::Scene* scene, SceneViewer* sceneViewer, ObjectInspector* objInspector) :
    mScene(scene), mSceneViewer(sceneViewer), mObjectInspector(objInspector)
{
}

void SceneHierarchy::MenuPerNode(Common::Entity *entity)
{
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Rename"))
        {
            mRenameStarted = true;
            mEntityToRename = entity;
        }

        ImGui::EndPopup();
    }

    if (mRenameStarted && entity == mEntityToRename)
    {
        auto &lbase = mEntityToRename->GetComponent<Common::Components::Base>();
        std::string newName(lbase.name);
        newName.resize(Common::Constants::MAX_NAME_SIZE);
        if (ImGui::InputText("New name", (char *)newName.c_str(), Common::Constants::MAX_NAME_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank))
        {
            lbase.name = newName;
            mRenameStarted = false;
            mEntityToRename = nullptr;
        }
    }
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

    MenuPerNode(entity);

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
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::MenuItem("Add new object"))
        {
            mScene->AddNewEntity(true, nullptr);
        }

        ImGui::EndPopup();
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
        mObjectInspector->SetEntity(entity, mScene);
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



