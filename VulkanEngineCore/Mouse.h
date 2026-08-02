#pragma once
#include "InputEnum.h"
#include "VulkanWindow.h"

#ifndef __ANDROID__
#include <GLFW/glfw3.h>

class DLL_EXPORT Mouse
{
public:
	static Mouse& Get();

private:
	Mouse() = default;
	~Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator=(const Mouse&) = delete;
	Mouse(Mouse&&) = delete;
	Mouse& operator=(Mouse&&) = delete;

public:

	float X;
	float Y;
	float XOffset;
	float YOffset;
	float XLast;
	float YLast;
	int WheelOffset;
	bool MouseButtonState[MAXMOUSEKEY];
	bool IsDragging = false;

	static void MouseMoveEvent(GLFWwindow* window, double Xoffset, double Yoffset);
	static void MouseButtonPressedEvent(GLFWwindow* window, int button, int action, int mods);
	static void MouseWheelEvent(GLFWwindow* window, double xpos, double ypos);
};
extern DLL_EXPORT Mouse& mouse;
inline Mouse& Mouse::Get()
{
	static Mouse instance;
	return instance;
}
#endif