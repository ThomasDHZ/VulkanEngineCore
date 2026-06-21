using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS
{
    public unsafe class VulkanSystem
    {
        public static void CreateLogMessageCallback(LogVulkanMessageDelegate callback)
        {
            DLLSystem.CallDLLFunc(() => VulkanSystem_CreateLogMessageCallback(callback));
        }

        public static void RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
        {
            DLLSystem.CallDLLFunc(() => VulkanSystem_RendererSetUp(windowHandle, windowSize, renderSize));
        }

        public static void hutdown()
        {
            DLLSystem.CallDLLFunc(() => VulkanSystem_Shutdown());
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate void LogVulkanMessageDelegate(string message, int severity);
        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.Cdecl)] public static extern void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageDelegate callback);
        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.StdCall)] private static extern void VulkanSystem_RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.StdCall)] private static extern void VulkanSystem_Shutdown();
    }
}
