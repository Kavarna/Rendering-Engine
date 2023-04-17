#pragma once 

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{
    enum class RayTracingType
    {
        PathTracing,
        None
    };
    RayTracingType GetRendererTypeFromString(std::string const& str);
    std::string GetStringFromRendererType(RayTracingType rendererType);

    struct RayTracing
    {
        RayTracingType rendererType;
        uint32_t numSamples;
        uint32_t maxDepth;

        friend std::ostream& operator << (std::ostream& stream, RayTracing const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, RayTracing& cameraInfo);
    };

    void to_json(nlohmann::json& j, const RayTracing& p);
    void from_json(const nlohmann::json& j, RayTracing& p);
}