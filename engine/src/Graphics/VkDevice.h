#pragma once

#include "VkUtil.h"
#include "Window/Window.h"

namespace Wraith
{
    class Device
    {
    public:
        Device(Window& window);
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;
        ~Device();

        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities{};
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;
        };

        struct QueueFamilyIndices
        {
            uint32_t graphics_family = 0;
            uint32_t present_family = 0;
            bool has_graphics = false;
            bool has_present = false;
            bool IsComplete() { return has_graphics && has_present; }
        };

        SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_PhysicalDevice); }
        u32 FindMemoryType(u32 type_filter, vk::MemoryPropertyFlags properties);
        QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_PhysicalDevice); }
        vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        void CreateImageWithInfo(const vk::ImageCreateInfo& image_info,
                                 vk::MemoryPropertyFlags properties,
                                 vk::UniqueImage& image,
                                 vk::UniqueDeviceMemory& image_memory);

        vk::PhysicalDeviceProperties& GetProperties() { return m_Properties; }
        vk::UniqueCommandPool& GetCommandPool() { return m_CommandPool; }
        vk::UniqueDevice& GetDevice() { return m_Device; }
        vk::UniqueSurfaceKHR& Surface() { return m_Surface; }
        vk::Queue& GraphicsQueue() { return m_GraphicsQueue; }
        vk::Queue& PresentQueue() { return m_PresentQueue; }

        bool GetValidationLayersEnabled()
        {
#ifdef NDEBUG
            return false;
#else
            return true;
#endif
        }

    private:
        void CreateInstance();
        void SetupDebugMessenger();
        void DestroyDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();

        bool CheckValidationLayerSupport();
        std::vector<const char*> GetValidationLayers();
        std::vector<const char*> GetRequiredExtensions();
        std::vector<const char*> GetDeviceExtensions();

        bool IsSuitableDevice(vk::PhysicalDevice device);
        bool IsPreferredDevice(vk::PhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device);
        void HasGLFWRequiredInstanceExtensions();
        bool CheckDeviceExtensionSupport(vk::PhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device);

        Window& m_Window;

        vk::UniqueInstance m_Instance;
        vk::DebugUtilsMessengerEXT m_DebugMessenger;
        vk::PhysicalDevice m_PhysicalDevice;
        vk::PhysicalDeviceProperties m_Properties;

        vk::UniqueDevice m_Device;
        vk::UniqueSurfaceKHR m_Surface;
        vk::Queue m_GraphicsQueue;
        vk::Queue m_PresentQueue;
        vk::UniqueCommandPool m_CommandPool;
    };
}  // namespace Wraith