#pragma once


#include <Jnrlib.h>

#include <nlohmann/json.hpp>
#include "CameraCreateInfo.h"
#include "Common/Scene/Accelerators/BVH.h"

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
        None = 0,
        Sphere,
        Mesh
    };
    PrimitiveType GetPrimitiveTypeFromString(std::string const& str);
    std::string GetStringFromPrimitiveType(PrimitiveType primitiveType);

    enum class AccelerationType
    {
        None = 0,
        BVH,
        KdTree,
    };
    AccelerationType GetAccelerationTypeFromString(std::string const& str);
    std::string GetStringFromAccelerationType(AccelerationType accelerationType);

    struct AccelerationStructure
    {
        AccelerationType accelerationType = AccelerationType::None;
        /* BVH */
        Common::Accelerators::BVH::SplitType splitType = Common::Accelerators::BVH::SplitType::SAH;
        uint32_t maxPrimsInNode = 5;

        /* Kd Tree */
    };

    void to_json(nlohmann::json& j, const AccelerationStructure& a);
    void from_json(const nlohmann::json& j, AccelerationStructure& a);

    struct Primitive
    {
        PrimitiveType primitiveType;
        
        std::string name, materialName;
        Jnrlib::Position position;

        std::string parentName;

        /* Sphere info */
        float radius;

        /* Mesh info */
        std::string path;

        /* Acceleration info */
        AccelerationStructure accelerationInfo;
    };

    void to_json(nlohmann::json& j, const Primitive& p);
    void from_json(const nlohmann::json& j, Primitive& p);

    struct Scene
    {
        std::string outputFile;
        ImageInfo imageInfo;

        CreateInfo::Camera cameraInfo;

        std::vector<Primitive> primitives;

        bool alsoBuildForRealTimeRendering = false;

        friend std::ostream& operator << (std::ostream& stream, Scene const& cameraInfo);
        friend std::istream& operator >> (std::istream& stream, Scene& cameraInfo);
    };

    void to_json(nlohmann::json& j, const Scene& p);
    void from_json(const nlohmann::json& j, Scene& p);
}