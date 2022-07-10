#include "boost/program_options.hpp"
#include <iostream>
#include <optional>

#include "gtest/gtest.h"
#include "Jnrlib.h"

enum class ApplicationMode
{
    UNDEFINED = 0,
    TESTING
};

struct ProgramOptions
{
    ApplicationMode mode = ApplicationMode::UNDEFINED;
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
    options_description configOptions{"Options"};
    configOptions.add_options()
        ("run-tests,rt", value<bool>()->default_value(false)->notifier([&](bool value)
            {
                if (value)
                {
                    result.mode = ApplicationMode::TESTING;
                }
            }), "Run tests")
        ;

    options_description cmdlineOptions;
    cmdlineOptions.add(genericOptions).add(configOptions);
    
    try
    {
        variables_map vm;
        store(command_line_parser(argc, argv).options(cmdlineOptions).run(), vm);
        notify(vm);

        if (vm.count("help") || argc == 1)
        {
            std::cout << cmdlineOptions << std::endl;
        }
        else if (vm.count("version"))
        {
            std::cout << "Renderer version: " << APPLICATION_VERSION << std::endl;
        }
    }
    catch (std::exception const& e)
    {
        std::cout << e.what() << std::endl;
    }

    return result;
}

int main(int argc, char const* argv[])
{
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
        std::cout << "Cannot run tests if tests were not built with the renderer" << std::endl;
#endif // BUILD_TESTS
    }
}
