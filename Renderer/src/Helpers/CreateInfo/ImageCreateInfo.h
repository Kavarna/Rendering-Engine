#pragma once

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{
    struct Image
    {

        friend std::ostream& operator << (std::ostream& stream, Image const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Image const& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Image& p);
    void from_json(const nlohmann::json& j, Image& p);

}
