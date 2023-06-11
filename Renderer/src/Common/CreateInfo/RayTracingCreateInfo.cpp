#include "RayTracingCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;


namespace CreateInfo
{
    RayTracingType GetRendererTypeFromString(std::string const& str)
    {
        if (boost::iequals(str, "pathtracing"))
            return RayTracingType::PathTracing;
        if (boost::iequals(str, "simple raytracing") || boost::iequals(str, "raytracing"))
            return RayTracingType::SimpleRayTracing;
        return RayTracingType::COUNT;
    }

    std::string GetStringFromRendererType(RayTracingType rendererType)
    {
        switch (rendererType)
        {
            case RayTracingType::PathTracing:
                return "PathTracing";
            case RayTracingType::SimpleRayTracing:
                return "Simple RayTracing";
            case RayTracingType::COUNT:
            default:
                return "None";
        }
    }

    std::ostream& operator<<(std::ostream& stream, RayTracing const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, RayTracing& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const RayTracing& p)
    {
        j["num-samples"] = p.numSamples;
        j["renderer-type"] = GetStringFromRendererType(p.rendererType);
        j["max-depth"] = p.maxDepth;
    }

    void from_json(const nlohmann::json& j, RayTracing& p)
    {
        std::string rendererTypeString;
        j.at("renderer-type").get_to(rendererTypeString);
        p.rendererType = GetRendererTypeFromString(rendererTypeString);
        j.at("max-depth").get_to(p.maxDepth);
        if (p.rendererType == RayTracingType::PathTracing)
        {
            j.at("num-samples").get_to(p.numSamples);
        }
    }
}