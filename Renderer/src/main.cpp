#include "boost/program_options.hpp"
#include <iostream>
#include <optional>

#ifdef BUILD_TESTS
#include "gtest/gtest.h"
#endif
#include "Jnrlib.h"
#include "Scene/SceneFactory.h"
#include "Renderer/Renderer.h"
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
        ("editor,e", bool_switch()->default_value(true)->notifier([&](bool value)
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

        if (vm.count("help"))
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
        LOG(INFO) << "Starting application in render mode";
        LOG(INFO) << "Running for config files: " << options->sceneFiles;

        std::vector<SceneFactory::ParsedScene> parsedScenes;
        parsedScenes.reserve(options->sceneFiles.size());
        for (const auto& it : options->sceneFiles)
        {
            if (auto parsedScene = SceneFactory::Get()->LoadSceneFromFile(it); parsedScene.has_value())
            {
                parsedScenes.emplace_back(std::move(*parsedScene));
            }
        }
        
        for (auto& parsedScene : parsedScenes)
        {
            Renderer::RenderScene(parsedScene.scene, parsedScene.rendererInfo);
        }
    }
    else if (options->mode == ApplicationMode::EDITOR)
    {
        LOG(INFO) << "Starting application in editor mode";
        std::vector<SceneFactory::ParsedScene> parsedScenes;
        parsedScenes.reserve(options->sceneFiles.size());
        for (const auto& it : options->sceneFiles)
        {
            if (auto parsedScene = SceneFactory::Get()->LoadSceneFromFile(it); parsedScene.has_value())
            {
                parsedScenes.emplace_back(std::move(*parsedScene));
            }
        }

        Editor::Editor::Get(options->enableValidationLayer)->Run();
        Editor::Editor::Destroy();
    }
}
