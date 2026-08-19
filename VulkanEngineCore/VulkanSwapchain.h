#pragma once

#include <Platform.h>

class VulkanSwapchain
{
	public:
		static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	private:
		VkExtent2D									  m_SwapChainResolution{};
		uint32										  m_ImageIndex = UINT32_MAX;
		uint32										  m_CommandIndex = UINT32_MAX;
		uint32										  m_SwapChainImageCount = UINT32_MAX;
		VkFormat									  m_SwapChainImageFormat;
		ivec2										  m_renderResolution;
		VkSwapchainKHR								  m_Swapchain = VK_NULL_HANDLE;
		Vector<VkFence>								  m_ImagesInFlight = Vector<VkFence>();
		Vector<VkImage>								  m_SwapChainImages = Vector<VkImage>();
		Vector<VkImageView>							  m_SwapChainImageViews = Vector<VkImageView>();
		Vector<VkSemaphore>							  m_PresentImageSemaphores = Vector<VkSemaphore>();
		std::array<VkFence, MAX_FRAMES_IN_FLIGHT>	  m_InFlightFences{};
		std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_AcquireImageSemaphores{};
		bool										  m_RebuildSwapChainFlag = false;

		void										  StartUpSwapChain();
		void										  StartUpSwapChainImages();
		VkSurfaceKHR								  StartUpVulkanSurface(void* windowHandle, VkInstance instance);
		void										  StartUpSwapChainImageViews();
		VkExtent2D									  StartUpSwapChainExtent();
		void										  StartUpSemaphores();
		VkSurfaceFormatKHR							  FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR							  FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
		void										  DestroySwapChainImageViews();
		void										  DestroySwapChain();

	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		 void										  Initialize(ivec2 renderResolution);
		 void										  RebuildSwapChain(void* windowHandle);
		 VkCommandBuffer							  StartFrame();
		 void										  EndFrame(VkCommandBuffer& commandBufferSubmit);
		 void										  Destroy();

		Vector<VkSurfaceFormatKHR>					  GetSurfaceFormats(VkPhysicalDevice physicalDevice);
		Vector<VkPresentModeKHR>					  GetSurfacePresentModes(VkPhysicalDevice physicalDevice);
		VkSurfaceCapabilitiesKHR					  GetSurfaceCapabilities(VkPhysicalDevice physicalDevice);
		void										  TriggerSwapChainFlag();

		[[nodiscard]] uint32						  ImageIndex()			 const;
		[[nodiscard]] uint32						  CommandIndex()		 const;
		[[nodiscard]] uint32						  SwapChainImageCount()  const;
		[[nodiscard]] VkFormat						  SwapChainImageFormat() const;
		[[nodiscard]] VkExtent2D					  SwapChainResolution()  const;
		[[nodiscard]] ivec2							  RenderPassResolution() const;
		const [[nodiscard]] Vector<VkImage>			  SwapChainImages()      const;
		const [[nodiscard]] Vector<VkImageView>		  SwapChainImageViews()  const;
};