#include "CameraUtils.h"

#include "EditorCamera.h"

#include "Scene/Components/Camera.h"
#include "Scene/Components/Base.h"

struct RayForPixelInfo
{
    Jnrlib::Float pixelX;
    Jnrlib::Float pixelY;

    glm::vec2 projectionSize;
    glm::vec2 viewportSize;

    Jnrlib::Position position;
    Jnrlib::Position upperLeftForner;
    Jnrlib::Direction forwardDirection, rightDirection, upDirection;
};

Common::Ray GetRayForPixel(RayForPixelInfo const& rp)
{
    Jnrlib::Float u = (rp.pixelX) / (rp.projectionSize.x - 1);
    Jnrlib::Float v = (rp.pixelY) / (rp.projectionSize.y - 1);

    return Common::Ray(
        rp.position, rp.upperLeftForner +
        u * rp.rightDirection * rp.viewportSize.x -
        v * rp.upDirection * rp.viewportSize.y - rp.position);
}

Common::Ray Common::CameraUtils::GetRayForPixel(Common::EditorCamera const* camera, uint32_t x, uint32_t y)
{
    RayForPixelInfo rp{};
    {
        rp.pixelX = (Jnrlib::Float)x;
        rp.pixelY = (Jnrlib::Float)y;

        rp.projectionSize = camera->GetProjectionSize();
        rp.viewportSize = camera->GetViewportSize();

        rp.position = camera->GetPosition();
        rp.upperLeftForner = camera->GetUpperLeftCorner();
        rp.forwardDirection = camera->GetForwardDirection();
        rp.rightDirection = camera->GetRightDirection();
        rp.upDirection = camera->GetUpDirection();
    }

    return ::GetRayForPixel(rp);
}

Common::Ray Common::CameraUtils::GetRayForPixel(Common::Components::Camera const* camera, uint32_t x, uint32_t y)
{
    RayForPixelInfo rp{};
    {
        rp.pixelX = (Jnrlib::Float)x;
        rp.pixelY = (Jnrlib::Float)y;

        rp.projectionSize = camera->projectionSize;
        rp.viewportSize = camera->viewportSize;

        rp.position = camera->baseComponent.position;
        rp.upperLeftForner = camera->upperLeftCorner;
        rp.forwardDirection = camera->forwardDirection;
        rp.rightDirection = camera->rightDirection;
        rp.upDirection = camera->upDirection;
    }

    return ::GetRayForPixel(rp);
}
