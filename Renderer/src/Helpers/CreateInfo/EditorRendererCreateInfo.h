#pragma once 

#include <Jnrlib.h>

#include <nlohmann/json.hpp>


namespace CreateInfo
{

    struct EditorRenderer
    {
        std::vector<std::string> instanceLayers;
        std::vector<std::string> instanceExtensions;

        friend std::ostream& operator << (std::ostream& stream, EditorRenderer const& info);
        friend std::istream& operator >> (std::istream& stream, EditorRenderer& info);
    };

    void to_json(nlohmann::json& j, const EditorRenderer& p);
    void from_json(const nlohmann::json& j, EditorRenderer& p);
}