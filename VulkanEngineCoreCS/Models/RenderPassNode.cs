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

        public static RenderPassNode FromDLL(in RenderPassNodeDLL src)
        {
            return new RenderPassNode
            {
                RenderPassGuid = src.RenderPassGuid,
                MipCount = src.MipCount,
                PostRenderPassCmd = src.PostRenderPassCmd,
                PreRenderPassCmd = src.PreRenderPassCmd,
              //  SubPassDrawMessage = VulkanDrawMessage.FromDLL(src.SubPassDrawMessage),
            };
        }

        public static List<RenderPassNode> FromDLL(IEnumerable<RenderPassNodeDLL> src)
        {
            var result = new List<RenderPassNode>();
            foreach (var item in src)
            {
                result.Add(FromDLL(item));
            }
            return result;
        }

        public static unsafe List<RenderPassNode> FromDLL(RenderPassNodeDLL* ptr, size_t count)
        {
            var result = new List<RenderPassNode>((int)count);
            for (size_t x = 0; x < count; x++)
            {
                result.Add(FromDLL(in ptr[x]));
            }
            return result;
        }

        public static List<RenderPassNode> FromDLL(ListPtr<RenderPassNodeDLL> src)
        {
            return FromDLL(src.Ptr, src.Count);
        }
    }
}
