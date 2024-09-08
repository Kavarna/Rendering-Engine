#include "SceneCreateInfo.h"

#include "TypeHelpers.h"
#include <boost/algorithm/string.hpp>

#include "Constants.h"

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

    AccelerationType GetAccelerationTypeFromString(std::string const& str)
    {
        auto type = magic_enum::enum_cast<AccelerationType>(str);
        if (type.has_value())
        {
            return *type;
        }
        else
        {
            return AccelerationType::None;
        }
    }

    std::string GetStringFromAccelerationType(AccelerationType accelerationType)
    {
        return std::string(magic_enum::enum_name(accelerationType));
    }

    void to_json(nlohmann::json& j, const AccelerationStructure& a)
    {
        j["type"] = GetStringFromAccelerationType(a.accelerationType);
        if (a.accelerationType == AccelerationType::BVH)
        {
            j["split-type"] = magic_enum::enum_name(a.splitType);
            j["max-prims-in-node"] = a.maxPrimsInNode;
        }
        else if (a.accelerationType == AccelerationType::KdTree)
        {

        }
    }

    void from_json(const nlohmann::json& j, AccelerationStructure& a)
    {
        if (j.contains("type"))
        {
            std::string typeString;
            j.at("type").get_to(typeString);
            AccelerationType type = GetAccelerationTypeFromString(typeString);
            a.accelerationType = type;
            switch (type)
            {
                case CreateInfo::AccelerationType::BVH:
                {
                    std::string splitTypeString;
                    if (j.contains("split-type"))
                    {
                        std::string splitTypeString;
                        j.at("split-type").get_to(splitTypeString);
                        auto splitTypeOptional = magic_enum::enum_cast<Common::Accelerators::BVH::SplitType>(splitTypeString);
                        if (splitTypeOptional.has_value())
                        {
                            a.splitType = *splitTypeOptional;
                        }
                    }
                    if (j.contains("max-prims-in-node"))
                    {
                        j.at("max-prims-in-node").get_to(a.maxPrimsInNode);
                    }
                    break;
                }
                case CreateInfo::AccelerationType::KdTree:
                    break;
                case CreateInfo::AccelerationType::None:
                default:
                    break;
            }
        }
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

        if (p.accelerationInfo.accelerationType != AccelerationType::None)
        {
            to_json(j["acceleration-strucutre"], p.accelerationInfo);
        }
    }

    void from_json(const nlohmann::json & j, Primitive& p)
    {
        std::string primitiveTypeString;
        j.at("type").get_to(primitiveTypeString);
        std::string positionString, materialName;
        j.at("name").get_to(p.name);

        p.name.resize(Common::Constants::MAX_NAME_SIZE);

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

        if (j.contains("acceleration-structure"))
        {
            from_json(j["acceleration-structure"], p.accelerationInfo);
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