#include "SceneCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

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
        auto type = magic_enum::enum_cast<PrimitiveType>(str);
        if (type.has_value())
        {
            return *type;
        }
        else
        {
            return PrimitiveType::None;
        }
    }

    std::string GetStringFromPrimitiveType(PrimitiveType primitiveType)
    {
        return std::string(magic_enum::enum_name(primitiveType));
    }

    void to_json(nlohmann::json& j, const Primitive& p)
    {
        j["type"] = GetStringFromPrimitiveType(p.primitiveType);
        j["name"] = p.name;
        j["position"] = Jnrlib::to_string(p.position);
        j["material"] = p.materialName;
        
        if (p.parentName.size() > 0)
        {
            j["parent"] = p.parentName;
        }
        
        if (p.primitiveType == PrimitiveType::Sphere)
        {
            j["radius"] = p.radius;
        }
    }

    void from_json(const nlohmann::json & j, Primitive& p)
    {
        std::string primitiveTypeString;
        j.at("type").get_to(primitiveTypeString);
        std::string positionString, materialName;
        j.at("name").get_to(p.name);

        p.primitiveType = GetPrimitiveTypeFromString(primitiveTypeString);
        CHECK(p.primitiveType != PrimitiveType::None) << "Primitive type " << primitiveTypeString << " is not valid";

        j.at("position").get_to(positionString); p.position = Jnrlib::to_type<Jnrlib::Position>(positionString);
        j.at("material").get_to(p.materialName);
        if (j.contains("parent"))
        {
            j.at("parent").get_to(p.parentName);
        }

        if (p.primitiveType == PrimitiveType::Sphere)
        {
            j.at("radius").get_to(p.radius);
        }
        else if (p.primitiveType == PrimitiveType::Mesh)
        {
            j.at("path").get_to(p.path);
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
        j["also-build-for-realtime-rendering"] = sceneInfo.alsoBuildForRealTimeRendering;
        to_json(j["image-info"], sceneInfo.imageInfo);
        to_json(j["camera"], sceneInfo.cameraInfo);

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
        if (j.contains("camera"))
        {
            from_json(j["camera"], p.cameraInfo);
        }
        if (j.contains("also-build-for-realtime-rendering"))
        {
            j.at("also-build-for-realtime-rendering").get_to(p.alsoBuildForRealTimeRendering);
        }
        else
        {
            p.alsoBuildForRealTimeRendering = false;
        }
    }

}