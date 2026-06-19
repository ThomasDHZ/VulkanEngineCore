//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//#include "VulkanSystem.h"
//#include <cstdlib>
//#include <iostream>
//#include "MemorySystem.h"
//#include "Platform.h"
//#include "BufferSystem.h"
//#if defined(__ANDROID__)
//#include <vulkan/vulkan_android.h>
//#endif
//
//VulkanSystem& vulkanSystem = VulkanSystem::Get();
////LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;
////
////void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
////{
////    g_logVulkanMessageCallback = callback;
////}
////
////void VulkanSystem_LogVulkanMessage(const char* message, int severity)
////{
////    if (g_logVulkanMessageCallback)
////    {
////        g_logVulkanMessageCallback(message, severity);
////    }
////}
//
////uint32 VulkanSystem::FindMaxApiVersion(VkPhysicalDevice physicalDevice)
////{
////    uint32 version = GetPhysicalDeviceProperties(physicalDevice).apiVersion;
////#ifndef __ANDROID__
////    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 4)) return VK_API_VERSION_1_4;
////#endif
////    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 3)) return VK_API_VERSION_1_3;
////    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 2)) return VK_API_VERSION_1_2;
////    if ((VK_VERSION_MAJOR(version) == 1 && VK_VERSION_MINOR(version) == 1)) return VK_API_VERSION_1_1;
////}
////
////void VulkanSystem::RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, ivec2& windowResolution, ivec2& renderResolution)
////{
////    DefaultRenderPassResolution = renderResolution;
////    WindowResolution = windowResolution;
////
////    vulkanSystem.ImageIndex = 0;
////    vulkanSystem.CommandIndex = 0;
////    vulkanSystem.RebuildRendererFlag = false;
////    vulkanSystem.Instance = instance;
////    vulkanSystem.Surface = surface;
////    //GetRayTracingCapability(vulkanSystem.PhysicalDevice, vulkanSystem.FeatureList, vulkanSystem.DeviceExtensionList);
////    vulkanSystem.PhysicalDevice = SetUpPhysicalDevice(vulkanSystem.Instance, vulkanSystem.Surface, vulkanSystem.GraphicsFamily, vulkanSystem.PresentFamily);
////    vulkanSystem.Device = SetUpDevice(vulkanSystem.PhysicalDevice, vulkanSystem.GraphicsFamily, vulkanSystem.PresentFamily);
////    bufferSystem.vmaAllocator = SetUpVmaAllocation();
////    vulkanSystem.MaxSampleCount = GetMaxSampleCount(vulkanSystem.PhysicalDevice);
////    SetUpSwapChain(windowHandle);
////    vulkanSystem.CommandPool = SetUpCommandPool(vulkanSystem.Device, vulkanSystem.GraphicsFamily);
////    SetUpCommandBuffers();
////    SetUpSemaphores();
////    GetDeviceQueue(vulkanSystem.Device, vulkanSystem.GraphicsFamily, vulkanSystem.PresentFamily, vulkanSystem.GraphicsQueue, vulkanSystem.PresentQueue);
////
////#if defined(__ANDROID__)
////    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(Device, "vkGetBufferDeviceAddress");
////    if (vkGetBufferDeviceAddress == nullptr) {
////        throw std::runtime_error("Failed to load vkGetBufferDeviceAddress function pointer!");
////    }
////#endif
////
////}
//
////VkExtent2D VulkanSystem::SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
////{
////#ifndef PLATFORM_ANDROID
////    int width;
////    int height;
////    glfwGetFramebufferSize((GLFWwindow*)windowHandle, &width, &height);
////
////    surfaceCapabilities = GetSurfaceCapabilities(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
////    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
////    {
////        return surfaceCapabilities.currentExtent;
////    }
////
////    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
////#else
////    VkExtent2D extent = {
////            surfaceCapabilities.currentExtent.width,
////            surfaceCapabilities.currentExtent.height
////    };
////    if (extent.width == UINT32_MAX) {
////        extent = { 1280, 720 };
////    }
////#endif
////    extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
////    extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
////    return extent;
////}
//
////void VulkanSystem::SetUpSwapChain(void* windowHandle)
////{
////    VkSurfaceCapabilitiesKHR surfaceCapabilities;
////    Vector<VkSurfaceFormatKHR> compatibleSwapChainFormatList = GetPhysicalDeviceFormats(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
////    VkExtent2D extent = SetUpSwapChainExtent(windowHandle, surfaceCapabilities);
////    GetQueueFamilies(vulkanSystem.PhysicalDevice, vulkanSystem.Surface, vulkanSystem.GraphicsFamily, vulkanSystem.PresentFamily);
////    Vector<VkPresentModeKHR> compatiblePresentModesList = GetPhysicalDevicePresentModes(vulkanSystem.PhysicalDevice, vulkanSystem.Surface);
////    VkSurfaceFormatKHR swapChainImageFormat = FindSwapSurfaceFormat(compatibleSwapChainFormatList);
////    VkPresentModeKHR swapChainPresentMode = FindSwapPresentMode(compatiblePresentModesList);
////
////    SetUpSwapChain();
////    SetUpSwapChainImages();
////    SetUpSwapChainImageViews(swapChainImageFormat);
////}
//
//VmaAllocator VulkanSystem::SetUpVmaAllocation()
//{
//    VmaVulkanFunctions vulkanFunctions = {
//        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
//        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
//    };
//
//    VmaAllocatorCreateInfo allocatorCreateInfo = {
//        .physicalDevice = PhysicalDevice,
//        .device = Device,
//        .preferredLargeHeapBlockSize = 64ull << 20,   // 64 MB
//        .pVulkanFunctions = &vulkanFunctions,
//        .instance = Instance,
//        .vulkanApiVersion = ApiVersion,
//    };
//
//    VmaAllocator allocator = VK_NULL_HANDLE;
//    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &allocator);
//
//    if (result != VK_SUCCESS)
//    {
//        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
//    }
//
//    return allocator;
//}
//
//void VulkanSystem::DestroyRenderer()
//{
//    DestroySwapChainImageView(vulkanSystem.Device, vulkanSystem.SwapChainImageViews);
//    DestroySwapChain(vulkanSystem.Device, &vulkanSystem.Swapchain);
//    DestroyFences(vulkanSystem.Device, vulkanSystem.AcquireImageSemaphores, vulkanSystem.PresentImageSemaphores, vulkanSystem.InFlightFences);
//    DestroyCommandPool(vulkanSystem.Device, &vulkanSystem.CommandPool);
//    vmaDestroyAllocator(bufferSystem.vmaAllocator);
//    DestroyDevice(vulkanSystem.Device);
//    DestroyDebugger(&vulkanSystem.Instance, vulkanSystem.DebugMessenger);
//    DestroySurface(vulkanSystem.Instance, &vulkanSystem.Surface);
//    DestroyInstance(&vulkanSystem.Instance);
//}
//
////uint32_t VulkanSystem::GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
////{
////    VkPhysicalDeviceMemoryProperties memProperties;
////    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
////
////    for (uint32_t x = 0; x < memProperties.memoryTypeCount; x++)
////    {
////        if ((typeFilter & (1 << x)) &&
////            (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
////        {
////            return x;
////        }
////    }
////
////    return UINT32_MAX;
////}
//
////void VulkanSystem::StartFrame()
////{
////    VkCommandBufferBeginInfo commandBufferBeginInfo
////    {
////        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
////        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
////    };
////    vulkanSystem.CommandIndex = (vulkanSystem.CommandIndex + 1) % vulkanSystem.SwapChainImageCount;
////
////    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkanSystem.Device, 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex], VK_TRUE, UINT64_MAX));
////    VULKAN_THROW_IF_FAIL(vkResetFences(vulkanSystem.Device, 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]));
////    VULKAN_THROW_IF_FAIL(vkResetCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], 0));
////    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], &commandBufferBeginInfo));
////    VkResult result = vkAcquireNextImageKHR(vulkanSystem.Device, vulkanSystem.Swapchain, UINT64_MAX, vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex], VK_NULL_HANDLE, &vulkanSystem.ImageIndex);
////    if (result == VK_ERROR_OUT_OF_DATE_KHR)
////    {
////        vulkanSystem.RebuildRendererFlag = true;
////    }
////    else if (result != VK_SUCCESS)
////    {
////        VULKAN_THROW_IF_FAIL(result);
////    }
////}
////
////void VulkanSystem::EndFrame(VkCommandBuffer& commandBufferSubmit)
////{
////    VkPipelineStageFlags waitStages[] =
////    {
////        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
////    };
////
////    VkSubmitInfo submitInfo =
////    {
////        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
////        .waitSemaphoreCount = 1,
////        .pWaitSemaphores = &vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex],
////        .pWaitDstStageMask = waitStages,
////        .commandBufferCount = 1,
////        .pCommandBuffers = &commandBufferSubmit,
////        .signalSemaphoreCount = 1,
////        .pSignalSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.CommandIndex]
////    };
////
////    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex]));
////    VkResult submitResult = vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]);
////    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
////        submitResult == VK_SUBOPTIMAL_KHR)
////    {
////        vulkanSystem.RebuildRendererFlag = true;
////        return;
////    }
////    else if (submitResult != VK_SUCCESS)
////    {
////        VULKAN_THROW_IF_FAIL(submitResult);
////    }
////
////    VkPresentInfoKHR presentInfo =
////    {
////        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
////        .waitSemaphoreCount = 1,
////        .pWaitSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.CommandIndex],
////        .swapchainCount = 1,
////        .pSwapchains = &vulkanSystem.Swapchain,
////        .pImageIndices = &vulkanSystem.ImageIndex
////    };
////
////    VkResult result = vkQueuePresentKHR(vulkanSystem.PresentQueue, &presentInfo);
////    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
////        result == VK_SUBOPTIMAL_KHR)
////    {
////        vulkanSystem.RebuildRendererFlag = true;
////        return;
////    }
////    else if (result != VK_SUCCESS)
////    {
////        VULKAN_THROW_IF_FAIL(result);
////    }
////}
//
//void VulkanSystem::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator)
//{
//    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//    if (func != NULL)
//    {
//        func(instance, debugUtilsMessengerEXT, pAllocator);
//    }
//    else
//    {
//        fprintf(stderr, "Failed to load vkDestroyDebugUtilsMessengerEXT function\n");
//    }
//}
//
//void VulkanSystem::DestroyFences(VkDevice device, Vector<VkSemaphore>& acquireImageSemaphores, Vector<VkSemaphore>& presentImageSemaphores, Vector<VkFence>& fences)
//{
//    for (auto& acquireImageSemaphore : acquireImageSemaphores)
//    {
//        if (acquireImageSemaphore != VK_NULL_HANDLE)
//        {
//            vkDestroySemaphore(device, acquireImageSemaphore, NULL);
//            acquireImageSemaphore = VK_NULL_HANDLE;
//        }
//    }
//    acquireImageSemaphores.clear();
//
//    for (auto& presentImageSemaphore : presentImageSemaphores)
//    {
//        if (presentImageSemaphore != VK_NULL_HANDLE)
//        {
//            vkDestroySemaphore(device, presentImageSemaphore, NULL);
//            presentImageSemaphore = VK_NULL_HANDLE;
//        }
//    }
//    presentImageSemaphores.clear();
//
//    for (auto& fence : fences)
//    {
//        if (fence != VK_NULL_HANDLE)
//        {
//            vkDestroyFence(device, fence, NULL);
//            fence = VK_NULL_HANDLE;
//        }
//    }
//    fences.clear();
//}
//
//void VulkanSystem::DestroyCommandPool(VkDevice device, VkCommandPool* commandPool)
//{
//    if (*commandPool != VK_NULL_HANDLE)
//    {
//        vkDestroyCommandPool(device, *commandPool, NULL);
//        *commandPool = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyDevice(VkDevice device)
//{
//    if (device != VK_NULL_HANDLE)
//    {
//        vkDestroyDevice(device, NULL);
//        device = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroySurface(VkInstance instance, VkSurfaceKHR* surface)
//{
//    if (*surface != VK_NULL_HANDLE)
//    {
//        vkDestroySurfaceKHR(instance, *surface, NULL);
//        *surface = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT)
//{
//    DestroyDebugUtilsMessengerEXT(*instance, debugUtilsMessengerEXT, NULL);
//}
//
//void VulkanSystem::DestroyInstance(VkInstance* instance)
//{
//    if (*instance != VK_NULL_HANDLE)
//    {
//        vkDestroyInstance(*instance, NULL);
//        *instance = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
//{
//    if (*renderPass != VK_NULL_HANDLE)
//    {
//        vkDestroyRenderPass(device, *renderPass, NULL);
//        *renderPass = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyFrameBuffers(VkDevice device, Vector<VkFramebuffer>& frameBufferList)
//{
//    for (auto& frameBuffer : frameBufferList)
//    {
//        if (frameBuffer != VK_NULL_HANDLE)
//        {
//            vkDestroyFramebuffer(device, frameBuffer, nullptr);
//            frameBuffer = VK_NULL_HANDLE;
//        }
//    }
//    frameBufferList.clear();
//}
//
//void VulkanSystem::DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
//{
//    if (*descriptorPool != VK_NULL_HANDLE)
//    {
//        vkDestroyDescriptorPool(device, *descriptorPool, NULL);
//        *descriptorPool = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout)
//{
//    if (*descriptorSetLayout != VK_NULL_HANDLE)
//    {
//        vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, NULL);
//        *descriptorSetLayout = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, Vector<VkCommandBuffer>& commandBufferList)
//{
//    vkFreeCommandBuffers(device, *commandPool, commandBufferList.size(), commandBufferList.data());
//    for (size_t x = 0; x < commandBufferList.size(); x++)
//    {
//        commandBufferList[x] = VK_NULL_HANDLE;
//    }
//    commandBufferList.clear();
//}
//
//void VulkanSystem::DestroyBuffer(VkDevice device, VkBuffer* buffer)
//{
//    if (*buffer != VK_NULL_HANDLE)
//    {
//        vkDestroyBuffer(device, *buffer, NULL);
//        *buffer = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory)
//{
//    if (*memory != VK_NULL_HANDLE)
//    {
//        vkFreeMemory(device, *memory, NULL);
//        *memory = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroySwapChainImageView(VkDevice device, Vector<VkImageView>& swapChainImageViewList)
//{
//    for (auto& swapChainImageView : swapChainImageViewList)
//    {
//        if (swapChainImageView != VK_NULL_HANDLE)
//        {
//            vkDestroyImageView(device, swapChainImageView, nullptr);
//            swapChainImageView = VK_NULL_HANDLE;
//        }
//    }
//    swapChainImageViewList.clear();
//}
//
//void VulkanSystem::DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain)
//{
//    vkDestroySwapchainKHR(device, *swapChain, NULL);
//    *swapChain = VK_NULL_HANDLE;
//}
//
//void VulkanSystem::DestroyImageView(VkDevice device, VkImageView* imageView)
//{
//    if (*imageView != VK_NULL_HANDLE)
//    {
//        vkDestroyImageView(device, *imageView, NULL);
//        *imageView = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyImage(VkDevice device, VkImage* image)
//{
//    if (*image != VK_NULL_HANDLE)
//    {
//        vkDestroyImage(device, *image, NULL);
//        *image = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroySampler(VkDevice device, VkSampler* sampler)
//{
//    if (*sampler != VK_NULL_HANDLE)
//    {
//        vkDestroySampler(device, *sampler, NULL);
//        *sampler = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyPipeline(VkDevice device, VkPipeline* pipeline)
//{
//    if (*pipeline != VK_NULL_HANDLE)
//    {
//        vkDestroyPipeline(device, *pipeline, NULL);
//        *pipeline = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout)
//{
//    if (*pipelineLayout != VK_NULL_HANDLE)
//    {
//        vkDestroyPipelineLayout(device, *pipelineLayout, NULL);
//        *pipelineLayout = VK_NULL_HANDLE;
//    }
//}
//
//void VulkanSystem::DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache)
//{
//    if (*pipelineCache != VK_NULL_HANDLE)
//    {
//        vkDestroyPipelineCache(device, *pipelineCache, NULL);
//        *pipelineCache = VK_NULL_HANDLE;
//    }
//}