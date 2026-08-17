using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    public struct MeshDrawMessage
    {
        public uint MeshId { get; set; } = uint.MaxValue;
        public uint Drawlayer { get; set; } = uint.MaxValue;
        public uint VertexBufferBinding { get; set; } = 0;
        public uint VertexCount { get; set; } = 0;
        public uint IndexCount { get; set; } = 0;           
        public uint InstanceCount { get; set; } = 1;
        public uint FirstVertex { get; set; } = 0;
        public uint FirstIndex { get; set; } = 0;
        public uint StartInstanceIndex { get; set; } = 0;
        public VkDeviceSize VertexOffset { get; set; } = 0;
        public VkDeviceSize InstanceOffset { get; set; } = 0;
        public VkBuffer VertexBuffer { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkBuffer IndexBuffer { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkBuffer InstanceBuffer { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public MeshDrawMessage()
        {
        }
    }
}
