#include "EditorCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace Common;

EditorCamera::EditorCamera(CreateInfo::Camera const& info) :
    mPosition(info.position),
    mFocalDistance(info.focalDistance), mFieldOfView(info.fieldOfView),
    mAspectRatio(info.aspectRatio),
    mViewportSize(glm::vec2{info.viewportWidth, info.viewportHeight}),
    mRoll(info.roll), mYaw(info.yaw), mPitch(info.pitch),
    mProjectionSize(glm::vec2{info.projectionWidth, info.projectionHeight})
{
    LOG(INFO) << "Creating camera with info: " << info;
    CalculateVectors();
    CalculateUpperLeftCorner();
    CalculateMatrices();
}

Jnrlib::Position const& EditorCamera::GetPosition() const
{
    return mPosition;
}

Jnrlib::Direction const& EditorCamera::GetForwardDirection() const
{
    return mForwardDirection;
}

Jnrlib::Direction const& EditorCamera::GetRightDirection() const
{
    return mRightDirection;
}

Jnrlib::Direction const& EditorCamera::GetUpDirection() const
{
    return mUpDirection;
}

Jnrlib::Float EditorCamera::GetFocalDistance() const
{
    return mFocalDistance;
}

glm::vec2 EditorCamera::GetViewportSize() const
{
    return mViewportSize;
}

glm::vec2 EditorCamera::GetProjectionSize() const
{
    return mProjectionSize;
}

Jnrlib::Float EditorCamera::GetRoll() const
{
    return mRoll;
}

Jnrlib::Float EditorCamera::GetYaw() const
{
    return mYaw;
}

Jnrlib::Float EditorCamera::GetPitch() const
{
    return mPitch;
}

Jnrlib::Float EditorCamera::GetFieldOfView() const
{
    return mFieldOfView;
}

Jnrlib::Direction const& EditorCamera::GetUpperLeftCorner() const
{
    return mUpperLeftCorner;
}

void EditorCamera::MoveForward(float amount)
{
    mPosition = mPosition + mForwardDirection * amount;
    MarkDirty();
}

void EditorCamera::MoveBackward(float amount)
{
    MoveForward(-amount);
}

void EditorCamera::StrafeRight(float amount)
{
    mPosition = mPosition + mRightDirection * amount;
    MarkDirty();
}

void EditorCamera::StrafeLeft(float amount)
{
    StrafeRight(-amount);
}

void EditorCamera::Roll(float amount)
{
    mRoll += amount;
    CalculateVectors();
}

void EditorCamera::Yaw(float amount)
{
    mYaw -= amount;
    CalculateVectors();
}

void EditorCamera::Pitch(float amount)
{
    mPitch -= amount;
    CalculateVectors();
}

glm::mat4x4 const& EditorCamera::GetView() const
{
    return mView;
}

glm::mat4x4 const& EditorCamera::GetProjection() const
{
    return mProjection;
}

uint32_t EditorCamera::GetDirtyFrames() const
{
    return mDirtyFrames;
}

void EditorCamera::CalculateVectors()
{
    auto rotateMatrix = glm::eulerAngleXYZ(mPitch, mYaw, mRoll);

    mForwardDirection = Constants::DEFAULT_FORWARD_DIRECTION * rotateMatrix;
    mRightDirection = Constants::DEFAULT_RIGHT_DIRECTION * rotateMatrix;
    mUpDirection = glm::cross(mForwardDirection, mRightDirection);

    MarkDirty();
}

void EditorCamera::MarkDirty()
{
    mDirtyFrames = Constants::FRAMES_IN_FLIGHT;
}

void EditorCamera::CalculateUpperLeftCorner()
{
    mUpperLeftCorner = mPosition +
        mForwardDirection * mFocalDistance - mRightDirection * mViewportSize.x * Jnrlib::Half +
        mUpDirection * mViewportSize.y * Jnrlib::Half;
}

void EditorCamera::CalculateViewMatrix()
{
    mView = glm::lookAt(GetPosition(), GetPosition() + GetForwardDirection(), GetUpDirection());
}

void EditorCamera::CalculateProjectionMatrix()
{
    mProjection = glm::perspectiveLH(mFieldOfView, mAspectRatio, 0.1f, 1000.0f);
    mProjection[1][1] *= -1;
}

void EditorCamera::CalculateMatrices()
{
    CalculateViewMatrix();
    CalculateProjectionMatrix();
}

void EditorCamera::PerformUpdate()
{
    if (mDirtyFrames)
        mDirtyFrames--;
}
