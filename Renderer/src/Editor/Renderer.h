#pragma once

#include <Jnrlib.h>
#include "vulkan/vulkan.h"
#include "CreateInfoUtils.h"


namespace Editor
{
    class Renderer : public Jnrlib::ISingletone<Renderer>
    {
        MAKE_SINGLETONE_CAPABLE(Renderer);

    private:
        Renderer(CreateInfo::EditorRenderer const&);
        ~Renderer();
    
    private:
        void InitInstance(CreateInfo::EditorRenderer const& info);
        void InitDevice();

    private:
        struct ExtensionsOutput
        {
            template <typename ExtensionInfo>
            struct Extension
            {
                ExtensionInfo info;
                bool enabled = false;
            };
            std::vector<const char*> extensionNames = {};
            Extension<VkDebugUtilsMessengerCreateInfoEXT> debugUtils = {};
        };

        std::vector<const char*> GetEnabledInstanceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& layers);
        ExtensionsOutput HandleEnabledInstanceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& extensions);

    private:
        VkInstance mInstance;

        std::vector<const char*> mInstanceLayers;
        ExtensionsOutput mInstanceExtensions;

        VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
    };
}

