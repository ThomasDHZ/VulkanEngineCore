#include "VulkanCoreSystem.h"
#include "VulkanDebugger.h"

VulkanCoreSystem& vulkanCoreSystem = VulkanCoreSystem::Get();

VkInstance VulkanCoreSystem::CreateVulkanInstance()
{
#if defined(__linux__)
    unsetenv("VK_INSTANCE_LAYERS");
    unsetenv("VK_LAYER_PATH");
#endif

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    Vector<VkValidationFeatureEnableEXT> enabledList;
    Vector<VkValidationFeatureDisableEXT> disabledList;

#ifndef NDEBUG
    enabledList =
    {
        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
    };

    disabledList =
    {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
        VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT
    };

    debugInfo =
    {
       .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
       .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
       .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
       .pfnUserCallback = DebugCallBack
    };
#endif

    VkValidationFeaturesEXT validationFeatures = {
          .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
          .pNext = nullptr,
          .enabledValidationFeatureCount = static_cast<uint32_t>(enabledList.size()),
          .pEnabledValidationFeatures = enabledList.data(),
          .disabledValidationFeatureCount = static_cast<uint32_t>(disabledList.size()),
          .pDisabledValidationFeatures = disabledList.data()
    };

#ifndef NDEBUG
    validationFeatures.pNext = &debugInfo;
#endif

    Vector<const char*> extensionNames = GetRequiredInstanceExtensions();
    VkApplicationInfo applicationInfo =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        #if defined(__ANDROID__)
            .apiVersion = VK_API_VERSION_1_3
        #else
            .apiVersion = VK_API_VERSION_1_4
        #endif
    };


    Vector<const char*> validationLayers = GetValidationLayerProperties();
    VkInstanceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &validationFeatures,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensionNames.size()),
        .ppEnabledExtensionNames = extensionNames.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateInstance(&createInfo, nullptr, &instance));

#ifndef NDEBUG
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
    {
        func(instance, &debugInfo, nullptr, &vulkanSystem.DebugMessenger);
    }
#endif

    return instance;
}

Vector<const char*> VulkanCoreSystem::GetRequiredInstanceExtensions()
{
    return Vector<const char*>();
}

bool VulkanCoreSystem::Initialize(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
{
   
}

Vector<const char*> VulkanCoreSystem::GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice)
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

void VulkanCoreSystem::BeginFrame()
{
}

void VulkanCoreSystem::EndFrame(VkCommandBuffer commandBuffer)
{
}

void VulkanCoreSystem::Shutdown()
{
}
