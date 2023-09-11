#include "VulkanRendererCreateInfo.h"

#include "TypeHelpers.h"
#include "nlohmann/detail/value_t.hpp"
#include "nlohmann/json_fwd.hpp"

using json = nlohmann::json;

namespace CreateInfo
{
std::ostream &operator<<(std::ostream &stream, VulkanRenderer const &info)
{
    json j;

    to_json(j, info);

    stream << j;
    return stream;
}

std::istream &operator>>(std::istream &stream, VulkanRenderer &info)
{
    json j;

    stream >> j;
    from_json(j, info);

    return stream;
}

void to_json(nlohmann::json &j, const VulkanRenderer &p)
{
    auto dump_to_json = [](std::vector<VulkanRenderer::LayerInfo> const &layersInfo) {
        nlohmann::json objects = nlohmann::json::array();
        for (auto const &layer : layersInfo)
        {
            objects.push_back({{"name", layer.name}, {"mandatory", layer.mandatory ? true : false}});
        }
        return objects;
    };
    j["instance-layers"] = dump_to_json(p.instanceLayers);
    j["instance-extensions"] = dump_to_json(p.instanceExtensions);
    j["device-layers"] = dump_to_json(p.deviceLayers);
    j["device-extensions"] = dump_to_json(p.deviceExtensions);
    if (p.window)
        j["window"] = "SPECIFIED";
    else
        j["window"] = "UNSPECIFIED";
}

void from_json(const nlohmann::json &j, VulkanRenderer &p)
{
    auto read_from_json = [](nlohmann::json const &j, std::vector<VulkanRenderer::LayerInfo> &layers) {
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            auto current_layer = (*it);
            std::string name;
            bool mandatory = true;
            if (current_layer.size() == 1)
            {
                name = j.at(0);
            }
            else
            {
                if (current_layer.contains("mandatory"))
                {
                    mandatory = current_layer["mandatory"];
                }
                if (current_layer.contains("name"))
                {
                    name = current_layer["name"];
                }
            }
            CHECK(!name.empty()) << "Name for a layer has to be non-empty";
            layers.emplace_back(name, mandatory);
        }
    };
    read_from_json(j["instance-layers"], p.instanceLayers);
    read_from_json(j["instance-extensions"], p.instanceExtensions);
    read_from_json(j["device-layers"], p.deviceLayers);
    read_from_json(j["device-extensions"], p.deviceExtensions);
    p.window = nullptr;
}
} // namespace CreateInfo
