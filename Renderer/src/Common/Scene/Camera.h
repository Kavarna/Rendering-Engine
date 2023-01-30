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

    private:
        void CalculateLowerLeftCorner();

    private:
        CreateInfo::Camera mInfo;

        Jnrlib::Direction mLowerLeftCorner;
    };

}
