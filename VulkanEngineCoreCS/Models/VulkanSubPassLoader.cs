using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS.Enum;
using VulkanEngineCoreCS.Models.VulkanEngineCS;

namespace VulkanEngineCoreCS.Models
{
    public struct VulkanSubPassLoader
    {
        public Guid PipelinePackageId { get; set; } = Guid.Empty;
        public MeshTypeEnum MeshType { get; set; } = MeshTypeEnum.kMesh_Undefined;
        public DrawFlagBits RenderPassDrawFlags;
        public string? ShaderPushConstant { get; set; } = string.Empty;
        public List<PushConstantUpdateRule> PushConstantUpdates { get; set; } = new List<PushConstantUpdateRule>();
        public List<Guid> InputAttachmentList { get; set; } = new List<Guid>();
        public List<Guid> InputTextureList { get; set; } = new List<Guid>();
        public List<Guid> OutputTextureList { get; set; } = new List<Guid>();
        public String CustomDrawCommand { get; set; } = string.Empty;
        public CustomDrawInitContext CustomDrawCommandContext { get; set; } = new CustomDrawInitContext();
        public bool OffScreenRenderPass { get; set; } = false;
        public bool UseCustomDrawCommand { get; set; } = false;
        public bool AlphaRenderPass { get; set; } = false;

        public VulkanSubPassLoader()
        {
        }
    }
}
