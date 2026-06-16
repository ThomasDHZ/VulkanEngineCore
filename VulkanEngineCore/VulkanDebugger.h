#pragma once
#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif
	typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
	DLL_EXPORT void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT void VulkanSystem_LogVulkanMessage(const char* message, int severity);
#ifdef __cplusplus
}
#endif

class VulkanDebugger
{
	static VkBool32 VKAPI_CALL			   DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* pUserData);
};

