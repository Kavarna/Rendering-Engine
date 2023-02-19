#pragma once


#include "Jnrlib.h"
#include "CreateInfo/CameraCreateInfo.h"

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
        
        glm::mat4x4 const& GetView() const;
        glm::mat4x4 const& GetProjection() const;

    private:
        void CalculateLowerLeftCorner();
        void CalculateMatrices();

    private:
        CreateInfo::Camera mInfo;

        glm::mat4x4 mView;
        mutable glm::mat4x4 mProjection;

        Jnrlib::Direction mLowerLeftCorner;
    };

}
