#include "FileHelpers.h"
#include <filesystem>
#include <fstream>

#include <glog/logging.h>

namespace Jnrlib
{
std::vector<char> ReadWholeFile(std::string const &given_path, bool assertIfFail)
{
    std::filesystem::path path(std::filesystem::absolute(given_path));
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (assertIfFail)
    {
        CHECK(file.is_open()) << "Unable to open file " << path << " for reading";
    }
    else
    {
        if (!file.is_open())
        {
            return {};
        }
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (assertIfFail)
    {
        CHECK(file.read(buffer.data(), size).good()) << "Unable to read file " << path << " after opening it";
    }
    else
    {
        if (!file.read(buffer.data(), size).good())
            return {};
    }

    return buffer;
}
void DumpWholeFile(std::string const &path, std::vector<unsigned char> const &data, bool assertIfFail)
{
    std::ofstream file(path.c_str(), std::ios::binary);
    if (assertIfFail)
    {
        CHECK(file.is_open()) << "Unable to open file " << path << " for writing";
    }

    file.write((const char *)data.data(), data.size());
}
} // namespace Jnrlib
