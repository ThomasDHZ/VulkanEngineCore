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
    m_renderPassResolution = ivec2(0) == renderPassLoader.RenderPassResolution ? vulkan.RenderPassResolution() : renderPassLoader.RenderPassResolution;
    m_renderPass = VK_NULL_HANDLE;
    //m_frameBufferList = Vector<VkFramebuffer>();
    m_clearValueList = renderPassLoader.ClearValueList;
    m_sampleCount = renderPassLoader.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : renderPassLoader.SampleCount;
    m_useCubeMapMultiView = renderPassLoader.UseCubeMapMultiView;
    m_isCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass;

    BuildRenderPass(renderPassLoader);
    BuildPipelinePackages(renderPassLoader.PipelinePackageList, renderPassLoader.UseGlobalBindlessSet);
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

    Vector<VkAttachmentDescription> attachmentDescriptionList = BuildAttachmentDescriptors(renderPassLoader);
    BuildAttachments(renderPassAttachmentTextureInfoMap);
    TransitionRenderPassAttachmentsToFinalLayout();

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
        .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &m_renderPass));
}

void VulkanRenderPass::BuildPipelinePackages(Vector<VulkanPipelinePackageLoader>& pipelinePackageLoaderList, bool useGlobalBindlessSet)
{
    auto BuildPipeline = [&](VulkanPipelinePackageLoader& pipelinePackageLoader, VulkanPipelineLoader& pipelineLoader)
        {
            VulkanPipeline pipeline;
            pipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = m_sampleCount;
            pipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (m_sampleCount > VK_SAMPLE_COUNT_1_BIT);
            pipeline.BuildPipelines(pipelinePackageLoader, pipelineLoader);
            m_pipelineList.emplace_back(pipeline);
            return pipeline.PipelineId();
        };

    for (auto& pipelinePackageLoader : pipelinePackageLoaderList)
    {
        Vector<VkDescriptorImageInfo> descriptorSetInfoList;
        for (int x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
        {
            for (auto& attachment : m_attachmentList[x])
            {
                descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
                    {
                        .sampler = attachment.m_textureSampler,
                        .imageView = attachment.m_textureViewList.front(),
                        .imageLayout = attachment.m_textureImageLayout
                    });
            }
        }

        pipelinePackageLoader.RenderPassId = m_renderPassId;
        pipelinePackageLoader.RenderPass = m_renderPass;
        pipelinePackageLoader.RenderPassResolution = m_renderPassResolution;
        pipelinePackageLoader.UseGlobalBindlessSet = useGlobalBindlessSet;
        pipelinePackageLoader.RenderPassInputTextures = descriptorSetInfoList;
        pipelinePackageLoader.RenderPassId = m_renderPassId;
        pipelinePackageLoader.RenderPass = m_renderPass;
        pipelinePackageLoader.RenderPassResolution = m_renderPassResolution;
        pipelinePackageLoader.UseGlobalBindlessSet = useGlobalBindlessSet;

        VulkanPipelinePackage pipelinePackage;
        pipelinePackage.PipelinePackageId = pipelinePackageLoader.PipelinePackageId;
        for (auto& pipelineLoader : pipelinePackageLoader.PipelineMap)
        {
            pipelinePackage.PipelineMap[pipelineLoader.first] = BuildPipeline(pipelinePackageLoader, pipelineLoader.second);
        }
        m_pipelinePackageList.emplace_back(pipelinePackage);
    }
}

VulkanSubPass VulkanRenderPass::BuildSubpasses(VulkanSubPassLoader& subPassLoader)
{
    return VulkanSubPass
    {
        .RenderPassGuid = m_renderPassId,
        .PipelinePackageGuid = subPassLoader.PipelinePackageGuid,
        .MeshType = subPassLoader.MeshType,
        .ShaderPushConstant = subPassLoader.ShaderPushConstant,
        .InputTextureList = subPassLoader.InputTextureList,
        .OutputTextureList = subPassLoader.OutputTextureList,
        .OffScreenFrameBuffer = subPassLoader.OffScreenRenderPass,
    };
}

Vector<VkAttachmentDescription> VulkanRenderPass::BuildAttachmentDescriptors(RenderPassLoader& renderPassLoader)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList;
    for(auto& attachment : renderPassLoader.AttachmentList)
    {
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        switch (attachment.TextureUsageType)
        {
            case kUsageType_SwapChainTexture:       initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
            case kUsageType_GBufferTexture:         initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         break;
            case kUsageType_OffscreenColorTexture:  initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_CubeMap:                initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_IrradianceTexture:      initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_BRDFTexture:            initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_PrefilterTexture:       initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_DepthBufferTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                  finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
            default: throw std::runtime_error("Unknown TextureUsageType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
                .format = attachment.TextureByteFormat,
                .samples = m_sampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : m_sampleCount,
                .loadOp = attachment.LoadOp,
                .storeOp = attachment.StoreOp,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = initialLayout,
                .finalLayout = finalLayout
            });
    }
    return attachmentDescriptionList;
}

