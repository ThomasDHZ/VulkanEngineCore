using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    using GlmSharp;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Text.Json;
    using System.Text.Json.Serialization;
    using System.Threading.Tasks;
    using VulkanEngineCoreCS.Models;

    namespace VulkanEngineCS
    {
        public enum PushConstantKind
        {
            ConstVariable = 0,
            Ref = 1
        }
        public enum TextureVarTypeEnum
        {
            kVar_TextureSize,
        };

        public struct PushConstantVariable
        {
            public object Value { get; set; }
            public ShaderMemberTypeEnum VariableType { get; set; }
        };

        public struct PushConstantRefrence
        {
            public Guid AssetId { get; set; }
            public TextureVarTypeEnum TextureVar { get; set; }
        };

        public struct PushConstantUpdateRule
        {
            public PushConstantKind PushConstantType { get; set; }
            public string VariableName { get; set; } = string.Empty;
            public PushConstantVariable PushConstantVariable { get; set; }
            public PushConstantRefrence PushConstantRef { get; set; }
            public PushConstantUpdateRule()
            {
            }
        }
    }
}
