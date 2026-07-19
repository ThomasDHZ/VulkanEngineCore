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
    m_renderPassId = renderPassLoader.RenderPassId;
    m_renderPassResolution = ivec2(INT32_MAX) == renderPassLoader.RenderPassResolution || ivec2(0) == renderPassLoader.RenderPassResolution ? vulkan.RenderPassResolution() : renderPassLoader.RenderPassResolution;
    m_renderPass = VK_NULL_HANDLE;
    m_frameBufferList = Vector<VkFramebuffer>();
    m_clearValueList = renderPassLoader.ClearValueList;
    m_sampleCount = renderPassLoader.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : renderPassLoader.SampleCount;
    m_useCubeMapMultiView = renderPassLoader.UseCubeMapMultiView;
    m_isCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass;

    BuildRenderPass(renderPassLoader);
    for (auto& pipeline : renderPassLoader.PipelineList)
    {
        pipeline.RenderPassId = m_renderPassId;
        pipeline.RenderPass = m_renderPass;
        pipeline.RenderPassResolution = m_renderPassResolution;
        pipeline.UseGlobalBindlessSet = renderPassLoader.UseGlobalBindlessSet;

        Vector<VkDescriptorImageInfo> descriptorSetInfoList;
        for (auto& attachment : m_attachmentList)
        {
            descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
                {
                    .sampler = attachment.m_textureSampler,
                    .imageView = attachment.m_textureViewList.front(),
                    .imageLayout = attachment.m_textureImageLayout
                });
        }
        pipeline.RenderPassInputTextures = descriptorSetInfoList;
        BuildPipeline(pipeline, renderPassLoader.UseGlobalBindlessSet);
    }
    BuildFrameBuffer(renderPassLoader);
    for (auto& renderPass : renderPassLoader.SubPassList)
    {
        Vector<VulkanSubPass> subPassList;
        for (auto& subPass : renderPass)
        {
            subPassList.emplace_back(BuildSubpasses(subPass));
        }
        m_subPassList.emplace_back(subPassList);
    }
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
        .attachmentCount = static_cast<uint32>(m_attachmentDescriptionList.size()),
        .pAttachments = m_attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &m_renderPass));
}

void VulkanRenderPass::BuildPipeline(VulkanPipelineLoader& pipelineLoader, bool useGlobalBindlessSet)
{
    pipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = m_sampleCount;
    pipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (m_sampleCount > VK_SAMPLE_COUNT_1_BIT);
    pipelineLoader.RenderPassId = m_renderPassId;
    pipelineLoader.RenderPass = m_renderPass;
    pipelineLoader.RenderPassResolution = m_renderPassResolution;
    pipelineLoader.UseGlobalBindlessSet = useGlobalBindlessSet;

    VulkanPipeline pipeline;
    pipeline.BuildPipelines(pipelineLoader);
    m_pipelineList.emplace_back(pipeline);
}

VulkanSubPass VulkanRenderPass::BuildSubpasses(VulkanSubPassLoader& subPassLoader)
{
    return VulkanSubPass
    {
        .RenderPassGuid = m_renderPassId,
        .PipelineGuid = subPassLoader.PipelineGuid,
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
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

        m_attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
                .format = renderAttachment.TextureByteFormat,
                .samples = m_sampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : m_sampleCount,
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
        VulkanTexture texture = VulkanTexture(m_renderPassResolution, attachment);
        m_attachmentList.emplace_back(texture);
        m_frameBufferAttachments.emplace_back(texture);
        if (texture.IsDepthTexture()) m_depthAttachment = texture;
    }
}

void VulkanRenderPass::BuildFrameBuffer(RenderPassLoader& renderPassLoader)
{
    if (m_attachmentList.empty()) return;

    uint32 maxMips = 0;
    for (const auto& attachment : m_attachmentList) maxMips = std::max(maxMips, attachment.MipMapLevels());
    m_frameBufferList.resize(maxMips);

    for (uint32 mip = 0; mip < maxMips; ++mip)
    {
        uint32 width = 0;
        uint32 height = 0;
        Vector<VkImageView> attachmentViews;

        for (auto& attachment : m_attachmentList)
        {
            if (mip < attachment.TextureViews().size())
            {
                attachmentViews.push_back(attachment.TextureViews()[mip]);
                if (width == 0)
                {
                    width = std::max(1u, static_cast<uint32>(attachment.TextureSize().x) >> mip);
                    height = std::max(1u, static_cast<uint32>(attachment.TextureSize().y) >> mip);
                }
            }
        }

        if (attachmentViews.empty()) continue;
        VkFramebufferCreateInfo info
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_renderPass,
            .attachmentCount = static_cast<uint32>(attachmentViews.size()),
            .pAttachments = attachmentViews.data(),
            .width = width,
            .height = height,
            .layers = info.layers = m_isCubeMapRenderPass ? 6u : 1u
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &m_frameBufferList[mip]));
    }
}

