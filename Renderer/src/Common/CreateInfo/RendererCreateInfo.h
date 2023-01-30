#pragma once 

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{
    enum class RendererType
    {
        PathTracing,
        None
    };
    RendererType GetRendererTypeFromString(std::string const& str);
    std::string GetStringFromRendererType(RendererType rendererType);

    struct Renderer
    {
        RendererType rendererType;
        uint32_t numSamples;
        uint32_t maxDepth;

        friend std::ostream& operator << (std::ostream& stream, Renderer const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Renderer& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Renderer& p);
    void from_json(const nlohmann::json& j, Renderer& p);
}