#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace Jnrlib
{
    std::vector<char> ReadWholeFile(std::string const& path, bool assertIfFail = true);
    void DumpWholeFile(std::string const& path, std::vector<unsigned char> const& data, bool assertIfFail = true);
}
