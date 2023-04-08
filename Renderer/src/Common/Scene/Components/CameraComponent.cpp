#include "CameraComponent.h"

#include "Constants.h"

#include "BaseComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

glm::vec3 Common::Components::Camera::GetUpperLeftCorner() const
{
    return upperLeftCorner;
}

glm::vec3 Common::Components::Camera::GetForwardDirection() const
{
    return forwardDirection;
}

glm::vec3 Common::Components::Camera::GetRightDirection() const
{
    return rightDirection;
}

glm::vec3 Common::Components::Camera::GetUpDirection() const
{
    return upDirection;
}

void Common::Components::Camera::Update()
{
    auto rotateMatrix = glm::eulerAngleXYZ(pitch, yaw, roll);

    forwardDirection = Constants::DEFAULT_FORWARD_DIRECTION * rotateMatrix;
    rightDirection = Constants::DEFAULT_RIGHT_DIRECTION * rotateMatrix;
    upDirection = glm::cross(forwardDirection, rightDirection);

    upperLeftCorner = baseComponent.position +
        forwardDirection * focalDistance - rightDirection * viewportSize.x * Jnrlib::Half +
        upDirection * viewportSize.y * Jnrlib::Half;
}
