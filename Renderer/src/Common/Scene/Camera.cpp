#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"

using namespace Common;

Camera::Camera(CreateInfo::Camera const& info) :
    mInfo(info)
{
    LOG(INFO) << "Creating camera with info: " << info;
    CalculateLowerLeftCorner();
    CalculateMatrices();
}

Jnrlib::Position const& Camera::GetPosition() const
{
    return mInfo.position;
}

Jnrlib::Direction const& Camera::GetForwardDirection() const
{
    return mInfo.forwardDirection;
}

Jnrlib::Direction const& Camera::GetRightDirection() const
{
    return mInfo.rightDirection;
}

Jnrlib::Direction const& Camera::GetUpDirection() const
{
    return mInfo.upDirection;
}

Jnrlib::Float const& Camera::GetFocalDistance() const
{
    return mInfo.focalLength;
}

Jnrlib::Float const& Camera::GetViewportWidth() const
{
    return mInfo.viewportHeight;
}

Jnrlib::Float const& Camera::GetViewportHeight() const
{
    return mInfo.viewportHeight;
}

Jnrlib::Direction const& Camera::GetLowerLeftCorner() const
{
    return mLowerLeftCorner;
}

glm::mat4x4 const& Camera::GetView() const
{
    return mView;
}

glm::mat4x4 const& Camera::GetProjection() const
{
    return mProjection;
}

void Camera::CalculateLowerLeftCorner()
{
    mLowerLeftCorner = mInfo.position +
        mInfo.forwardDirection * mInfo.focalLength - mInfo.rightDirection * mInfo.viewportWidth * Jnrlib::Half +
        mInfo.upDirection * mInfo.viewportHeight * Jnrlib::Half;
}

void Camera::CalculateMatrices()
{
    mView = glm::lookAt(mInfo.position, mInfo.position + mInfo.forwardDirection, mInfo.upDirection);
    mProjection = glm::perspectiveLH(mInfo.fieldOfView, mInfo.aspectRatio, 0.1f, 100.0f);
    mProjection[1][1] *= -1;
}
