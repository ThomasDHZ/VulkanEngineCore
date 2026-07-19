#pragma once
#include <Platform.h>

#ifdef __cplusplus
extern "C" {
#endif
	typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
#ifdef __cplusplus
}
#endif

class DLL_EXPORT VulkanDebugger
{
	friend class VulkanInstance;
private:
	VkDebugUtilsMessengerEXT			   m_DebugMessenger = VK_NULL_HANDLE;

public:
	VulkanDebugger();
	~VulkanDebugger();
	static VkBool32 VKAPI_CALL DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* pUserData);
	static void LogVulkanMessage(const char* message, int severity);
	static void CreateLogMessageCallback(LogVulkanMessageCallback callback);

	VkDebugUtilsMessengerEXT* DebugMessengerHandle()			 { return &m_DebugMessenger; }
};

