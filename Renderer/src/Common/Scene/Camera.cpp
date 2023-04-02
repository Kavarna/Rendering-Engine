#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace Common;

Camera::Camera(CreateInfo::Camera const& info) :
    mPosition(info.position),
    mFocalDistance(info.focalLength), mFieldOfView(info.fieldOfView),
    mAspectRatio(info.aspectRatio),
    mViewportWidth(info.viewportWidth),
    mViewportHeight(info.viewportHeight),
    mRoll(info.roll), mYaw(info.yaw), mPitch(info.pitch),
    mProjectionWidth(info.projectionWidth), mProjectionHeight(info.projectionHeight)
{
    LOG(INFO) << "Creating camera with info: " << info;
    CalculateVectors();
    CalculateUpperLeftCorner();
    CalculateMatrices();
}

Jnrlib::Position const& Camera::GetPosition() const
{
    return mPosition;
}

Jnrlib::Direction const& Camera::GetForwardDirection() const
{
    return mForwardDirection;
}

Jnrlib::Direction const& Camera::GetRightDirection() const
{
    return mRightDirection;
}

Jnrlib::Direction const& Camera::GetUpDirection() const
{
    return mUpDirection;
}

Jnrlib::Float Camera::GetFocalDistance() const
{
    return mFocalDistance;
}

Jnrlib::Float Camera::GetViewportWidth() const
{
    return mViewportWidth;
}

Jnrlib::Float Camera::GetViewportHeight() const
{
    return mViewportHeight;
}

Jnrlib::Float Camera::GetRoll() const
{
    return mRoll;
}

Jnrlib::Float Camera::GetYaw() const
{
    return mYaw;
}

Jnrlib::Float Camera::GetPitch() const
{
    return mPitch;
}

Jnrlib::Float Camera::GetFieldOfView() const
{
    return mFieldOfView;
}

Jnrlib::Float Camera::GetProjectionWidth() const
{
    return mProjectionHeight;
}

Jnrlib::Float Camera::GetProjectionHeight() const
{
    return mProjectionWidth;
}

Ray Camera::GetRayForPixel(uint32_t x, uint32_t y) const
{
    Jnrlib::Float u = ((Jnrlib::Float)x) / (mProjectionWidth - 1);
    Jnrlib::Float v = ((Jnrlib::Float)y) / (mProjectionHeight - 1);

    return Ray(mPosition, mUpperLeftCorner + u * mRightDirection * mViewportWidth - v * mUpDirection * mViewportHeight - mPosition);
}

Jnrlib::Direction const& Camera::GetLowerLeftCorner() const
{
    return mUpperLeftCorner;
}

void Camera::MoveForward(float amount)
{
    mPosition = mPosition + mForwardDirection * amount;
    MarkDirty();
}

void Camera::MoveBackward(float amount)
{
    MoveForward(-amount);
}

void Camera::StrafeRight(float amount)
{
    mPosition = mPosition + mRightDirection * amount;
    MarkDirty();
}

void Camera::StrafeLeft(float amount)
{
    StrafeRight(-amount);
}

void Camera::Roll(float amount)
{
    mRoll += amount;
    CalculateVectors();
}

void Camera::Yaw(float amount)
{
    mYaw -= amount;
    CalculateVectors();
}

void Camera::Pitch(float amount)
{
    mPitch -= amount;
    CalculateVectors();
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

void Camera::CalculateVectors()
{
    auto rotateMatrix = glm::eulerAngleXYZ(mPitch, mYaw, mRoll);

    static glm::vec4 defaultForwardDirection = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    static glm::vec4 defaultRightDirection = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    static glm::vec4 defaultUpDirection = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    mForwardDirection = defaultForwardDirection * rotateMatrix;
    mRightDirection = defaultRightDirection * rotateMatrix;
    mUpDirection = glm::cross(mForwardDirection, mRightDirection);

    MarkDirty();
}

void Camera::MarkDirty()
{
    mDirtyFrames = Constants::FRAMES_IN_FLIGHT;
}

void Camera::CalculateUpperLeftCorner()
{
    mUpperLeftCorner = mPosition +
        mForwardDirection * mFocalDistance - mRightDirection * mViewportWidth * Jnrlib::Half +
        mUpDirection * mViewportHeight * Jnrlib::Half;
}

void Camera::CalculateViewMatrix()
{
    mView = glm::lookAt(GetPosition(), GetPosition() + GetForwardDirection(), GetUpDirection());
}

void Camera::CalculateProjectionMatrix()
{
    mProjection = glm::perspectiveLH(mFieldOfView, mAspectRatio, 0.1f, 1000.0f);
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
