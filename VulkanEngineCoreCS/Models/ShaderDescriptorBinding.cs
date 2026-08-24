using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    public enum ShaderDescriptorType
    {
        SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER = 0,
        SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
        SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
        SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
        SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
        SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
        SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
        SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
        SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
        SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
        SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
        SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000
    }

    public struct ShaderDescriptorBinding
    {
        public string Name { get; set; }
        public uint DescriptorSet { get; set; } = uint.MaxValue;
        public uint Binding { get; set; } = uint.MaxValue;
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public ShaderDescriptorType DescriptorBindingType { get; set; }
        public VkDescriptorType DescripterType { get; set; }
        public ShaderDescriptorBinding()
        {
        }
    }
}
