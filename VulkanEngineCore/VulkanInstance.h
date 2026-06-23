#pragma once
#include "Platform.h"

class VulkanInstance
{
private:
    uint32				                   m_apiVersion = VK_API_VERSION_1_1;
    VkInstance			                   m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR		                   m_surface = VK_NULL_HANDLE;

    void					               SetUpVulkanSurface();

    uint32                                 GetMaxApiVersion(VkPhysicalDevice physicalDevice);
    Vector<const char*>					   GetRequiredInstanceExtensions();

public:
    VulkanInstance();
    ~VulkanInstance();

    void				                   SetUpVulkanInstance();
    void                                   Initialize();
    Vector<const char*>                    GetValidationLayerProperties();

    [[nodiscard]] uint32            ApiVersion()     const { return m_apiVersion; }
    [[nodiscard]] VkInstance        InstanceHandle() const { return m_instance; }
    [[nodiscard]] VkSurfaceKHR      Surface()        const { return m_surface; }
};

