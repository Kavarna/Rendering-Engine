#include "boost/program_options.hpp"
#include <iostream>
#include <optional>
#include <functional>

#ifdef BUILD_TESTS
#include "gtest/gtest.h"
#endif
#include "Jnrlib.h"
#include "Scene/SceneParser.h"
#include "Vulkan/Renderer.h"
#include "RayTracing/Renderer.h"
#include "Common/MaterialManager.h"
#include "Common/Scene/Scene.h"
#include "ThreadPool.h"
#include "Editor/Editor.h"

enum class ApplicationMode
{
    UNDEFINED = 0,
    TESTING,
    RENDER,
    EDITOR,
};

struct ProgramOptions
{
    ApplicationMode mode = ApplicationMode::UNDEFINED;
    std::vector<std::string> sceneFiles;
    bool enableValidationLayer = false;
};

std::optional<ProgramOptions> ParseCommandLine(int argc, char const* argv[])
{
    using namespace boost::program_options;

    ProgramOptions result = {};

    options_description genericOptions{"Generic options"};
    genericOptions.add_options()
        ("help,h", "Help screen")
        ("version,v", "Print version")
        ;
    options_description configOptions{"Application modes"};
    configOptions.add_options()
        ("render,r", bool_switch()->default_value(false)->notifier([&](bool value)
            {
                if (value)
                {
                    result.mode = ApplicationMode::RENDER;
                }
            }), "Render input file")
        ("test,t", bool_switch()->default_value(false)->notifier([&](bool value)
            {
                if (value)
                {
                    result.mode = ApplicationMode::TESTING;
                }
            }), "Run all tests")
        ("editor,e", bool_switch()->default_value(false)->notifier([&](bool value)
            {
                if (value)
                {
                    result.mode = ApplicationMode::EDITOR;
                }
            }), "Run the application as an editor");
        ;
        
    options_description editorOptions{"Editor options"};
    editorOptions.add_options()
        ("enable-validation-layers",
#if DEBUG
         value<bool>()->default_value(true),
#else
         value<bool>()->default_value(false),
#endif
         "Enables the validation layer for the vulkan renderer")
        ;


    options_description rendererOptions{"Renderer options"};
    rendererOptions.add_options()
        ("scenes", value<std::vector<std::string>>(&result.sceneFiles), "Scene files for the renderer")
        ;

    options_description visibleOptions;
    visibleOptions.add(genericOptions).add(configOptions).add(rendererOptions).add(editorOptions);
    
    positional_options_description inputFiles;
    inputFiles.add("scenes", -1);

    try
    {
        variables_map vm;
        store(command_line_parser(argc, argv).options(visibleOptions).positional(inputFiles).run(), vm);
        notify(vm);

        if (vm.count("help") || result.mode == ApplicationMode::UNDEFINED)
        {
            std::cout << visibleOptions << std::endl;
            return std::nullopt;
        }
        else if (vm.count("version"))
        {
            std::cout << "Renderer version: " << APPLICATION_VERSION << std::endl;
            return std::nullopt;
        }
        else if (vm.count("enable-validation-layers"))
        {
            result.enableValidationLayer = vm["enable-validation-layers"].as<bool>();
        }
    }
    catch (std::exception const& e)
    {
        LOG(ERROR) << e.what() << std::endl;
    }

    return result;
}

