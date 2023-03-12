#pragma once


#include <Jnrlib.h>

#include <nlohmann/json.hpp>

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

    enum class PrimitiveType
    {
        Sphere = 0,
        None
    };
    PrimitiveType GetPrimitiveTypeFromString(std::string const& str);
    std::string GetStringFromPrimitiveType(PrimitiveType primitiveType);


    struct Primitive
    {
        PrimitiveType primitiveType;
        
        std::string name, materialName;
        Jnrlib::Position position;

        std::string parentName;

        /* Sphere info */
        float radius;

    };

    void to_json(nlohmann::json& j, const Primitive& p);
    void from_json(const nlohmann::json& j, Primitive& p);

    struct Scene
    {
        std::string outputFile;
        ImageInfo imageInfo;

        std::vector<Primitive> primitives;

        bool alsoBuildForRealTimeRendering = false;

        friend std::ostream& operator << (std::ostream& stream, Scene const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Scene& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Scene& p);
    void from_json(const nlohmann::json& j, Scene& p);
}