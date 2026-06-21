#pragma once
#include <Platform.h>
#include <ConfigSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void							 ConfigSystem_SetRootDirectory(const String& engineRoot);
#ifdef __cplusplus
}
#endif