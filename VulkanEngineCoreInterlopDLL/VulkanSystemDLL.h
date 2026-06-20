#pragma once
#include <VulkanSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void							 RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
	DLL_EXPORT uint32						 GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT void							 Shutdown();
#ifdef __cplusplus
}
#endif