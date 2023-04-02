#include "CameraComponent.h"

#include "BaseComponent.h"
#include "UpdateComponent.h"
#include "Constants.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

void UpdateCamera(Common::Entity* cameraEntity)
{
    auto& update = cameraEntity->GetComponent<Common::Components::Update>();
    {
        update.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT;
    }
}

std::unique_ptr<Common::Entity> Common::Components::InitCameraEntity(
    entt::registry& registry, CreateInfo::Camera const& info, Entity* parent,  uint32_t index)
{
    std::unique_ptr<Entity> cameraEntity = std::make_unique<Common::Entity>(registry.create(), registry);

    cameraEntity->AddComponent(
        Components::Update{.dirtyFrames = Common::Constants::FRAMES_IN_FLIGHT, .bufferIndex = index}
    );

    registry.on_update<Components::Camera>().connect<&UpdateCamera>(cameraEntity.get());

    Camera cameraComponent{};
    {
        cameraComponent.position = info.position;
        cameraComponent.focalDistance = info.focalLength;
        cameraComponent.fieldOfView = info.fieldOfView;
        cameraComponent.aspectRatio = info.aspectRatio;
        cameraComponent.viewportWidth = info.viewportWidth;
        cameraComponent.viewportHeight = info.viewportHeight;
        cameraComponent.roll = info.roll;
        cameraComponent.yaw = info.yaw;
        cameraComponent.pitch = info.pitch;
        cameraComponent.projectionWidth = info.projectionWidth;
        cameraComponent.projectionHeight = info.projectionHeight;
    }

    /*cameraComponent.CalculateVectors();
    cameraComponent.CalculateMatrices();
    cameraComponent.CalculateUpperLeftCorner();*/

    return cameraEntity;
}

void Common::Components::Camera::CalculateVectors()
{
    auto rotateMatrix = glm::eulerAngleXYZ(pitch, yaw, roll);

    static glm::vec4 defaultForwardDirection = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    static glm::vec4 defaultRightDirection = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    static glm::vec4 defaultUpDirection = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    forwardDirection = defaultForwardDirection * rotateMatrix;
    rightDirection = defaultRightDirection * rotateMatrix;
    upDirection = glm::cross(forwardDirection, rightDirection);
}

void Common::Components::Camera::CalculateMatrices()
{
    /*CalculateViewMatrix();
    CalculateProjectionMatrix();*/
}

void Common::Components::Camera::CalculateUpperLeftCorner()
{
    upperLeftCorner = position +
        forwardDirection * focalDistance - rightDirection * viewportWidth * Jnrlib::Half +
        upDirection * viewportHeight * Jnrlib::Half;
}
