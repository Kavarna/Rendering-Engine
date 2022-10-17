#pragma once


#include <Jnrlib.h>

#include <nlohmann/json.hpp>
#include "Primitive.h"

namespace CreateInfo
{
    struct ImageInfo
    {
        std::size_t width;
        std::size_t height;
        // TODO: add pixel mappings

        friend std::ostream& operator << (std::ostream& stream, ImageInfo const& info);
        friend std::istream& operator >> (std::istream& stream, ImageInfo& info);
    };

    void to_json(nlohmann::json& j, const ImageInfo& p);
    void from_json(const nlohmann::json& j, ImageInfo& p);


    struct Scene
    {
        std::string outputFile;
        ImageInfo imageInfo;

        std::vector<std::unique_ptr<Primitive>> primitives;

        friend std::ostream& operator << (std::ostream& stream, Scene const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Scene& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Scene& p);
    void from_json(const nlohmann::json& j, Scene& p);
}