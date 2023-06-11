#include "CameraCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;

namespace CreateInfo
{
    void Camera::RecalculateViewport(uint32_t width, uint32_t height)
    {
        projectionWidth = (Jnrlib::Float)width;
        projectionHeight = (Jnrlib::Float)height;
        if (aspectRatio == -1)
        {
            aspectRatio = (float)width / (float)height;
        }

        Jnrlib::Float h = tan(fieldOfView * Jnrlib::Half);

        viewportHeight = 2.0f * h;
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
        j["roll"] = cameraInfo.roll;
        j["pitch"] = cameraInfo.pitch;
        j["yaw"] = cameraInfo.yaw;

        j["viewport-width"] = cameraInfo.viewportWidth;
        j["viewport-height"] = cameraInfo.viewportHeight;
        j["focal-distance"] = cameraInfo.focalDistance;
        j["aspect-ratio"] = cameraInfo.aspectRatio;

        j["field-of-view"] = cameraInfo.fieldOfView;
        
        j["projection-width"] = cameraInfo.projectionWidth;
        j["projection-height"] = cameraInfo.projectionHeight;
    }

    void from_json(const nlohmann::json& j, Camera& cameraInfo)
    {
        std::string positionStr;
        j.at("position").get_to(positionStr);
        j.at("field-of-view").get_to(cameraInfo.fieldOfView);
        
        cameraInfo.position = Jnrlib::to_type<Jnrlib::Position>(positionStr);

        /* Optional fields */
        if (j.contains("roll"))
        {
            j.at("roll").get_to(cameraInfo.roll);
        }
        if (j.contains("pitch"))
        {
            j.at("pitch").get_to(cameraInfo.pitch);
        }
        if (j.contains("yaw"))
        {
            j.at("yaw").get_to(cameraInfo.yaw);
        }

        if (j.contains("viewport-width"))
        {
            j.at("viewport-width").get_to(cameraInfo.viewportWidth);
        }
        if (j.contains("viewport-height"))
        {
            j.at("viewport-height").get_to(cameraInfo.viewportHeight);
        }
        if (j.contains("focal-length"))
        {
            j.at("focal-distance").get_to(cameraInfo.focalDistance);
        }
        if (j.contains("aspect-ratio"))
        {
            j.at("aspect-ratio").get_to(cameraInfo.aspectRatio);
        }

        if (j.contains("projection-width"))
        {
            j.at("projection-width").get_to(cameraInfo.projectionWidth);
        }
        if (j.contains("projection-height"))
        {
            j.at("projection-height").get_to(cameraInfo.projectionHeight);
        }

    }
}