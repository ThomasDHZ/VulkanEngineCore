#pragma once
#include "DLL.h"
#include <Platform.h>
#include "Typedef.h"
#include <GLFW/glfw3.h>

class VulkanWindow
{
private:
    VulkanWindow() = default;
    ~VulkanWindow() = default;
    VulkanWindow(const VulkanWindow&) = delete;
    VulkanWindow& operator=(const VulkanWindow&) = delete;

     uint32 m_GameControllerConnected = UINT32_MAX;
     uint32 m_width = 0;
     uint32 m_height = 0;

    static void ControllerConnectCallBack(int jid, int event);
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void ErrorCallback(int error, const char* description);

public:
    static VulkanWindow& Get();

    CORE_DLL_EXPORT  bool Create(const char* title, uint32 width, uint32 height);
    CORE_DLL_EXPORT  void CreateSurface(VkInstance& instance, VkSurfaceKHR& surface);
    CORE_DLL_EXPORT  void PollEvents();
    CORE_DLL_EXPORT   bool ShouldClose() const;
    CORE_DLL_EXPORT   void Close();

    CORE_DLL_EXPORT void* GetWindowHandle() const;
    CORE_DLL_EXPORT  HWND  GetHWND() const;
    CORE_DLL_EXPORT  HWND  GetHWND(GLFWwindow* window) const;
    CORE_DLL_EXPORT ivec2 GetSize() const;
    CORE_DLL_EXPORT  ivec2 GetFramebufferSize() const;

     GLFWwindow* m_window = nullptr;
};
CORE_DLL_EXPORT extern VulkanWindow& vulkanWindow;
inline VulkanWindow& VulkanWindow::Get()
{
    static VulkanWindow instance;
    return instance;
}