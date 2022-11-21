#include "Camera.h"

Camera::Camera(CreateInfo::Camera const& info) :
    mInfo(info)
{
    LOG(INFO) << "Creating camera with info: " << info;
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
    return mInfo.position +
        mInfo.forwardDirection * mInfo.focalLength - mInfo.rightDirection * mInfo.viewportWidth / Jnrlib::Two +
        mInfo.upDirection * mInfo.viewportHeight / Jnrlib::Two;
}
