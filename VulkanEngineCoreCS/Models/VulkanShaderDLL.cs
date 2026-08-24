using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VulkanShaderDLL
    {
        public VkShaderModule ShaderModule { get; set; }
        public VkShaderStageFlagBits ShaderStages { get; set; }
        public ShaderPushConstantDLL PushConstant { get; set; }
        public VkVertexInputAttributeDescription* InputVertexAttributeList { get; set; }
        public VkVertexInputAttributeDescription* OutputVertexAttributeList { get; set; }
        public VkVertexInputBindingDescription* VertexInputBindingList { get; set; }
        public ShaderDescriptorBindingDLL* DescriptorBindingList { get; set; }
        public size_t InputVertexAttributeCount { get; set; }
        public size_t OutputVertexAttributeCount { get; set; }
        public size_t VertexInputBindingCount { get; set; }
        public size_t DescriptorBindingCount { get; set; }
    }
}
