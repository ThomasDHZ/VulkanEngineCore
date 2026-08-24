using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct ShaderPushConstantDLL
    {
        public string PushConstantName { get; set; }
        public size_t PushConstantSize { get; set; } = 0;
        public VkShaderStageFlagBits ShaderStageFlags { get; set; }
        public ShaderVariableDLL* PushConstantVariableList { get; set; } = null;
        public bool GlobalPushConstant { get; set; } = false;
        public size_t PushConstantVariableCount { get; set; } = 0;
        public ShaderPushConstantDLL()
        {
        }

    }
}
