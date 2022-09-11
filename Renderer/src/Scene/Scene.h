#pragma once


#include "Jnrlib.h"
#include <nlohmann/json.hpp>

enum class RendererType
{
    PathTracing,
    None
};
RendererType GetRendererTypeFromString(std::string const& str);
std::string GetStringFromRendererType(RendererType rendererType);

class Scene
{
public:
    struct SceneCreateInfo
    {
        RendererType rendererType;
        std::string outputFile;
        struct ImageInfo
        {
            std::size_t width;
            std::size_t height;
            // TODO: add pixel mappings
        } imageInfo;

        friend std::ostream& operator << (std::ostream& stream, SceneCreateInfo sceneInfo)
        {
            nlohmann::json j;
            j["renderer-type"] = GetStringFromRendererType(sceneInfo.rendererType);
            j["output-file"] = sceneInfo.outputFile;
            
            nlohmann::json imageInfo;
            imageInfo["width"] = sceneInfo.imageInfo.width;
            imageInfo["height"] = sceneInfo.imageInfo.height;

            j["image-info"] = imageInfo;

            stream << j;
            return stream;
        };
    };
public:
    Scene(SceneCreateInfo const& info);
    ~Scene();

private:
    void RenderToScreen();

public:
    RendererType GetRendererType() const;
    std::string GetOutputFile() const;

    const SceneCreateInfo::ImageInfo& GetImageInfo() const;

private:
    SceneCreateInfo mInfo;



    std::function<void(uint32_t, uint32_t, float, float, float, float)> mWriteColorFunction;
};


