#pragma once
#include "Platform.h"

class VulkanSwapchain
{
	private:
		VkExtent2D							   m_SwapChainResolution{};
		uint32								   m_ImageIndex = UINT32_MAX;
		uint32								   m_CommandIndex = UINT32_MAX;
		uint32								   m_SwapChainImageCount = UINT32_MAX;
		ivec2								   m_DefaultRenderPassResolution;
		VkSwapchainKHR						   m_Swapchain = VK_NULL_HANDLE;
		Vector<VkFence>						   m_InFlightFences = Vector<VkFence>();
		Vector<VkImage>						   m_SwapChainImages = Vector<VkImage>();
		Vector<VkImageView>					   m_SwapChainImageViews = Vector<VkImageView>();
		Vector<VkSemaphore>					   m_AcquireImageSemaphores = Vector<VkSemaphore>();
		Vector<VkSemaphore>					   m_PresentImageSemaphores = Vector<VkSemaphore>();
		bool								   m_RebuildSwapChainFlag = false;

		void								   StartUpSwapChain();
		void								   StartUpSwapChainImages();
		VkSurfaceKHR						   StartUpVulkanSurface(void* windowHandle, VkInstance instance);
		void								   StartUpSwapChainImageViews();
		VkExtent2D							   StartUpSwapChainExtent();
		void								   StartUpSemaphores();
		VkSurfaceFormatKHR					   FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR					   FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);

	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		DLL_EXPORT void						   Initialize(ivec2 defaultRenderPassResolution);
		DLL_EXPORT void						   RebuildSwapChain(void* windowHandle);
		DLL_EXPORT void						   StartFrame();
		DLL_EXPORT void						   EndFrame(VkCommandBuffer& commandBufferSubmit);

		Vector<VkSurfaceFormatKHR>			   GetSurfaceFormats(VkPhysicalDevice physicalDevice);
		Vector<VkPresentModeKHR>			   GetSurfacePresentModes(VkPhysicalDevice physicalDevice);
		VkSurfaceCapabilitiesKHR			   GetSurfaceCapabilities(VkPhysicalDevice physicalDevice);
		void								   TriggerSwapChainFlag() { m_RebuildSwapChainFlag = true; }

		[[nodiscard]] uint32				   ImageIndex()			  const { return m_ImageIndex;					}
		[[nodiscard]] uint32				   CommandIndex()		  const { return m_CommandIndex;				}
		[[nodiscard]] uint32				   SwapChainImageCount()  const { return m_SwapChainImageCount;			}
		[[nodiscard]] VkExtent2D			   SwapChainResolution()  const { return m_SwapChainResolution;			}
		[[nodiscard]] ivec2					   RenderPassResolution() const { return m_DefaultRenderPassResolution; }
};


