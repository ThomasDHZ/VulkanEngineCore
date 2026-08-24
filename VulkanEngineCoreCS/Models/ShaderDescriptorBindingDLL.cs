using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    public unsafe struct ShaderDescriptorBindingDLL
    {
        public IntPtr Name { get; set; }
        public uint DescriptorSet { get; set; } = uint.MaxValue;
        public uint Binding { get; set; } = uint.MaxValue;
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public ShaderDescriptorType DescriptorBindingType { get; set; }
        public VkDescriptorType DescripterType { get; set; }
        public ShaderDescriptorBindingDLL()
        {
        }
    }
}
