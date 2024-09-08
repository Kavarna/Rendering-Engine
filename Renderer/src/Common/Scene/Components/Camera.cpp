#include "Camera.h"

#include "Constants.h"

#include "Base.h"

#include "Common/Scene/Entity.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
    auto *baseComponent = entityPtr->TryGetComponent<Base>();

    auto rotateMatrix = glm::eulerAngleXYZ(pitch, yaw, roll);

    forwardDirection = Constants::DEFAULT_FORWARD_DIRECTION * rotateMatrix;
    rightDirection = Constants::DEFAULT_RIGHT_DIRECTION * rotateMatrix;
    upDirection = glm::cross(forwardDirection, rightDirection);

    Jnrlib::Vec3 scale;
    Jnrlib::Quaternion rotation;
    Jnrlib::Position translation;
    Jnrlib::Vec3 skew;
    Jnrlib::Vec4 perspective;
    glm::decompose(baseComponent->world, scale, rotation, translation, skew, perspective);

    upperLeftCorner = translation +
        forwardDirection * focalDistance - rightDirection * viewportSize.x * Jnrlib::Half +
        upDirection * viewportSize.y * Jnrlib::Half;
}
