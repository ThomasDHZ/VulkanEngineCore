#pragma once
#include <stdlib.h>

#ifdef VulkanEngineCore_EXPORTS
#define CORE_DLL_EXPORT __declspec(dllexport)
#else
#define CORE_DLL_EXPORT __declspec(dllimport)
#endif