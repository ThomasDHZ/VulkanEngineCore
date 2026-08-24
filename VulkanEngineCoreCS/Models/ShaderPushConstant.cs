using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    public struct ShaderPushConstant
    {
        public string PushConstantName { get; set; } = string.Empty;
        public size_t PushConstantSize { get; set; }
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public List<ShaderVariable> PushConstantVariableList { get; set; } = new List<ShaderVariable>();
        public bool GlobalPushConstant { get; set; } = false;

        public ShaderPushConstant()
        {
        }
    };
}
