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
        void PickPhysicalDevice();
        void PickQueueFamilyIndices();
        void InitDevice(CreateInfo::EditorRenderer const& info);


    private:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;

            bool IsEmpty()
            {
                return !graphicsFamily.has_value();
            }

            std::unordered_set<uint32_t> GetUniqueFamilyIndices()
            {
                std::unordered_set<uint32_t> uniqueFamilyIndices;

                if (graphicsFamily.has_value())
                    uniqueFamilyIndices.insert(*graphicsFamily);

                return uniqueFamilyIndices;
            }
        };

        struct ExtensionsOutput
        {
            std::vector<const char*> extensionNames = {};
            std::optional<VkDebugUtilsMessengerCreateInfoEXT> debugUtils = {};
        };

    private:
        std::vector<const char*> GetEnabledInstanceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& layers);
        ExtensionsOutput HandleEnabledInstanceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& extensions);

        std::vector<const char*> GetEnabledDeviceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& layers);
        ExtensionsOutput HandleEnabledDeviceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& extensions);


    private:
        VkInstance mInstance;
        std::vector<const char*> mInstanceLayers;
        ExtensionsOutput mInstanceExtensions;
        VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
        
        VkPhysicalDevice mPhysicalDevice;
        VkPhysicalDeviceProperties mPhysicalDeviceProperties;
        VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
        QueueFamilyIndices mQueueIndices;

        VkDevice mDevice;
        std::vector<const char*> mDeviceLayers;
        ExtensionsOutput mDeviceExtensions;
        VkQueue mGraphicsQueue;

    };
}

