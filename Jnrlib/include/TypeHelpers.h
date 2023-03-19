#pragma once

#include <ostream>
#include <vector>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "effolkronium/random.hpp"

template <typename T>
std::ostream& operator<<(std::ostream& stream, std::vector<T> const& vct)
{
    if (vct.size() == 0)
    {
        stream << "[]";
    }
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

std::ostream& operator<< (std::ostream& stream, glm::vec2 const& v);
std::ostream& operator<< (std::ostream& stream, glm::dvec2 const& v);
std::ostream& operator<< (std::ostream& stream, glm::vec3 const& v);
std::ostream& operator<< (std::ostream& stream, glm::dvec3 const& v);
std::ostream& operator<< (std::ostream& stream, glm::vec4 const& v);
std::ostream& operator<< (std::ostream& stream, glm::dvec4 const& v);

glm::vec2 string_to_vec2(std::string const& str);
glm::vec3 string_to_vec3(std::string const& str);
glm::vec4 string_to_vec4(std::string const& str);
glm::vec2 string_to_dvec2(std::string const& str);
glm::vec3 string_to_dvec3(std::string const& str);
glm::vec4 string_to_dvec4(std::string const& str);

namespace Jnrlib
{
    template <typename T>
    T to_type(std::string const& str)
    {
        if constexpr (std::is_same_v<T, glm::vec2>)
        {
            return string_to_vec2(str);
        }
        else if constexpr (std::is_same_v<T, glm::vec3>)
        {
            return string_to_vec3(str);
        }
        else if constexpr (std::is_same_v<T, glm::vec4>)
        {
            return string_to_vec4(str);
        }
        else if constexpr (std::is_same_v<T, glm::dvec2>)
        {
            return string_to_dvec2(str);
        }
        else if constexpr (std::is_same_v<T, glm::dvec3>)
        {
            return string_to_dvec3(str);
        }
        else if constexpr (std::is_same_v<T, glm::dvec4>)
        {
            return string_to_dvec4(str);
        }
    }
}

namespace Jnrlib
{
    using Random = effolkronium::random_static;

#if USE_FLOAT32
    using Color = glm::vec4;
    using Position = glm::vec3;
    using Direction = glm::vec3;

    using Float = float;

#elif USE_FLOAT64
#pragma message("USE_FLOAT64 is not fully tested and might yield some bad results")
    using Color = glm::dvec4;
    using Position = glm::dvec3;
    using Direction = glm::dvec3;

    using Float = double;
#endif

    constexpr Float Zero = (Float)0;
    constexpr Float Quarter = (Float)1 / (Float)4;
    constexpr Float Half = (Float)1 / (Float)2;
    constexpr Float One = (Float)1;
    constexpr Float EPSILON = (Float)0.0001f;

    constexpr Float PI = glm::pi<Float>();

    constexpr Color Black = Color(Zero);
    constexpr Color Yellow = Color(One, One, Zero, One);
    constexpr Color Red = Color(One, Zero, Zero, One);
    constexpr Color Blue = Color(Zero, Zero, One, One);
    constexpr Color Green = Color(Zero, One, Zero, One);
    constexpr Color Cyan = Color(Zero, One, One, One);
}
