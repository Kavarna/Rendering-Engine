#include "EditorRendererCreateInfo.h"

#include "TypeHelpers.h"

using json = nlohmann::json;

namespace CreateInfo
{
    std::ostream& operator<<(std::ostream& stream, EditorRenderer const& info)
    {
        json j;

        to_json(j, info);

        stream << j;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, EditorRenderer& info)
    {
        json j;

        stream >> j;
        from_json(j, info);

        return stream;
    }

    void to_json(nlohmann::json& j, const EditorRenderer& p)
    {
        j["instance-layers"] = p.instanceLayers;
        j["instance-extensions"] = p.instanceExtensions;
        j["device-layers"] = p.deviceLayers;
        j["device-extensions"] = p.deviceExtensions;
        if (p.window)
            j["window"] = "SPECIFIED";
        else
            j["window"] = "UNSPECIFIED";
    }

    void from_json(const nlohmann::json& j, EditorRenderer& p)
    {
        j["instance-layers"].get_to(p.instanceLayers);
        j["instance-extensions"].get_to(p.instanceExtensions);
        j["device-layers"].get_to(p.deviceLayers);
        j["device-extensions"].get_to(p.deviceExtensions);
        p.window = nullptr;
    }

}