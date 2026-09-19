#pragma once

#ifdef VulkanEngineCore_EXPORTS
#define IMGUI_API __declspec(dllexport)
#else
#define IMGUI_API __declspec(dllimport)
#endif