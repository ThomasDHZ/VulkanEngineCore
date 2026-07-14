#include "VulkanRenderPass.h"
#include <VulkanPipeline.h>

VulkanRenderPass::VulkanRenderPass()
{
}

VulkanRenderPass::~VulkanRenderPass()
{
}

void VulkanRenderPass::LoadRenderPass(RenderPassLoader& renderPassLoader)
{
    RenderPassId = renderPassLoader.RenderPassId;
    RenderPassResolution = ivec2(INT32_MAX, INT32_MAX) == renderPassLoader.RenderPassResolution || ivec2(0) == renderPassLoader.RenderPassResolution ? vulkan.RenderPassResolution() : renderPassLoader.RenderPassResolution;
    RenderPass = VK_NULL_HANDLE;
    FrameBufferList = Vector<VkFramebuffer>();
    SubPassList = Vector<Vector<VulkanSubPass>>();
    ClearValueList = renderPassLoader.ClearValueList;
    SampleCount = renderPassLoader.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : renderPassLoader.SampleCount;
    UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView;
    IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass;

    BuildRenderPass(renderPassLoader);
    //for (auto& renderPass : renderPassLoader.SubPassList)
    //{
    //    Vector<VulkanSubPass> subPassList;
    //    for (auto& subPass : renderPass)
    //    {
    ////        //for (auto& pipeline : renderPassLoader.PipelineList)
    ////        //{
    ////        //    pipeline.RenderPassId = RenderPassId;
    ////        //    pipeline.RenderPass = RenderPass;
    ////        //    pipeline.RenderPassResolution = RenderPassResolution;
    ////        //    pipeline.UseGlobalBindlessSet = renderPassLoader.UseGlobalBindlessSet;

    ////        //    Vector<VkDescriptorImageInfo> descriptorSetInfoList;
    ////        //    for (auto& attachment : AttachmentList)
    ////        //    {
    ////        //        descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
    ////        //            {
    ////        //                .sampler = attachment.m_textureSampler,
    ////        //                .imageView = attachment.m_textureViewList.front(),
    ////        //                .imageLayout = attachment.m_textureImageLayout
    ////        //            });
    ////        //    }
    ////        //    pipeline.RenderPassInputTextures = descriptorSetInfoList;
    ////        //    BuildPipeline(pipeline);
    ////       // }
    //      // subPassList.emplace_back(BuildSubpasses(renderPassLoader, subPass));
    //    }
    //    SubPassList.emplace_back(subPassList);
    //}
    //BuildFrameBuffer(renderPassLoader);
}

void VulkanRenderPass::BuildRenderPass(RenderPassLoader& renderPassLoader)
{
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<bool> useDepthReferences(renderPassLoader.SubPassList.size(), false);
    Vector<VkAttachmentReference> depthReferences(renderPassLoader.SubPassList.size());

    Vector<VkSubpassDescription>          subPassDescriptionList;
    Vector<Vector<VkAttachmentReference>> inputAttachmentReferenceList(renderPassLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> colorAttachmentReferenceList(renderPassLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> resolveAttachmentReferenceList(renderPassLoader.SubPassList.size());
    Vector<Vector<VkSubpassDescription>>  preserveAttachmentReferenceList(renderPassLoader.SubPassList.size());
    Vector<RenderPassAttachmentLoader>    renderPassAttachmentTextureInfoMap = renderPassLoader.AttachmentList;
    for (int x = 0; x < renderPassLoader.SubPassList.size(); x++)
    {
        bool useDepthForThisSubpass = false;
        VkAttachmentReference depthRefForThisSubpass = {};
        for (int y = 0; y < renderPassAttachmentTextureInfoMap.size(); y++)
        {
            RenderPassAttachmentLoader renderAttachment = renderPassAttachmentTextureInfoMap[y];
            switch (renderAttachment.RenderAttachmentTypes[x])
            {
            case RenderAttachmentTypeEnum::ColorRenderedTexture: colorAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::InputAttachmentTexture: 
            {
                bool is_depth = (renderAttachment.TextureByteFormat >= VK_FORMAT_D16_UNORM && renderAttachment.TextureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT);
                VkImageLayout input_layout = is_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                inputAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = input_layout });
                break;
            }
            case RenderAttachmentTypeEnum::ResolveAttachmentTexture: resolveAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::DepthRenderedTexture:  depthRefForThisSubpass = VkAttachmentReference{ .attachment = (uint)(y), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }; useDepthForThisSubpass = true; break;
            case RenderAttachmentTypeEnum::SkipSubPass: break;
            default: throw std::runtime_error("Case doesn't exist: RenderedTextureType");
            }
        }

        depthReferences[x] = depthRefForThisSubpass;
        useDepthReferences[x] = useDepthForThisSubpass;

        subPassDescriptionList.emplace_back(VkSubpassDescription{
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList[x].size()),
                .pInputAttachments = inputAttachmentReferenceList[x].empty() ? nullptr : inputAttachmentReferenceList[x].data(),
                .colorAttachmentCount = static_cast<uint32>(colorAttachmentReferenceList[x].size()),
                .pColorAttachments = colorAttachmentReferenceList[x].empty() ? nullptr : colorAttachmentReferenceList[x].data(),
                .pResolveAttachments = resolveAttachmentReferenceList[x].empty() ? nullptr : resolveAttachmentReferenceList[x].data(),
                .pDepthStencilAttachment = useDepthReferences[x] ? &depthReferences[x] : nullptr,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr
            });
    }

    BuildAttachmentDescriptors(renderPassLoader);
    BuildAttachments(renderPassAttachmentTextureInfoMap);

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo{};
    if (renderPassLoader.UseCubeMapMultiView)
    {
        const uint32 viewMask = 0b0000111111;
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &viewMask
        };
    }

    Vector<VkSubpassDependency> subpassDependencies = renderPassLoader.SubpassDependencyList;
    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = renderPassLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
        .attachmentCount = static_cast<uint32>(AttachmentDescriptionList.size()),
        .pAttachments = AttachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &RenderPass));
}

