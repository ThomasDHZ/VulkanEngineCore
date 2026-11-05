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
    public struct ShaderLoader
    {
        public Guid ShaderId = new Guid();
        public string ShaderFile = string.Empty;
        public VkShaderStageFlagBits ShaderStage;

        public ShaderLoader()
        {
        }
    }
}
