#include "SceneCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

#include "Sphere.h"

using json = nlohmann::json;


namespace CreateInfo
{
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

    PrimitiveType GetPrimitiveTypeFromString(std::string const& str)
    {
        if (boost::iequals(str, "sphere"))
            return PrimitiveType::Sphere;
        return PrimitiveType::None;
    }

    std::string GetStringFromPrimitiveType(PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case CreateInfo::PrimitiveType::Sphere:
                return "Sphere";
            case CreateInfo::PrimitiveType::None:
            default:
                return "None";
        }
    }

    void to_json(nlohmann::json& j, const Primitive& p)
    {
        j["type"] = GetStringFromPrimitiveType(p.primitiveType);
        j["name"] = p.name;
        j["position"] = Jnrlib::to_string(p.position);
        j["material"] = p.materialName;
        
        if (p.primitiveType == PrimitiveType::Sphere)
        {
            j["radius"] = p.radius;
        }
    }

    void from_json(const nlohmann::json & j, Primitive& p)
    {
        std::string primitiveTypeString;
        j.at("type").get_to(primitiveTypeString);
        p.primitiveType = GetPrimitiveTypeFromString(primitiveTypeString);

        std::string positionString, materialName;
        j.at("name").get_to(p.name);
        j.at("position").get_to(positionString); p.position = Jnrlib::to_type<Jnrlib::Position>(positionString);
        j.at("material").get_to(p.materialName);

        if (p.primitiveType == PrimitiveType::Sphere)
        {
            j.at("radius").get_to(p.radius);
        }
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
        j["output-file"] = sceneInfo.outputFile;
        to_json(j["image-info"], sceneInfo.imageInfo);

        for (const auto& primitive : sceneInfo.primitives)
        {
            json object;
            to_json(object, primitive);
            j["objects"].push_back(object);
        }
    }

    void from_json(const nlohmann::json& j, Scene& p)
    {
        std::string rendererTypeString;
        j.at("output-file").get_to(p.outputFile);
        j.at("image-info").get_to(p.imageInfo);

        if (j.contains("objects"))
        {
            json objectsJson = j["objects"];
            p.primitives.reserve(objectsJson.size());
            for (auto const& object : objectsJson)
            {
                Primitive primitive;
                from_json(object, primitive);
                p.primitives.emplace_back(primitive);
            }
        }
    }

}