void VulkanRenderPass::BeginRenderPass(VkCommandBuffer& commandBuffer, uint mipLevel)
{
    const uint32 renderPassWidth =  std::max(1, m_renderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, m_renderPassResolution.y >> mipLevel);
    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = m_frameBufferList[mipLevel],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D
            {
                .x = 0,
                .y = 0
            },
           .extent = VkExtent2D
            {
                .width = renderPassWidth,
                .height = renderPassHeight
            }
        },
        .clearValueCount = static_cast<uint32>(m_clearValueList.size()),
        .pClearValues = m_clearValueList.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::BindViewPort(VkCommandBuffer& commandBuffer, uint drawMipLevel)
{
    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_renderPassResolution.x),
        .height = static_cast<float>(m_renderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D rect2D = VkRect2D
    {
       .offset = VkOffset2D {.x = 0, .y = 0 },
       .extent = VkExtent2D {.width = static_cast<uint32>(m_renderPassResolution.x), .height = static_cast<uint32>(m_renderPassResolution.y) }
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
}

void VulkanRenderPass::BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet)
{
    if (pipeline.Pipeline() == nullptr)
    {
        std::cout << "Pipeline not set" << std::endl;
        return;
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout(), firstSet, pipeline.DescriptorSetList().size(), pipeline.DescriptorSetList().data(), 0, nullptr);
}

void VulkanRenderPass::DrawMesh(VkCommandBuffer cmd, MeshDrawMessage& mesh)
{
    if (mesh.VertexBuffer != VK_NULL_HANDLE) vkCmdBindVertexBuffers(cmd, mesh.VertexBufferBinding, 1, &mesh.VertexBuffer, &mesh.VertexOffset);
    if (mesh.IndexBuffer != VK_NULL_HANDLE)
    {
        vkCmdBindIndexBuffer(cmd, mesh.IndexBuffer, mesh.FirstIndex * sizeof(uint32), VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, mesh.IndexCount, mesh.InstanceCount, mesh.FirstIndex, 0, mesh.StartInstanceIndex);
    }
    else
    {
        vkCmdDraw(cmd, mesh.VertexCount, mesh.InstanceCount, mesh.FirstVertex, mesh.StartInstanceIndex);
    }
}

void VulkanRenderPass::NextSubpass(VkCommandBuffer& commandBuffer)
{
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::EndRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

const VulkanPipeline* VulkanRenderPass::FindRenderPipeline(const VkGuid& pipelineId)
{
    auto it = std::find_if(m_pipelineList.begin(), m_pipelineList.end(), [&](const VulkanPipeline& pipeline)
        {
            return pipeline.PipelineId() == pipelineId;
        });

    if (it != m_pipelineList.end()) return &(*it);
    return nullptr;
}

void VulkanRenderPass::Destroy()
{
    for (auto& pipeline : m_pipelineList)
    {
        pipeline.Destroy();
    }
    for (auto& frameBuffer : m_frameBufferList)
    {
        if (!frameBuffer) vkDestroyFramebuffer(vulkan.LogicalDevice(), frameBuffer, nullptr);
        frameBuffer = VK_NULL_HANDLE;
    }
    if (!m_renderPass)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
}

VkGuid                        VulkanRenderPass::RenderPassId()         const noexcept { return m_renderPassId; }
ivec2                         VulkanRenderPass::RenderPassResolution() const noexcept { return m_renderPassResolution; }
Vector<VulkanTexture>         VulkanRenderPass::AttachmentList()       const noexcept { return m_attachmentList; }
Vector<VulkanPipeline>        VulkanRenderPass::PipelineList()         const noexcept { return m_pipelineList; }
Vector<Vector<VulkanSubPass>> VulkanRenderPass::SubPassList()          const noexcept { return m_subPassList; }
VkSampleCountFlagBits         VulkanRenderPass::SampleCount()          const noexcept { return m_sampleCount; }