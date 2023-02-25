#pragma once


#include "Jnrlib.h"
#include "CreateInfo/CameraCreateInfo.h"
#include "Constants.h"

namespace Common
{
    class Camera
    {
    public:
        Camera(CreateInfo::Camera const&);

    public:
        Jnrlib::Position const& GetPosition() const;
        Jnrlib::Direction const& GetForwardDirection() const;
        Jnrlib::Direction const& GetRightDirection() const;
        Jnrlib::Direction const& GetUpDirection() const;
        Jnrlib::Float GetFocalDistance() const;
        Jnrlib::Float GetViewportWidth() const;
        Jnrlib::Float GetViewportHeight() const;

        Jnrlib::Float GetRoll() const;
        Jnrlib::Float GetYaw() const;
        Jnrlib::Float GetPitch() const;

        Jnrlib::Float GetFieldOfView() const;

        Jnrlib::Direction const& GetLowerLeftCorner() const;

        void MoveForward(float amount);
        void MoveBackward(float amount);
        void StrafeRight(float amount);
        void StrafeLeft(float amount);
        
        void Roll(float amount);
        void Yaw(float amount);
        void Pitch(float amount);

        glm::mat4x4 const& GetView() const;
        glm::mat4x4 const& GetProjection() const;

        uint32_t GetDirtyFrames() const;

        void CalculateViewMatrix();
        void CalculateProjectionMatrix();
        void CalculateMatrices();

        void PerformUpdate();

    private:
        void CalculateVectors();

        void MarkDirty();

        void CalculateLowerLeftCorner();

    private:
        glm::vec3 mPosition;

        glm::vec3 mForwardDirection = glm::vec3(0.0f);
        glm::vec3 mRightDirection = glm::vec3(0.0f);
        glm::vec3 mUpDirection = glm::vec3(0.0f);

        Jnrlib::Float mFocalDistance = 0.0f, mFieldOfView = 0.0f, mAspectRatio = 0.0f;
        Jnrlib::Float mViewportWidth = 0.0f, mViewportHeight = 0.0f;

        Jnrlib::Float mRoll, mPitch, mYaw;

        uint32_t mDirtyFrames = Constants::FRAMES_IN_FLIGHT;

        glm::mat4x4 mView;
        glm::mat4x4 mProjection;

        Jnrlib::Direction mLowerLeftCorner;
    };

}
