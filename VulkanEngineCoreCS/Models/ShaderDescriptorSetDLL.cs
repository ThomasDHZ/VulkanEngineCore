using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe class ShaderDescriptorSetDLL
    {
        public string Name { get; set; } = string.Empty;
        public uint Binding { get; set; }
        public VkDescriptorType DescripterType { get; set; }
        public ShaderStructDLL* ShaderStructList { get; set; }
        public size_t ShaderStructCount { get; set; }
    }
}
