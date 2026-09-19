#pragma once

#ifdef VulkanEngineCore_EXPORTS
#define VMA_CALL_PRE
#define VMA_CALL_POST __declspec(dllexport)
#else
#define VMA_CALL_PRE
#define VMA_CALL_POST __declspec(dllimport)
#endif