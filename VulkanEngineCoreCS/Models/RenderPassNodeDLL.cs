using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct RenderPassNodeDLL
    {
        public Guid RenderPassGuid { get; set; }
        public VulkanDrawMessageDLL** SubPassDrawMessage { get; set; }
        public nuint SubPassDrawMessage_RenderPassCount { get; set; }
        public nuint* SubPassDrawMessage_SubPassCounts { get; set; }
        public IntPtr PreRenderPassCmd { get; set; }
        public IntPtr PostRenderPassCmd { get; set; }
        public uint MipCount { get; set; }
    }
}
