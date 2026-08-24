using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public struct ShaderVariableDLL
    {
        public IntPtr Name { get; set; }
        public size_t Size { get; set; } = 0;
        public size_t ByteAlignment { get; set; } = 0;
        public ShaderMemberTypeEnum MemberTypeEnum { get; set; } = ShaderMemberTypeEnum.kShaderMember_Undefined;
        public bool ConstVariable { get; set; } = false;
        public ShaderVariableDLL()
        {
        }
    }
}
