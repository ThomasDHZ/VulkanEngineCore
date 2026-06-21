#pragma once
#include "Platform.h"

class ConfigSystem
{
public:
	static ConfigSystem& Get();

private:
	ConfigSystem() = default;
	~ConfigSystem() = default;
	ConfigSystem(const ConfigSystem&) = delete;
	ConfigSystem& operator=(const ConfigSystem&) = delete;
	ConfigSystem(ConfigSystem&&) = delete;
	ConfigSystem& operator=(ConfigSystem&&) = delete;

public:

	DLL_EXPORT void SetRootDirectory(const String& engineRoot);
};
extern DLL_EXPORT ConfigSystem& configSystem;
inline ConfigSystem& ConfigSystem::Get()
{
	static ConfigSystem instance;
	return instance;
}

