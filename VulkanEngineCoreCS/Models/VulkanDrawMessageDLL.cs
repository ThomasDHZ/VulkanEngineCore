using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct VulkanDrawMessageDLL
    {
        public Guid RenderPassGuid { get; set; }
        public Guid PipelinePackageGuid { get; set; }
        public byte* PushConstant { get; set; }
        public PushConstantUpdateRule* PushConstantUpdateRules { get; set; }
        public MeshDrawMessage* DrawMeshList { get; set; }
        public Guid* RenderPassInputs { get; set; }
        public Guid* RenderPassOutputs { get; set; }
        public nuint PushConstantUpdateRulesCount { get; set; }
        public nuint DrawMeshListCount { get; set; }
        public nuint RenderPassInputsCount { get; set; }
        public nuint RenderPassOutputsCount { get; set; }
        public bool OffScreenRenderPass { get; set; }
        public IntPtr PushConstantsCmd { get; set; }
        public IntPtr PreDrawCmd { get; set; }
        public IntPtr CustomDrawCmd { get; set; }
        public IntPtr PostDrawCmd { get; set; }

        public static VulkanDrawMessageDLL ToDLL(in VulkanDrawMessage vulkanDrawMessage)
        {
            return new VulkanDrawMessageDLL
            {
                RenderPassGuid = vulkanDrawMessage.RenderPassGuid,
                PipelinePackageGuid = vulkanDrawMessage.PipelinePackageGuid,
                PushConstant = vulkanDrawMessage.PushConstant,
                PushConstantUpdateRules = vulkanDrawMessage.PushConstantUpdateRules.Ptr,
                DrawMeshList = vulkanDrawMessage.DrawMeshList.Ptr,
                RenderPassInputs = vulkanDrawMessage.RenderPassInputs.Ptr,
                RenderPassOutputs = vulkanDrawMessage.RenderPassOutputs.Ptr,
                PushConstantUpdateRulesCount = vulkanDrawMessage.PushConstantUpdateRules.Count,
                DrawMeshListCount = vulkanDrawMessage.DrawMeshList.Count,
                RenderPassInputsCount = vulkanDrawMessage.RenderPassInputs.Count,
                RenderPassOutputsCount = vulkanDrawMessage.RenderPassOutputs.Count,
                OffScreenRenderPass = vulkanDrawMessage.OffScreenRenderPass,
                PushConstantsCmd = vulkanDrawMessage.PushConstantsCmd,
                PreDrawCmd = vulkanDrawMessage.PreDrawCmd,
                CustomDrawCmd = vulkanDrawMessage.CustomDrawCmd,
                PostDrawCmd = vulkanDrawMessage.PostDrawCmd
            };
        }

        public static List<VulkanDrawMessageDLL> ToDLL(in List<VulkanDrawMessage> src)
        {
            List<VulkanDrawMessageDLL> vulkanDrawMessageList = new List<VulkanDrawMessageDLL>();
            foreach (var vulkanDrawMessage in src)
            {
                vulkanDrawMessageList.Add(VulkanDrawMessageDLL.ToDLL(vulkanDrawMessage));
            }
            return vulkanDrawMessageList;
        }
    }
}
