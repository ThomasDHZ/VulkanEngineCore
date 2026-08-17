using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    public struct PushConstantUpdateRule
    {
        public string Variable { get; set; } = string.Empty;
        public string SourceId { get; set; } = string.Empty;
        public string Value { get; set; } = string.Empty;
        public bool ConstValue { get; set; } = false;
        public bool DirtyFlag { get; set; } = true;
        public PushConstantUpdateRule()
        {
        }
    }
}
