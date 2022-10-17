#include "RendererCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

using json = nlohmann::json;


namespace CreateInfo
{
    RendererType GetRendererTypeFromString(std::string const& str)
    {
        if (boost::iequals(str, "pathtracing"))
            return RendererType::PathTracing;
        return RendererType::None;
    }

    std::string GetStringFromRendererType(RendererType rendererType)
    {
        switch (rendererType)
        {
            case RendererType::PathTracing:
                return "PathTracing";
            case RendererType::None:
            default:
                return "None";
        }
    }

    std::ostream& operator<<(std::ostream& stream, Renderer const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, Renderer& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const Renderer& p)
    {
        j["num-samples"] = p.numSamples;
        j["renderer-type"] = GetStringFromRendererType(p.rendererType);
        j["max-depth"] = p.maxDepth;
    }

    void from_json(const nlohmann::json& j, Renderer& p)
    {
        std::string rendererTypeString;
        j.at("renderer-type").get_to(rendererTypeString);
        p.rendererType = GetRendererTypeFromString(rendererTypeString);
        j.at("num-samples").get_to(p.numSamples);
        j.at("max-depth").get_to(p.maxDepth);
    }
}