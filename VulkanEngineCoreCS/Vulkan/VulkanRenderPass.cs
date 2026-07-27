//using GlmSharp;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;
//using VulkanCS;
//using static VulkanEngineCoreCS.VulkanSystem;

//namespace VulkanEngineCoreCS.Vulkan
//{
//    public struct RenderPassLoader
//    {

//    }

//    public unsafe class VulkanRenderPass
//    {
//        private IntPtr _handle;
//        public Guid RenderPassId { get; private set; }
//        public ivec2 RenderPassResolution { get; private set; }
//        public ListPtr<VulkanTexture> AttachmentList { get; private set; }
//        public ListPtr<VulkanPipeline> PipelineList { get; private set; }
//        public ListPtr<ListPtr<VulkanSubPass>> SubPassList { get; private set; }
//        public VkSampleCountFlagBits SampleCount { get; private set; }
//        public VulkanRenderPass()
//        {
//            _handle = VulkanRenderPass_Create();
//        }

//        public void LoadRenderPass(RenderPassLoader loader)
//        {
//            DLLSystem.CallDLLFunc(() => VulkanRenderPass_LoadRenderPass(_handle, &loader));
//        } 

//        public void Dispose()
//        {
//            if (_handle != IntPtr.Zero)
//            {
//                VulkanDLL.VulkanRenderPass_Destroy(_handle);
//                _handle = IntPtr.Zero;
//            }
//        }

//        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr VulkanRenderPass_Create();
//        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void VulkanRenderPass_LoadRenderPass(IntPtr handle, RenderPassLoader* loader);
//    }
//}
