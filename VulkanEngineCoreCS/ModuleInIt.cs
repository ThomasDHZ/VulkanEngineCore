using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS
{
    public static class Module
    {
        public const String VulkanEngineInterop = "VulkanEngineInterop.dll";

        [ModuleInitializer]
        internal static void Initialize()
        {
            DLLSystem.Initialize();
        }
    }
}
