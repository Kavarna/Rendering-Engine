#include "SceneCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

#include "Sphere.h"

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

    std::ostream& operator<<(std::ostream& stream, ImageInfo const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, ImageInfo& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const ImageInfo& p)
    {
        j["width"] = p.width;
        j["height"] = p.height;
    }

    void from_json(const nlohmann::json & j, ImageInfo & p)
    {
        j.at("width").get_to(p.width);
        j.at("height").get_to(p.height);
    }

    std::ostream& operator<<(std::ostream& stream, Scene const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, Scene& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const Scene& sceneInfo)
    {
        j["renderer-type"] = GetStringFromRendererType(sceneInfo.rendererType);
        j["output-file"] = sceneInfo.outputFile;
        to_json(j["image-info"], sceneInfo.imageInfo);

        for (const auto& primitive : sceneInfo.primitives)
        {
            json object;
            if (Sphere* s = dynamic_cast<Sphere*>(primitive.get()); s != nullptr)
            {
                to_json(object, *s);
            }
            j["objects"].push_back(object);
        }
    }

    void from_json(const nlohmann::json& j, Scene& p)
    {
        std::string rendererTypeString;
        j.at("renderer-type").get_to(rendererTypeString);
        p.rendererType = GetRendererTypeFromString(rendererTypeString);
        j.at("output-file").get_to(p.outputFile);
        j.at("image-info").get_to(p.imageInfo);

        if (j.contains("objects"))
        {
            json objectsJson = j["objects"];
            p.primitives.reserve(objectsJson.size());
            for (auto const& object : objectsJson)
            {
                std::string type = object["type"].get<std::string>();
                if (boost::iequals(type, "sphere"))
                {
                    std::unique_ptr<Sphere> sphere = std::make_unique<Sphere>();
                    from_json(object, *sphere);
                    p.primitives.emplace_back(std::move(sphere));
                }
            }
        }
    }

}