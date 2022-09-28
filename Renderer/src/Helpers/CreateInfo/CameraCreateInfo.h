#pragma once

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{
    struct Camera
    {
        Jnrlib::Position position = Jnrlib::Position(0.0f);
        Jnrlib::Direction forwardDirection = Jnrlib::Direction(0.0f, 0.0f, 1.0f);
        Jnrlib::Direction rightDirection = Jnrlib::Direction(1.0f, 0.0f, 0.0f);
        Jnrlib::Direction upDirection = Jnrlib::Direction(0.0f, 1.0f, 0.0f);

        Jnrlib::Float aspectRatio = -1.f;
        Jnrlib::Float viewportHeight = -1.f;
        Jnrlib::Float viewportWidth = -1.f;
        Jnrlib::Float focalLength = 1.0f;

        void RecalculateViewport(uint32_t width, uint32_t height);

        friend std::ostream& operator << (std::ostream& stream, Camera const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Camera const& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Camera& p);
    void from_json(const nlohmann::json& j, Camera& p);

}
