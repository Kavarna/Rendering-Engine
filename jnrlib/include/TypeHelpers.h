#pragma once

#include <ostream>
#include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& stream, std::vector<T> const& vct)
{
    stream << vct.size() << " : [ ";
    for (uint32_t i = 0; i < vct.size() - 1; ++i)
    {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, char*> || std::is_same_v<T, const char*>)
        {
            stream << "\"" << vct[i] << "\", ";
        }
        else
        {
            stream << vct[i] << ", ";
        }
    }
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, char*> || std::is_same_v<T, const char*>)
    {
        stream << "\"" << vct[vct.size() - 1] << "\" ]";
    }
    else
    {
        stream << vct[vct.size() - 1] << " ]";
    }

    return stream;
}