void VulkanRenderPass::BuildPipeline(VulkanPipelineLoader& pipelineLoader, bool useGlobalBindlessSet)
{
    pipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = SampleCount;
    pipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (SampleCount > VK_SAMPLE_COUNT_1_BIT);
    pipelineLoader.RenderPassId = RenderPassId;
    pipelineLoader.RenderPass = RenderPass;
    pipelineLoader.RenderPassResolution = RenderPassResolution;
    pipelineLoader.UseGlobalBindlessSet = useGlobalBindlessSet;

    VulkanPipeline pipeline;
    pipeline.BuildPipelines(pipelineLoader);
    PipelineList.emplace_back(pipeline);
}

VulkanSubPass VulkanRenderPass::BuildSubpasses(RenderPassLoader& renderPassLoader, VulkanSubPassLoader& subPassLoader)
{
    return VulkanSubPass
    {
        .RenderPassGuid = RenderPassId,
        .PipelineGuid = renderPassLoader.PipelineList.front().PipelineId,
        .MeshType = subPassLoader.MeshType,
        .ShaderPushConstant = subPassLoader.ShaderPushConstant,
        .InputTextureList = subPassLoader.InputTextureList,
        .OutputTextureList = subPassLoader.OutputTextureList,
        .OffScreenFrameBuffer = subPassLoader.OffScreenRenderPass,
    };
}

void VulkanRenderPass::BuildAttachmentDescriptors(RenderPassLoader& renderPassLoader)
{
    for (int x = 0; x < renderPassLoader.AttachmentList.size(); x++)
    {
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        const RenderPassAttachmentLoader& renderAttachment = renderPassLoader.AttachmentList[x];
        switch (renderAttachment.TextureUsageType)
        {
            case kUsageType_SwapChainTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
            case kUsageType_OffscreenColorTexture: initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
            case kUsageType_GBufferTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_IrradianceTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_PrefilterTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_CubeMap:               initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_BRDFTexture:           initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            default: throw std::runtime_error("Unknown TextureUsageType");
        }

        AttachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
                .format = renderAttachment.TextureByteFormat,
                .samples = SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : SampleCount,
                .loadOp = renderAttachment.LoadOp,
                .storeOp = renderAttachment.StoreOp,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = initialLayout,
                .finalLayout = finalLayout
            });
    }
}

void VulkanRenderPass::BuildAttachments(Vector<RenderPassAttachmentLoader>& attachmentTextureList)
{
    for (auto& attachment : attachmentTextureList)
    {
        VulkanTexture texture = VulkanTexture(RenderPassResolution, attachment);
        AttachmentList.emplace_back(texture);
        m_frameBufferAttachments.emplace_back(texture);
        if (texture.IsDepthTexture()) m_depthAttachment = texture;
    }
}

//void VulkanRenderPass::BuildFrameBuffer(RenderPassLoader& renderPassLoader)
//{
//    if (AttachmentList.empty()) return;
//
//    uint32 maxMips = 0;
//    for (const auto& attachment : AttachmentList) maxMips = std::max(maxMips, attachment.MipMapLevels());
//    FrameBufferList.resize(maxMips);
//
//    for (uint32 mip = 0; mip < maxMips; ++mip)
//    {
//        uint32 width = 0;
//        uint32 height = 0;
//        Vector<VkImageView> attachmentViews;
//
//        for (auto& attachment : AttachmentList)
//        {
//            if (mip < attachment.TextureViews().size())
//            {
//                attachmentViews.push_back(attachment.TextureViews()[mip]);
//                if (width == 0)
//                {
//                    width = std::max(1u, static_cast<uint32>(attachment.TextureSize().x) >> mip);
//                    height = std::max(1u, static_cast<uint32>(attachment.TextureSize().y) >> mip);
//                }
//            }
//        }
//
//        if (attachmentViews.empty()) continue;
//        VkFramebufferCreateInfo info
//        {
//            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//            .renderPass = RenderPass,
//            .attachmentCount = static_cast<uint32>(attachmentViews.size()),
//            .pAttachments = attachmentViews.data(),
//            .width = width,
//            .height = height,
//            .layers = info.layers = IsCubeMapRenderPass ? 6u : 1u
//        };
//        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &FrameBufferList[mip]));
//    }
//}