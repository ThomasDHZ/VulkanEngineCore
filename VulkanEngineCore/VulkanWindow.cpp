#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "VulkanWindow.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>

VulkanWindow& vulkanWindow = VulkanWindow::Get();

bool VulkanWindow::Create(const char* title, uint32 width, uint32 height)
{
    if (m_window) return true;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
    glfwSetErrorCallback(ErrorCallback);

    return true;
}

void VulkanWindow::CreateSurface(VkInstance& instance, VkSurfaceKHR& surface)
{
    GLFWwindow* handle = (GLFWwindow*)m_window;
    VkResult result = glfwCreateWindowSurface(instance, handle, nullptr, &surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan surface! Error code: %d\n", result);
        return;
    }
}

void VulkanWindow::PollEvents()
{
    glfwPollEvents();
}

bool VulkanWindow::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void VulkanWindow::Close()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void* VulkanWindow::GetWindowHandle() const
{
    return m_window;
}

HWND VulkanWindow::GetHWND() const
{
#ifdef _WIN32
    if (!m_window) return nullptr;

    HWND hwnd = glfwGetWin32Window(m_window);
    if (!hwnd) throw std::runtime_error("VulkanWindow::GetHWND: Win32 HWND is null");
    return hwnd;
#else
    return nullptr;
#endif
}

HWND VulkanWindow::GetHWND(GLFWwindow* window) const
{
#ifdef _WIN32
    if (!window) return nullptr;

    HWND hwnd = glfwGetWin32Window(window);
    if (!hwnd) throw std::runtime_error("VulkanWindow::GetHWND: Win32 HWND is null");
    return hwnd;
#else
    return nullptr;
#endif
}

ivec2 VulkanWindow::GetSize() const
{
    return { m_width, m_height };
}

ivec2 VulkanWindow::GetFramebufferSize() const
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    return { w, h };
}

void VulkanWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    VulkanWindow* self = (VulkanWindow*)glfwGetWindowUserPointer(window);
    if (self)
    {
        self->m_framebufferResized = true;
        self->m_width = width;
        self->m_height = height;
    }
}

void VulkanWindow::ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}