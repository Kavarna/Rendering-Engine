#pragma once

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{
    struct Camera
    {
        Jnrlib::Position position = Jnrlib::Position(Jnrlib::Zero);
        Jnrlib::Float roll = Jnrlib::Zero, pitch = Jnrlib::Zero, yaw = Jnrlib::Zero;

        Jnrlib::Float aspectRatio = -Jnrlib::One;
        Jnrlib::Float viewportHeight = -Jnrlib::One;
        Jnrlib::Float viewportWidth = -Jnrlib::One;
        Jnrlib::Float focalLength = Jnrlib::One;
        Jnrlib::Float fieldOfView = Jnrlib::PI * Jnrlib::Half;

        void RecalculateViewport(uint32_t width, uint32_t height);

        friend std::ostream& operator << (std::ostream& stream, Camera const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Camera const& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Camera& p);
    void from_json(const nlohmann::json& j, Camera& p);

}
