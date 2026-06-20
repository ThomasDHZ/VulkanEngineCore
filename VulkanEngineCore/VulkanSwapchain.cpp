#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanSystem.h"
#include <GLFW/glfw3.h>

void VulkanSwapchain::Initialize()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = GetSurfaceCapabilities(vulkan.PhysicalDevice());
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = vulkan.Device().GetPhysicalDeviceFormats(vulkan.PhysicalDevice());
    Vector<VkPresentModeKHR> compatiblePresentModesList = vulkan.Device().GetPhysicalDevicePresentModes(vulkan.PhysicalDevice());
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
        extent.width = std::clamp(static_cast<uint32>(m_WindowResolution.x), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32>(m_WindowResolution.y), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    m_SwapChainResolution = extent;
    m_SwapChainImageCount = imageCount;
    m_MaxFramesInFlight = m_SwapChainImageCount;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkan.Instance().Surface(),
        .minImageCount = m_SwapChainImageCount,
        .imageFormat = swapChainImageFormat.format,
        .imageColorSpace = swapChainImageFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = m_Swapchain
    };

    if (vulkan.Device().GraphicsFamily() != vulkan.Device().PresentFamily())
    {
        Vector<uint32> queueFamilyIndices =
        {
            vulkan.Device().GraphicsFamily(),
            vulkan.Device().PresentFamily()
        };

        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32>(queueFamilyIndices.size());
        SwapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VULKAN_THROW_IF_FAIL(vkCreateSwapchainKHR(vulkan.Device().LogicalDevice(), &SwapChainCreateInfo, nullptr, &m_Swapchain));
}

void VulkanSwapchain::Initialize(void* windowHandle)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = vulkan.Device().GetPhysicalDeviceFormats(vulkan.PhysicalDevice());
    VkExtent2D extent = SetUpSwapChainExtent(windowHandle, surfaceCapabilities);
    vulkan.Device().GetQueueFamilies(vulkan.Device().PhysicalDevice());
    Vector<VkPresentModeKHR> compatiblePresentModesList = vulkan.Device().GetPhysicalDevicePresentModes(vulkan.Device().PhysicalDevice());
    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);

    Initialize();
    SetUpSwapChainImages();
    SetUpSwapChainImageViews(swapChainImageFormat);
}

void VulkanSwapchain::StartFrame()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    m_CommandIndex = (m_CommandIndex + 1) % m_SwapChainImageCount;

    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkan.Device().LogicalDevice(), 1, &m_InFlightFences[m_CommandIndex], VK_TRUE, UINT64_MAX));
    VULKAN_THROW_IF_FAIL(vkResetFences(vulkan.Device().LogicalDevice(), 1, &m_InFlightFences[m_CommandIndex]));
    VULKAN_THROW_IF_FAIL(vkResetCommandBuffer(vulkan.CommandBuffer().CommandBuffers[m_CommandIndex], 0));
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(vulkan.CommandBuffer().CommandBuffers[m_CommandIndex], &commandBufferBeginInfo));
    VkResult result = vkAcquireNextImageKHR(vulkan.Device().LogicalDevice(), m_Swapchain, UINT64_MAX, m_AcquireImageSemaphores[m_CommandIndex], VK_NULL_HANDLE, &m_ImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vulkan.Swapchain().TriggerSwapChainFlag();
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
}

void VulkanSwapchain::EndFrame(VkCommandBuffer& commandBufferSubmit)
{
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_AcquireImageSemaphores[m_CommandIndex],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBufferSubmit,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_PresentImageSemaphores[m_CommandIndex]
    };

    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(vulkan.CommandBuffer().CommandBuffers[m_CommandIndex]));
    VkResult submitResult = vkQueueSubmit(vulkan.Device().m_graphicsQueue, 1, &submitInfo, m_InFlightFences[m_CommandIndex]);
    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
        submitResult == VK_SUBOPTIMAL_KHR)
    {
        vulkan.Swapchain().TriggerSwapChainFlag();
        return;
    }
    else if (submitResult != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(submitResult);
    }

    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_PresentImageSemaphores[m_CommandIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &m_ImageIndex
    };

    VkResult result = vkQueuePresentKHR(vulkan.Device().m_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        vulkan.Swapchain().TriggerSwapChainFlag();
        return;
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
}

void VulkanSwapchain::RebuildSwapChain(void* windowHandle)
{

}

void VulkanSwapchain::SetUpSwapChainImages()
{
    uint32 swapChainImageCount = UINT32_MAX;
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.Device().LogicalDevice(), m_Swapchain, &swapChainImageCount, nullptr));
    m_SwapChainImages.resize(swapChainImageCount);
    VULKAN_THROW_IF_FAIL(vkGetSwapchainImagesKHR(vulkan.Device().LogicalDevice(), m_Swapchain, &swapChainImageCount, m_SwapChainImages.data()));
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
    m_SwapChainImageViews.resize(m_SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < m_SwapChainImageCount; x++)
    {
        VkImageViewCreateInfo swapChainViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_SwapChainImages[x],
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
        VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkan.Device().LogicalDevice(), &swapChainViewInfo, nullptr, &m_SwapChainImageViews[x]));
    }
}

VkExtent2D VulkanSwapchain::SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
#ifndef PLATFORM_ANDROID
    int width;
    int height;
    glfwGetFramebufferSize((GLFWwindow*)windowHandle, &width, &height);

    surfaceCapabilities = GetSurfaceCapabilities(vulkan.Device().PhysicalDevice());
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


    m_InFlightFences.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
    m_AcquireImageSemaphores.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
    m_PresentImageSemaphores.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
    for (int x = 0; x < m_MaxFramesInFlight; x++)
    {
        VULKAN_THROW_IF_FAIL(vkCreateFence(vulkan.Device().LogicalDevice(), &fenceInfo, NULL, &m_InFlightFences[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.Device().LogicalDevice(), &semaphoreCreateInfo, NULL, &m_AcquireImageSemaphores[x]));
        VULKAN_THROW_IF_FAIL(vkCreateSemaphore(vulkan.Device().LogicalDevice(), &semaphoreCreateInfo, NULL, &m_PresentImageSemaphores[x]));
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

Vector<VkSurfaceFormatKHR> VulkanSwapchain::GetSurfaceFormats(VkPhysicalDevice physicalDevice)
{
    uint32 surfaceFormatCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR>  surfaceFormatList = Vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkan.Surface(), &surfaceFormatCount, surfaceFormatList.data()));
    return surfaceFormatList;
}

Vector<VkPresentModeKHR> VulkanSwapchain::GetSurfacePresentModes(VkPhysicalDevice physicalDevice)
{
    uint32_t presentModeCount = 0;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, NULL));
    Vector<VkPresentModeKHR> presentModeList = Vector<VkPresentModeKHR>(presentModeCount);
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkan.Surface(), &presentModeCount, presentModeList.data()));
    return presentModeList;
}

VkSurfaceCapabilitiesKHR VulkanSwapchain::GetSurfaceCapabilities(VkPhysicalDevice physicalDevice)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VULKAN_THROW_IF_FAIL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkan.Surface(), &surfaceCapabilities));
    return surfaceCapabilities;
}
