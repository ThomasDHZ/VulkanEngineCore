using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    public enum ShaderMemberTypeEnum
    {
        kShaderMember_Undefined,
        kShaderMember_Int,
        kShaderMember_Uint,
        kShaderMember_Float,
        kShaderMember_Ivec2,
        kShaderMember_Ivec3,
        kShaderMember_Ivec4,
        kShaderMember_Vec2,
        kShaderMember_Vec3,
        kShaderMember_Vec4,
        kShaderMember_Mat2,
        kShaderMember_Mat3,
        kShaderMember_Mat4,
        kShaderMember_bool
    };

    public struct ShaderVariable
    {

        public String Name { get; set; } = "ShaderVar";
        public ShaderMemberTypeEnum MemberTypeEnum { get; set; } = ShaderMemberTypeEnum.kShaderMember_Undefined;
        public object Value { get; set; }

        public ShaderVariable()
        {
        }
    }
}
