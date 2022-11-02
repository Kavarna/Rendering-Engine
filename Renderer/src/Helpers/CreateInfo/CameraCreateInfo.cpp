#include "CameraCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;

namespace CreateInfo
{
    void Camera::RecalculateViewport(uint32_t width, uint32_t height)
    {
        if (aspectRatio == -1)
        {
            aspectRatio = (float)width / (float)height;
        }
        viewportHeight = 2.0f;
        viewportWidth = viewportHeight * aspectRatio;
    }

    std::ostream& operator << (std::ostream& stream, CreateInfo::Camera const& cameraInfo)
    {
        json j;

        to_json(j, cameraInfo);

        stream << j;
        return stream;
    }

    std::istream& operator >> (std::istream& stream, CreateInfo::Camera& cameraInfo)
    {
        json j;

        stream >> j;
        from_json(j, cameraInfo);

        return stream;
    }

    void to_json(nlohmann::json& j, const Camera& cameraInfo)
    {
        j["position"] = Jnrlib::to_string(cameraInfo.position);
        j["forward-direction"] = Jnrlib::to_string(cameraInfo.forwardDirection);
        j["right-direction"] = Jnrlib::to_string(cameraInfo.rightDirection);
        j["up-direction"] = Jnrlib::to_string(cameraInfo.upDirection);

        j["viewport-width"] = cameraInfo.viewportWidth;
        j["viewport-height"] = cameraInfo.viewportHeight;
        j["focal-length"] = cameraInfo.focalLength;
        j["aspect-ratio"] = cameraInfo.aspectRatio;
    }

    void from_json(const nlohmann::json& j, Camera& cameraInfo)
    {
        std::string positionStr, forwardDirection, rightDirection, upDirection;
        j.at("position").get_to(positionStr);
        j.at("forward-direction").get_to(forwardDirection);
        j.at("right-direction").get_to(rightDirection);
        j.at("up-direction").get_to(upDirection);

        cameraInfo.position = Jnrlib::to_type<Jnrlib::Position>(positionStr);
        cameraInfo.forwardDirection = Jnrlib::to_type<Jnrlib::Position>(forwardDirection);
        cameraInfo.rightDirection = Jnrlib::to_type<Jnrlib::Position>(rightDirection);
        cameraInfo.upDirection = Jnrlib::to_type<Jnrlib::Position>(upDirection);

        j.at("viewport-width").get_to(cameraInfo.viewportWidth);
        j.at("viewport-height").get_to(cameraInfo.viewportHeight);
        j.at("focal-length").get_to(cameraInfo.focalLength);
        j.at("aspect-ratio").get_to(cameraInfo.aspectRatio);
    }
}