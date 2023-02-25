#include "FileHelpers.h"
#include <fstream>

#include <glog/logging.h>

namespace Jnrlib
{
    std::vector<char> ReadWholeFile(std::string const& path)
    {
        std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
        CHECK(file.is_open()) << "Unable to open file " << path << " for reading";
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        CHECK(file.read(buffer.data(), size).good()) << "Unable to read file " << path << " after opening it";

        return buffer;
    }
    void DumpWholeFile(std::string const& path, std::vector<unsigned char> const& data)
    {
        std::ofstream file(path.c_str(), std::ios::binary);
        CHECK(file.is_open()) << "Unable to open file " << path << " for writing";

        file.write((const char*)data.data(), data.size());
    }
}
