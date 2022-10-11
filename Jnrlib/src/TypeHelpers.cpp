#include "TypeHelpers.h"
#include <string>
#include <sstream>

template <typename NumberType>
NumberType ParseNumber(std::istream& stream)
{
    char ch = stream.peek();
    while (!((ch >= '0' && ch <= '9') || ch == '-'))
    {
        stream.get();
        ch = stream.peek();
    }
    NumberType number{};
    stream >> number;
    return number;
}

std::ostream& operator<< (std::ostream& stream, glm::vec2 const& v)
{
    stream << "(" << v.x << ", " << v.y << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, glm::dvec2 const& v)
{
    stream << "(" << v.x << ", " << v.y << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, glm::vec3 const& v)
{
    stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, glm::dvec3 const& v)
{
    stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, glm::vec4 const& v)
{
    stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, glm::dvec4 const& v)
{
    stream << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return stream;
}

glm::vec2 string_to_vec2(std::string const& str)
{
    std::stringstream stream(str);
    float v1, v2;

    v1 = ParseNumber<float>(stream);
    v2 = ParseNumber<float>(stream);

    return glm::vec2(v1, v2);
}

glm::vec3 string_to_vec3(std::string const& str)
{
    std::stringstream stream(str);
    float v1, v2, v3;
    std::string separator;

    v1 = ParseNumber<float>(stream);
    v2 = ParseNumber<float>(stream);
    v3 = ParseNumber<float>(stream);

    return glm::vec3(v1, v2, v3);
}

glm::vec4 string_to_vec4(std::string const& str)
{
    std::stringstream stream(str);
    double v1, v2, v3, v4;
    std::string separator;

    v1 = ParseNumber<float>(stream);
    v2 = ParseNumber<float>(stream);
    v3 = ParseNumber<float>(stream);
    v4 = ParseNumber<float>(stream);

    return glm::vec4(v1, v2, v3, v4);
}

glm::vec2 string_to_dvec2(std::string const& str)
{
    std::stringstream stream(str);
    double v1, v2;
    std::string separator;

    v1 = ParseNumber<double>(stream);
    v2 = ParseNumber<double>(stream);

    return glm::vec2(v1, v2);
}

glm::vec3 string_to_dvec3(std::string const& str)
{
    std::stringstream stream(str);
    double v1, v2, v3;
    std::string separator;

    v1 = ParseNumber<double>(stream);
    v2 = ParseNumber<double>(stream);
    v3 = ParseNumber<double>(stream);

    return glm::vec3(v1, v2, v3);
}

glm::vec4 string_to_dvec4(std::string const& str)

{
    std::stringstream stream(str);
    double v1, v2, v3, v4;
    std::string separator;

    v1 = ParseNumber<double>(stream);
    v2 = ParseNumber<double>(stream);
    v3 = ParseNumber<double>(stream);
    v4 = ParseNumber<double>(stream);

    return glm::vec4(v1, v2, v3, v4);
}