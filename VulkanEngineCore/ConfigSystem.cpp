#include "ConfigSystem.h"

ConfigSystem& configSystem = ConfigSystem::Get();

void ConfigSystem::SetRootDirectory(const String& engineRoot)
{
#ifdef _WIN32
    if (_chdir(engineRoot.c_str()) != 0)
    {
        std::cerr << "Failed to set CWD to: " << engineRoot << std::endl;
        return;
    }

    char cwd[MAX_PATH];
    if (_getcwd(cwd, MAX_PATH))
    {
        std::cout << "C++ CWD SET TO: " << cwd << std::endl;
    }
#else
    if (chdir(engineRoot.c_str()) != 0)
    {
        std::cerr << "Failed to set CWD to: " << engineRoot << std::endl;
        return;
    }

    char cwd[PATH_MAX] = {};
    if (getcwd(cwd, PATH_MAX))
#endif
    {
        std::cout << "C++ CWD SET TO: " << cwd << std::endl;
    }
}
