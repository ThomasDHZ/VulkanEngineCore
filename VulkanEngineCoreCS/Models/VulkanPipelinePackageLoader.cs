using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;
using VulkanEngineCoreCS.Enum;

namespace VulkanEngineCoreCS.Models
{
    public struct VulkanPipelinePackageLoader
    {
        public string PipelinePackageName { get; set; } = string.Empty;
        public Guid PipelinePackageId { get; set; }
        public Dictionary<PipelineTypeEnum, Guid> PipelineMap { get; set; }

        public VulkanPipelinePackageLoader()
        {
        }
    }
}