void VulkanRenderPass::BuildAttachments(Vector<RenderPassAttachmentLoader>& attachmentTextureList)
{
    for (int x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
    {
        for (auto& attachment : attachmentTextureList)
        {
            VulkanTexture texture = VulkanTexture(m_renderPassResolution, attachment);
            m_attachmentList[x].emplace_back(texture);
        }
    }
}

void VulkanRenderPass::BuildFrameBuffer(RenderPassLoader& renderPassLoader)
{
    for (uint32 f = 0; f < MAX_FRAMES_IN_FLIGHT; ++f)
    {
        if (m_attachmentList[f].empty()) continue;

        uint32 maxMips = 1;
        for (const auto& att : m_attachmentList[f])
        {
            maxMips = std::max(maxMips, att.MipMapLevels());
        }

        m_frameBufferList[f].resize(maxMips);

        for (uint32 mip = 0; mip < maxMips; ++mip)
        {
            Vector<VkImageView> views;
            views.reserve(m_attachmentList[f].size());

            for (auto& att : m_attachmentList[f])
            {
                if (mip < att.TextureViews().size()) views.push_back(att.TextureViews()[mip]);
            }

            if (views.empty()) continue;
            const uint32 width = std::max(1u, static_cast<uint32>(m_attachmentList[f][0].TextureSize().x) >> mip);
            const uint32 height = std::max(1u, static_cast<uint32>(m_attachmentList[f][0].TextureSize().y) >> mip);
            VkFramebufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_renderPass,
                .attachmentCount = static_cast<uint32>(views.size()),
                .pAttachments = views.data(),
                .width = width,
                .height = height,
                .layers = renderPassLoader.UseCubeMapMultiView ? 1u : (m_isCubeMapRenderPass ? 6u : 1u)
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &m_frameBufferList[f][mip]));
        }
    }
}

void VulkanRenderPass::TransitionRenderPassAttachmentsToFinalLayout()
{
    VkCommandBuffer commandBuffer = vulkan.CommandBuffer().BeginSingleUseCommand();
    for (int x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
    {
        for (auto& texture : m_attachmentList[x])
        {
            VkImageAspectFlags aspect = texture.IsDepthTexture() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            if (texture.IsStencil()) aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

            VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = 0,
                .dstAccessMask = static_cast<VkAccessFlags>(texture.IsDepthTexture() ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT),
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = texture.IsDepthTexture() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = texture.TextureImage(),
                .subresourceRange = {
                    .aspectMask = aspect,
                    .baseMipLevel = 0,
                    .levelCount = texture.MipMapLevels(),
                    .baseArrayLayer = 0,
                    .layerCount = texture.IsCubeMap() ? 6u : 1u
                }
            };
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, texture.IsDepthTexture() ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }
    }
    vulkan.CommandBuffer().EndSingleUseCommand(commandBuffer);
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

const VulkanPipelinePackage* VulkanRenderPass::FindPipelinePackage(VkGuid& pipelinePackageId)
{
    auto it = std::find_if(m_pipelinePackageList.begin(), m_pipelinePackageList.end(), [&](const VulkanPipelinePackage& pipelinePackage)
        {
            return pipelinePackage.PipelinePackageId == pipelinePackageId;
        });

    if (it != m_pipelinePackageList.end()) return &(*it);
    return nullptr;
}

void VulkanRenderPass::BeginRenderPass(VkCommandBuffer& commandBuffer, uint mipLevel)
{
    const uint32 renderPassWidth =  std::max(1, m_renderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, m_renderPassResolution.y >> mipLevel);
    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = m_frameBufferList[vulkan.Swapchain().CommandIndex()][mipLevel],
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

void VulkanRenderPass::BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet = 0)
{
    if (!FindRenderPipeline(pipeline.PipelineId()))
    {
        std::cerr << "[VulkanRenderPass] Pipeline not registered with this render pass.\n";
        return;
    }

   // if (m_currentBoundPipeline == pipeline.PipelineId()) return;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout(), firstSet, static_cast<uint32>(pipeline.DescriptorSetList().size()), pipeline.DescriptorSetList().data(), 0, nullptr);
   // m_currentBoundPipeline = pipeline.PipelineId();
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

void VulkanRenderPass::Destroy()
{
    //for (auto& frameBuffer : m_frameBufferList)
    //{
    //    if (frameBuffer != VK_NULL_HANDLE)
    //    {
    //        vkDestroyFramebuffer(vulkan.LogicalDevice(), frameBuffer, nullptr);
    //        frameBuffer = VK_NULL_HANDLE;
    //    }
    //}
    if (m_renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    if (!m_renderPass)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
}

VkGuid                        VulkanRenderPass::RenderPassId()         const noexcept { return m_renderPassId; }
ivec2                         VulkanRenderPass::RenderPassResolution() const noexcept { return m_renderPassResolution; }
Vector<VulkanTexture>         VulkanRenderPass::AttachmentList()       const noexcept { return m_attachmentList[vulkan.Swapchain().CommandIndex()]; }
Vector<VulkanPipeline>        VulkanRenderPass::PipelineList()         const noexcept { return m_pipelineList; }
Vector<VulkanPipelinePackage> VulkanRenderPass::PipelinePackageList()  const noexcept { return m_pipelinePackageList; }
Vector<Vector<VulkanSubPass>> VulkanRenderPass::SubPassList()          const noexcept { return m_subPassList; }
VkSampleCountFlagBits         VulkanRenderPass::SampleCount()          const noexcept { return m_sampleCount; }
bool                          VulkanRenderPass::IsCubeMapRenderPass()  const noexcept { return m_isCubeMapRenderPass; }