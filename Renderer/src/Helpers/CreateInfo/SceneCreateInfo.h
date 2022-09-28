#pragma once


#include <Jnrlib.h>

#include <nlohmann/json.hpp>

#include "CameraCreateInfo.h"

namespace CreateInfo
{
    enum class RendererType
    {
        PathTracing,
        None
    };
    RendererType GetRendererTypeFromString(std::string const& str);
    std::string GetStringFromRendererType(RendererType rendererType);

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
        RendererType rendererType;
        std::string outputFile;
        ImageInfo imageInfo;

        friend std::ostream& operator << (std::ostream& stream, Scene const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Scene& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Scene& p);
    void from_json(const nlohmann::json& j, Scene& p);
}