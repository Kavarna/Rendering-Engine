#include "ObjectInspector.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "Constants.h"

#include "Common/Scene/Components/BaseComponent.h"
#include "Common/Scene/Components/UpdateComponent.h"
#include "Common/Scene/Components/SphereComponent.h"

using namespace Editor;
using namespace Common;
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

    bool isUpdatable = mActiveEntity->TryGetComponent<Components::Update>() ? true : false;

    if (ImGui::CollapsingHeader("Base"))
    {
        RenderBase(mActiveEntity->GetComponent<Base>(), isUpdatable);
    }

    if (auto* s = mActiveEntity->TryGetComponent<Sphere>(); s != nullptr && ImGui::CollapsingHeader("Sphere"))
    {
        RenderSphere(*s, isUpdatable);
    }

    ImGui::End();
}

void ObjectInspector::SetEntity(Entity* entity)
{
    mActiveEntity = entity;
}

void ObjectInspector::RenderBase(Base& b, bool isUpdatable)
{

    std::string name = b.name;
    glm::vec3 position = b.position, scaling = b.scaling;

    auto inputTextFlags = ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue;
    if (isUpdatable)
    {
        inputTextFlags |= ImGuiInputTextFlags_ReadOnly;
    }

    bool nameChanged = ImGui::InputText(
        "Name", &name, inputTextFlags,
        [](ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            std::string* name = (std::string*)data->UserData;
            name->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
        }
    return 0;
    }, &b.name);
    if (nameChanged)
    {
        if (name.size() < Constants::MAX_NAME_SIZE)
        {
            b.name = name;
        }
    }

    if (ImGui::DragFloat3("Position", (float*)&position) && isUpdatable)
    {
            mActiveEntity->PatchComponent<Base>([&](Base& b)
            {
                b.position = position;
            });
    }
    if (ImGui::DragFloat3("Scaling", (float*)&scaling) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Base>([&](Base& b)
        {
            b.scaling = scaling;
        });
    }
}

void ObjectInspector::RenderSphere(Sphere& s, bool isUpdatable)
{
    /* TODO: Create a material inspector, and set the material there */
    ImGui::Text("Material name: %s", s.material->GetName().c_str());
}
