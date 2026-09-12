#pragma once
#include <stdlib.h>

#if defined(_WIN32)
#ifdef VULKAN_ENGINE_CORE
#define CORE_DLL_EXPORT __declspec(dllexport)
#else
#define CORE_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define CORE_DLL_EXPORT __attribute__((visibility("default")))
#endif