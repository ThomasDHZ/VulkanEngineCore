using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    public class VulkanShader
    {
        public VkShaderModule ShaderModule { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkShaderStageFlagBits ShaderStages { get; set; }
        public ShaderPushConstant PushConstant { get; set; }
        public List<VkVertexInputAttributeDescription> InputVertexAttributeList { get; set; } = new List<VkVertexInputAttributeDescription>();
        public List<VkVertexInputAttributeDescription> OutputVertexAttributeList { get; set; } = new List<VkVertexInputAttributeDescription> { };
        public List<VkVertexInputBindingDescription> VertexInputBindingList { get; set; } = new List<VkVertexInputBindingDescription> { };
        public List<ShaderDescriptorBinding> DescriptorBindingList { get; set; }
        // public List<SpvReflectSpecializationConstant> m_specializationConstantList { get; private set; }
    }
}
