#pragma once
#include "Platform.h"

class VulkanSwapchain
{
private:
	uint32								   ImageIndex = UINT32_MAX;
	uint32								   CommandIndex = UINT32_MAX;
	uint32								   MaxFramesInFlight = UINT32_MAX;

	VkFormat							   Format = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR						   ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR					   PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkSampleCountFlagBits				   MaxSampleCount = VK_SAMPLE_COUNT_1_BIT;

	VkSurfaceKHR						   Surface = VK_NULL_HANDLE;
	VkSwapchainKHR						   Swapchain = VK_NULL_HANDLE;
	VkCommandPool						   CommandPool = VK_NULL_HANDLE;
	Vector<VkFence>						   InFlightFences = Vector<VkFence>();
	Vector<VkImage>						   SwapChainImages = Vector<VkImage>();
	Vector<VkCommandBuffer>				   CommandBuffers = Vector<VkCommandBuffer>();
	Vector<VkImageView>					   SwapChainImageViews = Vector<VkImageView>();
	Vector<VkSemaphore>					   AcquireImageSemaphores = Vector<VkSemaphore>();
	Vector<VkSemaphore>					   PresentImageSemaphores = Vector<VkSemaphore>();

	void								   SetUpSwapChainImages();
	VkSurfaceKHR						   SetUpVulkanSurface(void* windowHandle, VkInstance instance);
	void								   SetUpSwapChainImageViews(VkSurfaceFormatKHR swapChainImageFormat);
	VkExtent2D							   SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	void								   SetUpSemaphores();
	VkSurfaceFormatKHR					   FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR					   FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
	Vector<VkSurfaceFormatKHR>			   GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR>			   GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSurfaceCapabilitiesKHR			   GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

public:
	VkExtent2D							   SwapChainResolution{};
	uint32								   SwapChainImageCount = UINT32_MAX;
	ivec2								   DefaultRenderPassResolution;
	ivec2								   WindowResolution;

	DLL_EXPORT void						   SetUpSwapChain();
	DLL_EXPORT void						   SetUpSwapChain(void* windowHandle);
	DLL_EXPORT void						   RebuildSwapChain(void* windowHandle);
};


