#pragma once
#include "Platform.h"
#include "TypeDef.h"

class VulkanDevice
{
    private:
        void                                   SetUpLogicalDevice(VkSurfaceKHR surface);
        VkPhysicalDevice                       SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily);

        VkSampleCountFlagBits				   GetMaxSampleCount(VkPhysicalDevice gpuDevice);
        Vector<const char*>					   GetRequiredInstanceExtensions();
        Vector<const char*>					   GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
        Vector<const char*>                    GetValidationLayerProperties();
        void                                   GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily);
        void								   GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue& graphicsQueue, VkQueue& presentQueue);
        bool								   GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList);

    public:
        VulkanDevice();
        ~VulkanDevice();

        VkPhysicalDevice                       PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                               LogicalDevice  = VK_NULL_HANDLE;
        VkQueue                                GraphicsQueue  = VK_NULL_HANDLE;
        VkQueue                                PresentQueue   = VK_NULL_HANDLE;

        uint32                                 GraphicsFamily = UINT32_MAX;
        uint32                                 PresentFamily  = UINT32_MAX;

        VkPhysicalDeviceFeatures               Features{};
        VkPhysicalDeviceFeatures2              Features2{};
        VkPhysicalDeviceVulkan12Features       Features12{};

        DLL_EXPORT bool                        Initialize(VkInstance instance, VkSurfaceKHR surface);
        DLL_EXPORT VkPhysicalDeviceProperties  GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);
        DLL_EXPORT VkPhysicalDeviceFeatures    GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
        DLL_EXPORT VkPhysicalDeviceFeatures2   GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice);
        DLL_EXPORT Vector<VkPhysicalDevice>    GetPhysicalDeviceList(VkInstance& instance);
        DLL_EXPORT Vector<VkSurfaceFormatKHR>  GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        DLL_EXPORT Vector<VkPresentModeKHR>    GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        DLL_EXPORT void                        Shutdown();
};