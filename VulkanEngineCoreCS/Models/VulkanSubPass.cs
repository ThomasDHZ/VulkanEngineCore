using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    public enum MeshTypeEnum
    {
        kMesh_None,
        kMesh_StaticMesh,
        kMesh_InstanceMesh,
        kMesh_Skinned,
        kMesh_Procedural,
        kMesh_FrameBuffer,
        kMesh_Undefined
    };

    public class VulkanSubPass
    {
        public Guid RenderPassGuid { get; set; }
        public Guid PipelinePackageGuid { get; set; }
        public MeshTypeEnum MeshType { get; set; } = MeshTypeEnum.kMesh_Undefined;
        public String? ShaderPushConstant { get; set; }
        public List<Guid> InputTextureList { get; set; } = new List<Guid>();
        public List<Guid> OutputTextureList { get; set; } = new List<Guid>();
        public bool OffScreenFrameBuffer { get; set; } = false;
    }
}
