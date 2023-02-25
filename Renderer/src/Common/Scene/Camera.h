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
        Jnrlib::Float const& GetFocalDistance() const;
        Jnrlib::Float const& GetViewportWidth() const;
        Jnrlib::Float const& GetViewportHeight() const;

        Jnrlib::Direction const& GetLowerLeftCorner() const;

        void MoveForward(float amount);
        void MoveBackward(float amount);
        void StrafeRight(float amount);
        void StrafeLeft(float amount);
        
        glm::mat4x4 const& GetView() const;
        glm::mat4x4 const& GetProjection() const;

        uint32_t GetDirtyFrames() const;

        void CalculateViewMatrix();
        void CalculateProjectionMatrix();
        void CalculateMatrices();

        void PerformUpdate();

    private:
        void MarkDirty();

        void CalculateLowerLeftCorner();

    private:
        CreateInfo::Camera mInfo;

        uint32_t mDirtyFrames = Constants::FRAMES_IN_FLIGHT;

        glm::mat4x4 mView;
        glm::mat4x4 mProjection;

        Jnrlib::Direction mLowerLeftCorner;
    };

}