Common::SceneParser::ParsedScene CreateDefaultScene()
{
    Common::SceneParser::ParsedScene scene = {};
    {
        /* Renderer info */
        scene.rendererInfo.maxDepth = 10;
        scene.rendererInfo.numSamples = 100;
        scene.rendererInfo.rendererType = CreateInfo::RayTracingType::SimpleRayTracing;
    }
    {
        /* Default materials */
        CreateInfo::Material defaultMaterial = {};
        {
            defaultMaterial.name = "Green";
            defaultMaterial.attenuation = Jnrlib::Green;
            defaultMaterial.type = CreateInfo::MaterialType::Lambertian;
            defaultMaterial.mask |= CreateInfo::Material::Attenuation;
        }
        scene.materialsInfo.push_back(defaultMaterial);
    }
    {
        scene.sceneInfo.alsoBuildForRealTimeRendering = true;
        scene.sceneInfo.imageInfo.width = 1024;
        scene.sceneInfo.imageInfo.height = 1024;
        scene.sceneInfo.outputFile = "output.png";
        scene.sceneInfo.cameraInfo.position = Jnrlib::Position(0.0f, 5.0f, 0.0f);
        scene.sceneInfo.cameraInfo.focalDistance = 1.0f;
        scene.sceneInfo.cameraInfo.fieldOfView = 0.9f;
        scene.sceneInfo.cameraInfo.RecalculateViewport((uint32_t)scene.sceneInfo.imageInfo.width, (uint32_t)scene.sceneInfo.imageInfo.height);

        CreateInfo::Primitive earthPrimitive = {};
        {
            earthPrimitive.materialName = "Green";
            earthPrimitive.name = "Earth";
            earthPrimitive.position = Jnrlib::Position(0.0f, -10000.f, 0.0f);
            earthPrimitive.radius = 10000.f;
            earthPrimitive.primitiveType = CreateInfo::PrimitiveType::Sphere;
        }

        scene.sceneInfo.primitives.push_back(earthPrimitive);
    }

    return scene;
}

int main(int argc, char const* argv[])
{
    FLAGS_logtostderr = true;
    FLAGS_logtostdout = false;
    FLAGS_log_dir = "Logs";
    FLAGS_v = 0;
    FLAGS_colorlogtostderr = true;
    google::InitGoogleLogging(argv[0]);

    auto options = ParseCommandLine(argc, argv);
    if (!options.has_value())
        return 0;

    if (options->mode == ApplicationMode::TESTING)
    {
        /* TODO: Make it possible to pass flags here ::testing::GTEST_FLAG(filter) = "*Pool*"; */
#ifdef BUILD_TESTS
        testing::InitGoogleTest();
        RUN_ALL_TESTS();
        return 0;
#else
        LOG(ERROR) << "Cannot run tests if tests were not built with the renderer" << std::endl;
        return 0;
#endif // BUILD_TESTS
    }
    else if (options->mode == ApplicationMode::RENDER)
    {
        using Common::SceneParser;
        LOG(INFO) << "Starting application in render mode";
        LOG(INFO) << "Running for config files: " << options->sceneFiles;

        std::vector<SceneParser::ParsedScene> parsedScenes;
        parsedScenes.reserve(options->sceneFiles.size());
        for (const auto& it : options->sceneFiles)
        {
            if (auto parsedScene = SceneParser::Get()->LoadSceneFromFile(it); parsedScene.has_value())
            {
                parsedScenes.emplace_back(std::move(*parsedScene));
                Common::MaterialManager::Get()->AddMaterials(parsedScene->materialsInfo);
            }
        }
        
        for (auto& parsedScene : parsedScenes)
        {
            auto s = std::make_unique<Common::Scene>(parsedScene.sceneInfo);
            RayTracing::RenderScene(s, parsedScene.rendererInfo);
        }
    }
    else if (options->mode == ApplicationMode::EDITOR)
    {
        using Common::SceneParser;
        LOG(INFO) << "Starting application in editor mode";
        std::vector<SceneParser::ParsedScene> parsedScenes;
        if (options->sceneFiles.size() > 0)
        {
            parsedScenes.reserve(options->sceneFiles.size());
            for (const auto &it : options->sceneFiles)
            {
                if (auto parsedScene = SceneParser::Get()->LoadSceneFromFile(it); parsedScene.has_value())
                {
                    Common::MaterialManager::Get()->AddMaterials(parsedScene->materialsInfo);
                    parsedScene->sceneInfo.alsoBuildForRealTimeRendering = true;
                    parsedScenes.emplace_back(std::move(*parsedScene));
                }
            }
        }
        else
        {
            SceneParser::ParsedScene scene = CreateDefaultScene();
            Common::MaterialManager::Get()->AddMaterials(scene.materialsInfo);
            parsedScenes.push_back(scene);
        }

        Editor::Editor::Get(options->enableValidationLayer, parsedScenes)->Run();
        Editor::Editor::Destroy();
    }
}
