#pragma once
#include "Platform.h"

class VulkanSwapchain
{
	private:
		VkExtent2D							   m_SwapChainResolution{};
		uint32								   m_ImageIndex = UINT32_MAX;
		uint32								   m_CommandIndex = UINT32_MAX;
		uint32								   m_MaxFramesInFlight = UINT32_MAX;
		uint32								   m_SwapChainImageCount = UINT32_MAX;
		ivec2								   m_DefaultRenderPassResolution;
		ivec2								   m_WindowResolution;

		VkFormat							   m_Format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR						   m_ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkPresentModeKHR					   m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkSampleCountFlagBits				   m_MaxSampleCount = VK_SAMPLE_COUNT_1_BIT;

		VkSwapchainKHR						   m_Swapchain = VK_NULL_HANDLE;
		VkCommandPool						   m_CommandPool = VK_NULL_HANDLE;
		Vector<VkFence>						   m_InFlightFences = Vector<VkFence>();
		Vector<VkImage>						   m_SwapChainImages = Vector<VkImage>();
		Vector<VkCommandBuffer>				   m_CommandBuffers = Vector<VkCommandBuffer>();
		Vector<VkImageView>					   m_SwapChainImageViews = Vector<VkImageView>();
		Vector<VkSemaphore>					   m_AcquireImageSemaphores = Vector<VkSemaphore>();
		Vector<VkSemaphore>					   m_PresentImageSemaphores = Vector<VkSemaphore>();
		bool								   m_RebuildSwapChainFlag = false;

		void								   SetUpSwapChainImages();
		VkSurfaceKHR						   SetUpVulkanSurface(void* windowHandle, VkInstance instance);
		void								   SetUpSwapChainImageViews(VkSurfaceFormatKHR swapChainImageFormat);
		VkExtent2D							   SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities);
		void								   SetUpSemaphores();
		VkSurfaceFormatKHR					   FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR					   FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);

	public:

		DLL_EXPORT void						   Initialize();
		DLL_EXPORT void						   Initialize(void* windowHandle);
		DLL_EXPORT void						   RebuildSwapChain(void* windowHandle);
		DLL_EXPORT void						   StartFrame();
		DLL_EXPORT void						   EndFrame(VkCommandBuffer& commandBufferSubmit);

		Vector<VkSurfaceFormatKHR>			   GetSurfaceFormats(VkPhysicalDevice physicalDevice);
		Vector<VkPresentModeKHR>			   GetSurfacePresentModes(VkPhysicalDevice physicalDevice);
		VkSurfaceCapabilitiesKHR			   GetSurfaceCapabilities(VkPhysicalDevice physicalDevice);
		void								   TriggerSwapChainFlag() { m_RebuildSwapChainFlag = true; }

		[[nodiscard]] uint32				   ImageIndex()			  const { return m_ImageIndex;					}
		[[nodiscard]] uint32				   CommandIndex()		  const { return m_CommandIndex;				}
		[[nodiscard]] ivec2					   WindowResolution()	  const { return m_WindowResolution;			}
		[[nodiscard]] uint32				   SwapChainImageCount()  const { return m_ImageIndex;					}
		[[nodiscard]] VkExtent2D			   SwapChainResolution()  const { return m_SwapChainResolution;			}
		[[nodiscard]] ivec2					   RenderPassResolution() const { return m_DefaultRenderPassResolution; }
};


