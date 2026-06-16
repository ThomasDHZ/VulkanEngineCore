#include "VulkanSwapchain.h"
#include "VulkanCoreSystem.h"
#include "VulkanDevice.h"

void VulkanSwapchain::SetUpSwapChain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities(vulkanCoreSystem.Device().PhysicalDevice, vulkanSystem.Surface);
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = vulkanCoreSystem.Device().GetPhysicalDeviceFormats(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
    Vector<VkPresentModeKHR> compatiblePresentModesList = vulkanCoreSystem.Device().GetPhysicalDevicePresentModes(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);

    uint32 imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0)
    {
        imageCount = std::min(imageCount, surfaceCapabilities.maxImageCount);
    }
    imageCount = std::max(imageCount, surfaceCapabilities.minImageCount);

    VkExtent2D extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width = std::clamp(static_cast<uint32>(WindowResolution.x), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32>(WindowResolution.y), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    SwapChainResolution = extent;
    SwapChainImageCount = imageCount;
    MaxFramesInFlight = SwapChainImageCount;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkanSystem.Surface,
        .minImageCount = static_cast<uint32>(vulkanSystem.SwapChainImageCount),
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = vulkanSystem.Swapchain
    };

    if (vulkanSystem.GraphicsFamily != vulkanSystem.PresentFamily)
    {
        Vector<uint32> queueFamilyIndices =
        {
            vulkanSystem.GraphicsFamily,
            vulkanSystem.PresentFamily
        };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_THROW_IF_FAIL(vkCreateSwapchainKHR(vulkanSystem.Device, &SwapChainCreateInfo, nullptr, &vulkanSystem.Swapchain));
}

void VulkanSwapchain::SetUpSwapChain(void* windowHandle)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = GetPhysicalDeviceFormats(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
    VkExtent2D extent = SetUpSwapChainExtent(windowHandle, surfaceCapabilities);
    GetQueueFamilies(vulkanSystem.PhysicalDevice, vulkanSystem.Surface, vulkanSystem.GraphicsFamily, vulkanSystem.PresentFamily);
    Vector<VkPresentModeKHR> compatiblePresentModesList = GetPhysicalDevicePresentModes(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);

    SetUpSwapChain();
    SetUpSwapChainImages();
    SetUpSwapChainImageViews(swapChainImageFormat);
}

void VulkanSwapchain::RebuildSwapChain(void* windowHandle)
{

}

void VulkanSwapchain::SetUpSwapChainImages()
{
    uint32 swapChainImageCount = UINT32_MAX;
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkanCoreSystem.Device(), Swapchain, &swapChainImageCount, nullptr));
    SwapChainImages.resize(swapChainImageCount);
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkanCoreSystem.Device(), Swapchain, &swapChainImageCount, SwapChainImages.data()));
}

VkSurfaceKHR VulkanSwapchain::SetUpVulkanSurface(void* windowHandle, VkInstance instance)
{
    if (!windowHandle || instance == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Invalid window handle (%p) or instance (%p)\n", windowHandle, (void*)instance);
        return VK_NULL_HANDLE;
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = (HWND)windowHandle;

    VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateWin32SurfaceKHR failed: %d (%s)\n",
            result,
            (result == VK_ERROR_EXTENSION_NOT_PRESENT) ? "VK_ERROR_EXTENSION_NOT_PRESENT" :
            (result == VK_ERROR_INITIALIZATION_FAILED) ? "VK_ERROR_INITIALIZATION_FAILED" :
            "unknown");
        return VK_NULL_HANDLE;
    }

#elif defined(__linux__) && !defined(__ANDROID__)
    GLFWwindow* window = (GLFWwindow*)windowHandle;
    glfwCreateWindowSurface(instance, window, nullptr, &surface);

#elif defined(__ANDROID__)
    VkAndroidSurfaceCreateInfoKHR surfaceInfo =
    {
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .window = (ANativeWindow*)windowHandle
    };

    if (vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Vulkan", "Failed to create Android surface!");
        return VK_NULL_HANDLE;
    }
#endif

    fprintf(stdout, "Surface created successfully: %p\n", (void*)surface);  // debug success
    return surface;
}

void VulkanSwapchain::SetUpSwapChainImageViews(VkSurfaceFormatKHR swapChainImageFormat)
{
    SwapChainImageViews.resize(SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < SwapChainImageCount; x++)
    {
        VkImageViewCreateInfo swapChainViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = SwapChainImages[x],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat.format,
            .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanCoreSystem.Device(), &swapChainViewInfo, nullptr, &SwapChainImageViews[x]));
    }
}

VkExtent2D VulkanSwapchain::SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
#ifndef PLATFORM_ANDROID
    int width;
    int height;
    glfwGetFramebufferSize((GLFWwindow*)windowHandle, &width, &height);

    surfaceCapabilities = GetSurfaceCapabilities(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return surfaceCapabilities.currentExtent;
    }

    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
#else
    VkExtent2D extent = {
            surfaceCapabilities.currentExtent.width,
            surfaceCapabilities.currentExtent.height
    };
    if (extent.width == UINT32_MAX) {
        extent = { 1280, 720 };
    }
#endif
    extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
    return extent;
}

void VulkanSwapchain::SetUpSemaphores()
{
    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
    .pNext = NULL,
    .semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
    .initialValue = 0,
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCreateInfo
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };


    InFlightFences.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    AcquireImageSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    PresentImageSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
    for (int x = 0; x < MaxFramesInFlight; x++)
    {
        VULKAN_THROW_IF_FAIL(vkCreateFence(Device, &fenceInfo, NULL, &InFlightFences[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(Device, &semaphoreCreateInfo, NULL, &AcquireImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(Device, &semaphoreCreateInfo, NULL, &PresentImageSemaphores[x]));
    }
}

VkSurfaceFormatKHR VulkanSwapchain::FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (uint32 x = 0; x < availableFormats.size(); x++)
    {
        if (availableFormats[x].format == VK_FORMAT_R8G8B8A8_UNORM &&
            availableFormats[x].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormats[x];
        }
    }
    fprintf(stderr, "Couldn't find a usable swap surface format.\n");
    return VkSurfaceFormatKHR{ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
}

VkPresentModeKHR VulkanSwapchain::FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
{
    for (uint32 x = 0; x < availablePresentModes.size(); x++)
    {
        if (availablePresentModes[x] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentModes[x];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

Vector<VkSurfaceFormatKHR> VulkanSwapchain::GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR>  surfaceFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormatList.data()));
    return surfaceFormatList;
}

Vector<VkPresentModeKHR> VulkanSwapchain::GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL));
    Vector<VkPresentModeKHR> presentModeList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModeList.data()));
    return presentModeList;
}

VkSurfaceCapabilitiesKHR VulkanSwapchain::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
    return surfaceCapabilities;
}
