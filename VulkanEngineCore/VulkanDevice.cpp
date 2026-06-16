#include "VulkanDevice.h"
#include "VulkanCoreSystem.h"
#include "VkSwapChain"

VulkanDevice::VulkanDevice()
{
}

VulkanDevice::~VulkanDevice()
{
}

bool VulkanDevice::Initialize(VkInstance instance, VkSurfaceKHR surface)
{
}

void VulkanDevice::SetUpLogicalDevice(VkSurfaceKHR surface)
{
    float queuePriority = 1.0f;
    Vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
    if (GraphicsFamily != UINT32_MAX)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = GraphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }
    if (PresentFamily != UINT32_MAX &&
        PresentFamily != GraphicsFamily)
    {
        queueCreateInfoList.emplace_back(VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = PresentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
            });
    }

    Vector<const char*> DeviceExtensionList = GetRequiredDeviceExtensions(PhysicalDevice);
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT,
        .pNext = &indexingFeatures,
        .colorWriteEnable = VK_TRUE
    };


    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &colorWriteFeatures,
        .features =
        {
            .geometryShader = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_TRUE,
            .fragmentStoresAndAtomics = VK_TRUE,
            .shaderFloat64 = VK_TRUE,
            .shaderInt64 = VK_TRUE,
            .shaderInt16 = VK_TRUE,
        },
    };

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &physicalDeviceFeatures2,
        .storageBuffer8BitAccess = VK_TRUE,
        .uniformAndStorageBuffer8BitAccess = VK_TRUE,
        .descriptorIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .scalarBlockLayout = VK_TRUE,
        .separateDepthStencilLayouts = VK_TRUE,
        .timelineSemaphore = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
        .vulkanMemoryModel = VK_TRUE,
        .vulkanMemoryModelDeviceScope = VK_TRUE
    };

    VkPhysicalDeviceRobustness2FeaturesEXT physicalDeviceRobustness2Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan12Features,
        .nullDescriptor = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features physicalDeviceVulkan13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &physicalDeviceRobustness2Features,
        .shaderDemoteToHelperInvocation = VK_TRUE,
    };

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT divisorFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT,
        .pNext = &physicalDeviceVulkan13Features,
        .vertexAttributeInstanceRateDivisor = VK_TRUE
    };

    VkPhysicalDeviceVulkan11Features physicalDeviceVulkan11Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &divisorFeatures,
        .multiview = VK_TRUE
    };

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
        .pNext = &physicalDeviceVulkan11Features,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &bufferDeviceAddressFeatures,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfoList.size()),
        .pQueueCreateInfos = queueCreateInfoList.data(),
        .enabledExtensionCount = static_cast<uint32>(DeviceExtensionList.size()),
        .ppEnabledExtensionNames = DeviceExtensionList.data(),
        .pEnabledFeatures = nullptr
    };

#ifndef NDEBUG
    Vector<const char*> validationLayers = GetValidationLayerProperties();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

    VULKAN_THROW_IF_FAIL(vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &LogicalDevice));
}

VkPhysicalDevice VulkanDevice::SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    Vector<VkPhysicalDevice> physicalDeviceList = GetPhysicalDeviceList(instance);
    for (auto& physicalDevice : physicalDeviceList)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties = GetPhysicalDeviceProperties(physicalDevice);
        VkPhysicalDeviceFeatures physicalDeviceFeatures = GetPhysicalDeviceFeatures(physicalDevice);
        GetQueueFamilies(physicalDevice, surface, graphicsFamily, presentFamily);
        Vector<VkSurfaceFormatKHR> surfaceFormatList = GetSurfaceFormats(physicalDevice, surface);
        Vector<VkPresentModeKHR> presentModeList = GetSurfacePresentModes(physicalDevice, surface);

        if (graphicsFamily != UINT32_MAX &&
            presentFamily != UINT32_MAX &&
            surfaceFormatList.size() > 0 &&
            presentModeList.size() > 0 &&
            physicalDeviceFeatures.samplerAnisotropy)
        {
            return PhysicalDevice;
        }
    }

    return VK_NULL_HANDLE;
}

void VulkanDevice::GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily)
{
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    Vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, families.data());

    for (uint32 x = 0; x < queueFamilyCount; x++)
    {
        if (families[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = x;

            VkBool32 presentSupport = VK_FALSE;
            VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, x, surface, &presentSupport));
            if (presentSupport) presentFamily = x;
            if (graphicsFamily != UINT32_MAX) break;
        }
    }
}

void VulkanDevice::GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue& graphicsQueue, VkQueue& presentQueue)
{
    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX) 
    {
        fprintf(stderr, "ERROR: Invalid queue family index!\n");
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);

    if (graphicsQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: graphicsQueue is NULL! Family: %u (index 0 invalid?)\n", graphicsFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }
    if (presentQueue == VK_NULL_HANDLE) {
        fprintf(stderr, "FATAL: presentQueue is NULL! Family: %u (index 0 invalid?)\n", presentFamily);
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }
    printf("SUCCESS: GraphicsQueue = %p (family %u), PresentQueue = %p (family %u)\n", (void*)graphicsQueue, graphicsFamily, (void*)presentQueue, presentFamily);
}

