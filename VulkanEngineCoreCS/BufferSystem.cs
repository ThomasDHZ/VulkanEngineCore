using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static VulkanEngineCoreCS.VulkanSystem;

namespace VulkanEngineCoreCS
{
    public unsafe class BufferSystem
    {
        public static void SetUpVmaAllocator()
        {
            DLLSystem.CallDLLFunc(() => BufferSystem_SetUpVmaAllocation());
        }
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void BufferSystem_SetUpVmaAllocation();
    }
}
