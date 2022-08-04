#include "boost/program_options.hpp"
#include <iostream>
#include <optional>

#ifdef BUILD_TESTS
#include "gtest/gtest.h"
#endif
#include "glog/logging.h"
#include "Jnrlib.h"

enum class ApplicationMode
{
    UNDEFINED = 0,
    TESTING,
    RENDER,
};

struct ProgramOptions
{
    ApplicationMode mode = ApplicationMode::UNDEFINED;
    std::vector<std::string> sceneFiles;
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
        ;

    options_description rendererOptions{"Renderer options"};
    rendererOptions.add_options()
        ("scenes", value<std::vector<std::string>>(&result.sceneFiles), "Scene files for the renderer")
        ;

    options_description visibleOptions;
    visibleOptions.add(genericOptions).add(configOptions).add(rendererOptions);
    
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
        }
        else if (vm.count("version"))
        {
            std::cout << "Renderer version: " << APPLICATION_VERSION << std::endl;
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
    FLAGS_logtostdout = true;
    FLAGS_v = 0;
    google::InitGoogleLogging(argv[0]);

    auto options = ParseCommandLine(argc, argv);
    if (!options.has_value())
        return 0;

    if (options->mode == ApplicationMode::TESTING)
    {
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
    }
}
