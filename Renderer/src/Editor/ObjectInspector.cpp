#include "ObjectInspector.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "Constants.h"

#include "Common/Scene/Components/BaseComponent.h"
#include "Common/Scene/Components/UpdateComponent.h"

using namespace Editor;
using namespace Common::Components;

void ObjectInspector::OnRender()
{
    ImGui::Begin("Object inspector");

    if (mActiveEntity == nullptr)
    {
        ImGui::Text("No entity selected");
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Base"))
    {
        Base& b = mActiveEntity->GetComponent<Base>();
        Update& u = mActiveEntity->GetComponent<Update>();

        ImGui::InputText("Name", &b.name, ImGuiInputTextFlags_CharsNoBlank,
            [](ImGuiInputTextCallbackData* data)
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                std::string* name = (std::string*)data->UserData;
                name->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
            }
        return 0;
        }, &b.name);
        
        if (ImGui::DragFloat3("Position", (float*)&b.position))
        {
            u.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT;
        }
        if (ImGui::DragFloat3("Scaling", (float*)&b.scaling))
        {
            u.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT;
        }
    }

    ImGui::End();
}

void ObjectInspector::SetEntity(Common::Entity* entity)
{
    mActiveEntity = entity;
}
