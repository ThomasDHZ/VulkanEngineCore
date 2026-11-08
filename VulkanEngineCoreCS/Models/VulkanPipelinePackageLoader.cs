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
    public struct VulkanPipelinePackage
    {
        public PipelineTypeEnum PipelineType { get; set; }
        public Guid PipelineId { get; set; }
    };

    public struct VulkanPipelinePackageLoader
    {
        public string PipelinePackageName { get; set; } = string.Empty;
        public Guid PipelinePackageId { get; set; } = Guid.Empty;
        public List<VulkanPipelinePackage> PipelineList { get; set; }= new List<VulkanPipelinePackage>();

        public VulkanPipelinePackageLoader()
        {
        }
    }
}
