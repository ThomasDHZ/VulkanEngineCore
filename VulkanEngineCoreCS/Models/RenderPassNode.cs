using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS.Vulkan;

namespace VulkanEngineCoreCS.Models
{
    public unsafe struct RenderPassNode
    {
        public Guid RenderPassGuid { get; set; }
        public List<List<VulkanDrawMessage>> SubPassDrawMessage {  get; set; }
        public IntPtr PreRenderPassCmd { get; set; }
        public IntPtr PostRenderPassCmd { get; set; }
        public uint MipCount { get; set; } = 0;
        public RenderPassNode()
        {

        }
    }
}
