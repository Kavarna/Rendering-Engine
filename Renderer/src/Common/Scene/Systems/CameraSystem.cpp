#include "CameraSystem.h"
#include "Scene/Scene.h"

#include "Constants.h"

#include "Scene/Components/BaseComponent.h"
#include "Scene/Components/CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace Common;

Systems::Camera::Camera(Scene* scene) : 
    mScene(scene)
{

}

void Systems::Camera::Update()
{
    if (mFirstCall)
    {
        InitializeCameras();
    }
    else
    {
        UpdateCameras();
    }
}

void Systems::Camera::InitializeCameras()
{
    auto cameras = mScene->GetRegistry().view<Components::Base, Components::Camera>();
    for (auto const& [entity, base, camera] : cameras.each())
    {
        Entity* entityPtr = base.entityPtr;

        auto rotateMatrix = glm::eulerAngleXYZ(camera.pitch, camera.yaw, camera.roll);

        camera.forwardDirection = Constants::DEFAULT_FORWARD_DIRECTION * rotateMatrix;
        camera.rightDirection = Constants::DEFAULT_RIGHT_DIRECTION * rotateMatrix;
        camera.upDirection = glm::cross(camera.forwardDirection, camera.rightDirection);

        camera.upperLeftCorner = base.position +
            camera.forwardDirection * camera.focalDistance - camera.rightDirection * camera.viewportSize.x * Jnrlib::Half +
            camera.upDirection * camera.viewportSize.y * Jnrlib::Half;

        camera.view = glm::lookAt(base.position, base.position + camera.forwardDirection, camera.upDirection);
        camera.projection = glm::perspectiveLH(camera.fieldOfView, camera.aspectRatio, 0.1f, 1000.0f);
        camera.projection[1][1] *= -1;
    }
}

void Systems::Camera::UpdateCameras()
{
    /* TODO: Implement this - will update only cameras that have the Update component and changed */
}