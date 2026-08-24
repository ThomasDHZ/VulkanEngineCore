using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public class ShaderStructDLL
    {
        public string Name { get; set; } = string.Empty;
        public ShaderVariableDLL ShaderBufferVariableList { get; set; } = new ShaderVariableDLL();
        public size_t ShaderBufferVariableCount { get; set; }
    }
}
