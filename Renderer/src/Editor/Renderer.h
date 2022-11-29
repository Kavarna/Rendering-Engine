#pragma once

#include <Jnrlib.h>
#include "vulkan/vulkan.h"
#include "CreateInfoUtils.h"
#include "VulkanHelpers/VulkanHelpers.h"


namespace Editor
{
    class Renderer : public Jnrlib::ISingletone<Renderer>
    {
        MAKE_SINGLETONE_CAPABLE(Renderer);

    private:
        Renderer(CreateInfo::EditorRenderer const&);
        ~Renderer();
    
    public:
        VkDevice GetDevice();

    private:
        void InitInstance(CreateInfo::EditorRenderer const& info);
        void PickPhysicalDevice();
        void PickQueueFamilyIndices();
        void InitDevice(CreateInfo::EditorRenderer const& info);
        void InitSurface();
        void InitSwapchain();

    private:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool IsEmpty()
            {
                return !graphicsFamily.has_value() && !presentFamily.has_value();
            }

            bool IsComplete()
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }

            std::unordered_set<uint32_t> GetUniqueFamilyIndices()
            {
                std::unordered_set<uint32_t> uniqueFamilyIndices;
                uniqueFamilyIndices.reserve(sizeof(*this) / sizeof(decltype(graphicsFamily)));

                if (graphicsFamily.has_value())
                    uniqueFamilyIndices.insert(*graphicsFamily);
                if (presentFamily.has_value())
                    uniqueFamilyIndices.insert(*presentFamily);

                return uniqueFamilyIndices;
            }
        };

        struct ExtensionsOutput
        {
            std::vector<const char*> extensionNames = {};
            std::optional<VkDebugUtilsMessengerCreateInfoEXT> debugUtils = {};
        };

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

    private:
        std::vector<const char*> GetEnabledInstanceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& layers);
        ExtensionsOutput HandleEnabledInstanceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& extensions);

        std::vector<const char*> GetEnabledDeviceLayers(decltype(CreateInfo::EditorRenderer::instanceLayers) const& layers);
        ExtensionsOutput HandleEnabledDeviceExtensions(decltype(CreateInfo::EditorRenderer::instanceExtensions) const& extensions);

        SwapchainSupportDetails GetSwapchainCapabilities();

        VkPresentModeKHR SelectBestPresentMode(std::vector<VkPresentModeKHR> const& presentModes);
        VkSurfaceFormatKHR SelectBestSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& formats);
        VkExtent2D SelectSwapchainExtent(VkSurfaceCapabilitiesKHR const& capabilities);

    private:
        GLFWwindow* mWindow;

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
        VkQueue mPresentQueue;

        VkSurfaceKHR mRenderingSurface;
        SwapchainSupportDetails mSwapchainDetails;
        VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
        std::vector<VkImage> mSwapchainImages;
        std::vector<VkImageView> mSwapchainImageViews;
    };
}

