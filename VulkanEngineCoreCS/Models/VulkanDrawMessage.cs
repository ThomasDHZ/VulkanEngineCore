using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS.Vulkan;

namespace VulkanEngineCoreCS.Models
{
    public unsafe struct VulkanDrawMessage
    {

        public Guid RenderPassGuid { get; set; }
        public Guid PipelinePackageGuid { get; set; }
        public byte* PushConstant { get; set; }
        public ListPtr<PushConstantUpdateRule> PushConstantUpdateRules { get; set; } = new ListPtr<PushConstantUpdateRule>();
        public ListPtr<MeshDrawMessage> DrawMeshList { get; set; } = new ListPtr<MeshDrawMessage>();
        public ListPtr<Guid> RenderPassInputs { get; set; } = new ListPtr<Guid>();
        public ListPtr<Guid> RenderPassOutputs { get; set; } = new ListPtr<Guid>();
        public bool OffScreenRenderPass { get; set; } = false;
        public IntPtr PushConstantsCmd { get; set; }
        public IntPtr PreDrawCmd { get; set; }
        public IntPtr CustomDrawCmd { get; set; }
        public IntPtr PostDrawCmd { get; set; }
        public VulkanDrawMessage()
        {
        }

        public static VulkanDrawMessage FromDLL(in VulkanDrawMessageDLL src)
        {
            return new VulkanDrawMessage
            {
                RenderPassGuid = src.RenderPassGuid,
                PipelinePackageGuid = src.PipelinePackageGuid,
                PushConstant = src.PushConstant,
                PushConstantUpdateRules = new ListPtr<PushConstantUpdateRule>(src.PushConstantUpdateRules, src.PushConstantUpdateRulesCount),
                DrawMeshList = new ListPtr<MeshDrawMessage>(src.DrawMeshList, src.DrawMeshListCount),
                RenderPassInputs = new ListPtr<Guid>(src.RenderPassInputs, src.RenderPassInputsCount),
                RenderPassOutputs = new ListPtr<Guid>(src.RenderPassOutputs, src.RenderPassOutputsCount),
                OffScreenRenderPass = src.OffScreenRenderPass,
                PushConstantsCmd = src.PushConstantsCmd,
                PreDrawCmd = src.PreDrawCmd,
                CustomDrawCmd = src.CustomDrawCmd,
                PostDrawCmd = src.PostDrawCmd
            };
        }

        public static List<VulkanDrawMessage> FromDLL(IEnumerable<VulkanDrawMessageDLL> src)
        {
            var result = new List<VulkanDrawMessage>();
            foreach (var item in src)
            {
                result.Add(FromDLL(item));
            }
            return result;
        }

        public static unsafe List<VulkanDrawMessage> FromDLL(VulkanDrawMessageDLL* ptr, size_t count)
        {
            var result = new List<VulkanDrawMessage>((int)count);
            for (size_t x = 0; x < count; x++)
            {
                result.Add(FromDLL(in ptr[x]));
            }
            return result;
        }

        public static List<VulkanDrawMessage> FromDLL(ListPtr<VulkanDrawMessageDLL> src)
        {
            return FromDLL(src.Ptr, src.Count);
        }
    }
}
