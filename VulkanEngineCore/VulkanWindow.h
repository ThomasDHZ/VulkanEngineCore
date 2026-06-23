#pragma once
#include "Platform.h"
#include "Typedef.h"
#include <GLFW/glfw3.h>

class VulkanWindow
{
public:
    static VulkanWindow& Get();

    bool Create(const char* title, uint32 width, uint32 height);
    void CreateSurface(VkInstance& instance, VkSurfaceKHR& surface);
    void PollEvents();
    bool ShouldClose() const;
    void Close();

    void* GetHandle() const;
    ivec2 GetSize() const;
    ivec2 GetFramebufferSize() const;

    GLFWwindow* m_window = nullptr;
private:
    VulkanWindow() = default;
    ~VulkanWindow() = default;
    VulkanWindow(const VulkanWindow&) = delete;
    VulkanWindow& operator=(const VulkanWindow&) = delete;

    bool m_framebufferResized = false;
    uint32 m_width = 0;
    uint32 m_height = 0;

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void ErrorCallback(int error, const char* description);
};
extern DLL_EXPORT VulkanWindow& vulkanWindow;
inline VulkanWindow& VulkanWindow::Get()
{
    static VulkanWindow instance;
    return instance;
}