#include "ObjectInspector.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "Constants.h"

#include "Common/Scene/Components/BaseComponent.h"
#include "Common/Scene/Components/UpdateComponent.h"
#include "Common/Scene/Components/SphereComponent.h"
#include "Common/Scene/Components/CameraComponent.h"
#include "Common/Scene/Components/MeshComponent.h"

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

    if (auto* c = mActiveEntity->TryGetComponent<Camera>(); c != nullptr && ImGui::CollapsingHeader("Camera"))
    {
        RenderCamera(*c, isUpdatable);
    }

    if (auto* m = mActiveEntity->TryGetComponent<Mesh>(); m != nullptr && ImGui::CollapsingHeader("Mesh"))
    {
        RenderMesh(*m, isUpdatable);
    }

    ImGui::End();
}

void ObjectInspector::SetEntity(Entity* entity)
{
    mActiveEntity = entity;
}

void ObjectInspector::ClearSelection()
{
    mActiveEntity = nullptr;
}

void ObjectInspector::RenderBase(Base& b, bool isUpdatable)
{

    std::string name = b.name;
    glm::vec3 position = b.position;

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

    if (ImGui::DragFloat3("Position", (float*)&position, 0.01f) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Base>([&](Base& b)
        {
            b.position = position;
        });
    }
}

void ObjectInspector::RenderSphere(Sphere& s, bool isUpdatable)
{
    float radius = s.radius;
    if (ImGui::DragFloat("Radius", &radius, 0.01f) && isUpdatable)
    {
        if (radius > 0.0f)
        {
            mActiveEntity->PatchComponent<Sphere>([&](Sphere& b)
            {
                s.radius = radius;
            });
        }
    }
}

void ObjectInspector::RenderCamera(Camera& c, bool isUpdatable)
{
    bool isPrimary = c.primary;
    Jnrlib::Float focalDistance = c.focalDistance;
    glm::vec2 viewportSize = c.viewportSize, projectionSize = c.projectionSize;
    glm::vec3 rotation = glm::vec3(c.pitch, c.yaw, c.roll);
    if (ImGui::Checkbox("Primary", &isPrimary) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Camera>([&](Camera& c)
        {
            c.primary = isPrimary;
        });
    }

    if (ImGui::DragFloat("Focal distance", &focalDistance, 0.01f) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Camera>([&](Camera& c)
        {
            c.focalDistance = focalDistance;
        });
    }

    if (ImGui::DragFloat2("Viewport size", (float*)&viewportSize, 0.01f) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Camera>([&](Camera& c)
        {
            c.viewportSize = viewportSize;
        });
    }

    if (ImGui::DragFloat2("Projection size", (float*)&projectionSize, 0.01f) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Camera>([&](Camera& c)
        {
            c.projectionSize = projectionSize;
        });
    }

    if (ImGui::DragFloat3("Euler rotation", (float*)&rotation, 0.01f) && isUpdatable)
    {
        mActiveEntity->PatchComponent<Camera>([&](Camera& c)
        {
            c.pitch = rotation.x;
            c.yaw = rotation.y;
            c.roll = rotation.z;
        });
    }
}

void ObjectInspector::RenderMesh(Mesh& m, bool isUpdatable)
{
    /* TODO: Create a material inspector and set the material there */
    ImGui::Text("Material name: %s", m.material->GetName().c_str());

    /* TODO: Create a combo box / drop down menu from which you can select the mesh */
    ImGui::Text("Mesh: %s", m.name.c_str());
}
