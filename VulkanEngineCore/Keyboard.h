#pragma once
#include "InputEnum.h"
#include "VulkanWindow.h"

#ifndef __ANDROID__
#include <GLFW/glfw3.h>
class DLL_EXPORT Keyboard
{
public:
    static Keyboard& Get();

private:
    Keyboard() = default;
    ~Keyboard() = default;
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;
    Keyboard(Keyboard&&) = delete;
    Keyboard& operator=(Keyboard&&) = delete;

public:

    KeyState KeyPressed[MAXKEYBOARDKEY];
    static void KeyboardKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
    const KeyState* GetKeyBoardState() const { return KeyPressed; }
     bool IsKeyDown(int key) const;
     bool IsKeyPressed(int key) const;
     bool IsKeyReleased(int key) const;
};
extern DLL_EXPORT Keyboard& keyboard;
inline Keyboard& Keyboard::Get()
{
    static Keyboard instance;
    return instance;
}
#endif