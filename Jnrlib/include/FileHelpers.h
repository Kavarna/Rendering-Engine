#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace Jnrlib
{
    std::vector<char> ReadWholeFile(std::string const& path);
}
