using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static VulkanEngineCoreCS.VulkanSystem;

namespace VulkanEngineCoreCS
{
    public unsafe class ConfigSystem
    {
        public static void SetRootDirectory(string engineRoot)
        {
            DLLSystem.CallDLLFunc(() => ConfigSystem_SetRootDirectory(engineRoot));
        }

        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.Cdecl)] public static extern void ConfigSystem_SetRootDirectory([MarshalAs(UnmanagedType.LPStr)] string engineRoot);
    }
}
