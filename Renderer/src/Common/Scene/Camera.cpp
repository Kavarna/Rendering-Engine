#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

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

void Common::Camera::MoveForward(float amount)
{
    mInfo.position = mInfo.position + mInfo.forwardDirection * amount;
    MarkDirty();
}

void Common::Camera::MoveBackward(float amount)
{
    MoveForward(-amount);
}

void Common::Camera::StrafeRight(float amount)
{
    mInfo.position = mInfo.position + mInfo.rightDirection * amount;
    MarkDirty();
}

void Common::Camera::StrafeLeft(float amount)
{
    StrafeRight(-amount);
}

glm::mat4x4 const& Camera::GetView() const
{
    return mView;
}

glm::mat4x4 const& Camera::GetProjection() const
{
    return mProjection;
}

uint32_t Camera::GetDirtyFrames() const
{
    return mDirtyFrames;
}

void Camera::MarkDirty()
{
    mDirtyFrames = Constants::FRAMES_IN_FLIGHT;
}

void Camera::CalculateLowerLeftCorner()
{
    mLowerLeftCorner = mInfo.position +
        mInfo.forwardDirection * mInfo.focalLength - mInfo.rightDirection * mInfo.viewportWidth * Jnrlib::Half +
        mInfo.upDirection * mInfo.viewportHeight * Jnrlib::Half;
}

void Camera::CalculateViewMatrix()
{
    mView = glm::lookAt(mInfo.position, mInfo.position + mInfo.forwardDirection, mInfo.upDirection);
}

void Camera::CalculateProjectionMatrix()
{
    mProjection = glm::perspectiveLH(mInfo.fieldOfView, mInfo.aspectRatio, 0.1f, 100.0f);
    mProjection[1][1] *= -1;
}

void Camera::CalculateMatrices()
{
    CalculateViewMatrix();
    CalculateProjectionMatrix();
}

void Camera::PerformUpdate()
{
    if (mDirtyFrames)
        mDirtyFrames--;
}