VkSampleCountFlagBits VulkanDevice::GetMaxSampleCount(VkPhysicalDevice gpuDevice)
{
    VkPhysicalDeviceLimits physicalDeviceProperties = GetPhysicalDeviceProperties(PhysicalDevice).limits;
    VkSampleCountFlags counts = physicalDeviceProperties.framebufferColorSampleCounts & physicalDeviceProperties.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

Vector<const char*> VulkanDevice::GetRequiredInstanceExtensions()
{
    uint32 count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    Vector<VkExtensionProperties> availableExtensionList(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensionList.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& extension : availableExtensionList)
                if (strcmp(extension.extensionName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            std::cout << "Extension not supported: " << ext << '\n';
        };

    AddExtensionIfSupported(VK_KHR_SURFACE_EXTENSION_NAME);
    AddExtensionIfSupported(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
#if defined(_WIN32)
    AddExtensionIfSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__) && !defined(__ANDROID__)
    AddExtensionIfSupported("VK_KHR_xcb_surface");
    AddExtensionIfSupported("VK_KHR_wayland_surface");
#elif defined(__ANDROID__)
    AddExtensionIfSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if !defined(NDEBUG) && !defined(__ANDROID__)
    AddExtensionIfSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    if (extensions.empty() ||
        (extensions.size() == 1 && extensions[0] == VK_KHR_SURFACE_EXTENSION_NAME))
    {
        throw std::runtime_error("No platform surface extension available — cannot create window.");
    }
    return extensions;
}

Vector<const char*> VulkanDevice::GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32 count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, availableDeviceExtensions.data());

    std::vector<const char*> enabledDeviceExtensions;
    auto AddDeviceExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& deviceExtensions : availableDeviceExtensions)
            {
                if (strcmp(deviceExtensions.extensionName, ext) == 0)
                {
                    enabledDeviceExtensions.push_back(ext);
                    std::cout << "[Device] Enabling: " << ext << '\n';
                    return true;
                }
            }
            std::cout << "[Device] Missing:   " << ext << '\n';
            return false;
        };

    if (!AddDeviceExtensionIfSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
    {
        throw std::runtime_error("FATAL: Swapchain extension not supported!");
    }
    AddDeviceExtensionIfSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    return enabledDeviceExtensions;
}

VkPhysicalDeviceProperties VulkanDevice::GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    return props;
}

VkPhysicalDeviceFeatures VulkanDevice::GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    return features;
}

VkPhysicalDeviceFeatures2 VulkanDevice::GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures2 features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    return features;
}

Vector<VkPhysicalDevice> VulkanDevice::GetPhysicalDeviceList(VkInstance& instance)
{
    uint32 deviceCount = 0;
    Vector<VkPhysicalDevice> physicalDeviceList = Vector<VkPhysicalDevice>();
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    physicalDeviceList = Vector<VkPhysicalDevice>(deviceCount);
    VULKAN_THROW_IF_FAIL(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data()));
    return physicalDeviceList;
}

Vector<VkSurfaceFormatKHR> VulkanDevice::GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, compatibleSwapChainFormatList.data()));
    return compatibleSwapChainFormatList;
}

Vector<VkPresentModeKHR> VulkanDevice::GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    Vector<VkPresentModeKHR> compatiblePresentModesList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, compatiblePresentModesList.data()));
    return compatiblePresentModesList;
}

Vector<const char*> VulkanDevice::GetValidationLayerProperties()
{
    uint32 layerCount = UINT32_MAX;
    Vector<const char*> validationLayers;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    Vector<const char*> extensions;
    auto AddExtensionIfSupported = [&](const char* ext)
        {
            for (const auto& layer : availableLayers)
            {
                if (strcmp(layer.layerName, ext) == 0)
                {
                    extensions.push_back(ext);
                    std::cout << "Enabling instance extension: " << ext << '\n';
                    return;
                }
            }
            std::cout << "Extension not supported: " << ext << '\n';
            //   __android_log_print(ANDROID_LOG_WARN, "Vulkan", "Validation layers not available - running without validation");
        };
    AddExtensionIfSupported("VK_LAYER_KHRONOS_validation");

    return extensions;
}

bool VulkanDevice::GetRayTracingCapability(VkPhysicalDevice gpuDevice, Vector<String>& featureList, Vector<const char*>& deviceExtensionList)
{
    VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures{};
    AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures{};
    RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    RayTracingPipelineFeatures.pNext = &AccelerationStructureFeatures;

    VkPhysicalDeviceFeatures2 DeviceFeatures2{};
    DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    DeviceFeatures2.pNext = &RayTracingPipelineFeatures;
    vkGetPhysicalDeviceFeatures2(gpuDevice, &DeviceFeatures2);

    if (RayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE &&
        AccelerationStructureFeatures.accelerationStructure == VK_TRUE)
    {
        if (std::find(featureList.begin(), featureList.end(), VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) != featureList.end() &&
            std::find(featureList.begin(), featureList.end(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) != featureList.end())
        {
            deviceExtensionList.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensionList.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            return true;
        }
        return false;
    }
    else
    {
        std::cout << "GPU/MotherBoard isn't ray tracing compatible." << std::endl;
        return false;
    }
    return false;
}

void VulkanDevice::Shutdown()
{
}
