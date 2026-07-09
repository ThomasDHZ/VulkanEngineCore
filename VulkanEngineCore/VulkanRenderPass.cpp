#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "PushConstantRegistry.h"

VulkanRenderPass::VulkanRenderPass()
{
}

VulkanRenderPass::~VulkanRenderPass()
{
}

void VulkanRenderPass::BuildRenderPass(const RenderPassLoader& renderPassJsonLoader)
{
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<bool> useDepthReferences(renderPassJsonLoader.SubPassList.size(), false);
    Vector<VkAttachmentReference> depthReferences(renderPassJsonLoader.SubPassList.size());
    Vector<VkSubpassDescription> subPassDescriptionList;
    Vector<Vector<VkAttachmentReference>> inputAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> colorAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> resolveAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkSubpassDescription>> preserveAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoMap = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassJsonLoader.SubPassList.size(); x++)
    {
        bool useDepthForThisSubpass = false;
        VkAttachmentReference depthRefForThisSubpass = {};
        for (int y = 0; y < renderPassAttachmentTextureInfoMap.size(); y++)
        {
            RenderPassAttachmentTexture renderAttachment = renderPassAttachmentTextureInfoMap[y];
            switch (renderAttachment.RenderAttachmentTypes[x])
            {
            case RenderAttachmentTypeEnum::ColorRenderedTexture: colorAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::InputAttachmentTexture: {
                bool is_depth = (renderAttachment.Format >= VK_FORMAT_D16_UNORM && renderAttachment.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
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

    Vector<VkAttachmentDescription> attachmentDescriptionList = BuildRenderPassAttachments(vulkanRenderPass);
    Vector<VulkanTexture> frameBufferTextureList = BuildRenderPassAttachmentTextures(vulkanRenderPass);

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo{};
    if (renderPassJsonLoader.UseCubeMapMultiView)
    {
        const uint32 viewMask = 0b0000111111;  // bits 0-5 for 6 faces
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &viewMask
        };
    }

    Vector<VkSubpassDependency> subpassDependencies = renderPassJsonLoader.SubpassDependencyList;
    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = renderPassJsonLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
        .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &vulkanRenderPass.RenderPass));
}

Vector<VkAttachmentDescription> VulkanRenderPass::BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        const RenderPassAttachmentTexture& renderAttachment = renderPassAttachmentTextureInfoList[x];
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

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
            .format = renderAttachment.Format,
            .samples = vulkanRenderPass.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : vulkanRenderPass.SampleCount,
            .loadOp = renderAttachment.LoadOp,
            .storeOp = renderAttachment.StoreOp,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initialLayout,
            .finalLayout = finalLayout
            });
    }
    return attachmentDescriptionList;
}

Vector<VulkanTexture> VulkanRenderPass::BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass)
{
    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();

    VulkanTexture depthTexture;
    Vector<VulkanTexture> renderedTextureList;
    Vector<VulkanTexture> frameBufferTextureList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        VulkanTexture texture = textureSystem.CreateRenderPassTexture(vulkanRenderPass, x, renderPassAttachmentTextureInfoList[x].TextureType);
        renderedTextureList.emplace_back(texture);
        frameBufferTextureList.emplace_back(texture);

        if (texture.textureType == TextureTypeEnum::kTextureType_DepthTexture)
        {
            depthTexture = texture;
        }

        if (texture.textureType == TextureTypeEnum::kTextureType_CubeMap) memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.CubeMapDescriptorBinding);
        else memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.Texture2DBinding);
    }
    if (!renderedTextureList.empty()) textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderedTextureList);
    if (depthTexture.textureImage != VK_NULL_HANDLE) textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, depthTexture);
    return frameBufferTextureList;
}

void VulkanRenderPass::BuildFrameBuffer(VulkanRenderPass& vulkanRenderPass)
{
    Vector<VulkanTexture> frameBufferAttachment = textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId);
    if (frameBufferAttachment.empty()) return;

    const VulkanTexture& firstTex = frameBufferAttachment[0];
    bool isCubeMap = (firstTex.textureType == TextureTypeEnum::kTextureType_CubeMap);
    if (isCubeMap && !firstTex.textureViewList.empty())
    {
        uint32 mipLevels = firstTex.mipMapLevels;
        uint32 baseSize = firstTex.width;
        vulkanRenderPass.FrameBufferList.resize(mipLevels);
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            uint32 mipWidth = std::max(1u, baseSize >> mip);
            uint32 mipHeight = std::max(1u, baseSize >> mip);
            if (mip >= firstTex.textureViewList.size())
            {
                std::cerr << "Error: Missing mip view " << mip << " for cubemap\n";
                break;
            }

            Vector<VkImageView> attachments{ firstTex.textureViewList[mip] };
            VkFramebufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = mipWidth,
                .height = mipHeight,
                .layers = 1u
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &vulkanRenderPass.FrameBufferList[mip]));
        }
    }
    else
    {
        vulkanRenderPass.FrameBufferList.resize(1);

        Vector<VkImageView> attachments;
        attachments.reserve(frameBufferAttachment.size());
        for (const auto& tex : frameBufferAttachment)
        {
            if (!tex.textureViewList.empty()) attachments.push_back(tex.textureViewList.front());
        }

        VkFramebufferCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vulkanRenderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &vulkanRenderPass.FrameBufferList[0]));
    }
}

void VulkanRenderPass::BuildFrameBuffer(VulkanRenderPass& vulkanRenderPass)
{
    for (auto& renderPassNode : RenderPassNodeList)
    {
        const VulkanRenderPass& renderPass = FindRenderPass(renderPassNode.RenderPassGuid);

        uint32 mipCount = std::max(1u, renderPassNode.MipCount);
        if (renderPassNode.PreRenderPassCmd) renderPassNode.PreRenderPassCmd(commandBuffer, renderPassNode);
        for (uint32 mip = 0; mip < mipCount; mip++)
        {
            bool firstSubPass = true;
            const ivec2 renderPassResolution = ivec2(std::max(1, renderPass.RenderPassResolution.x >> mip),
                std::max(1, renderPass.RenderPassResolution.y >> mip));

            BeginRenderPass(commandBuffer, renderPass, renderPassResolution, mip);
            BindViewPort(commandBuffer, renderPassResolution, mip);
            for (auto& subPass : renderPassNode.SubPassDrawMessage)
            {
                if (!firstSubPass)
                {
                    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
                }

                for (auto& renderPassLayer : subPass)
                {
                    const VulkanPipeline& pipeline = FindRenderPipeline(renderPassLayer.PipelineGuid);

                    VulkanTexture inputTexture;
                    if (!renderPassLayer.RenderPassInputs.empty()) inputTexture = textureSystem.FindRenderedTexture(renderPassLayer.RenderPassInputs[0]);
                    if (renderPassLayer.PreDrawCmd)
                    {
                        renderPassLayer.PreDrawCmd(commandBuffer, renderPassLayer);
                    }

                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline());
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout(), 0, pipeline.DescriptorSetList().size(), pipeline.DescriptorSetList().data(), 0, nullptr);
                    if (renderPassLayer.OffScreenRenderPass)
                    {
                        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
                    }
                    else
                    {
                        for (int x = 0; x < renderPassLayer.DrawMeshList.size(); x++)
                        {
                            if (renderPassLayer.PushConstant.has_value())
                            {
                                const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassLayer.RenderPassGuid);
                                const VulkanPipeline& pipeline = renderSystem.FindRenderPipeline(renderPassLayer.PipelineGuid);
                                PushConstantContext pushConstantContext = PushConstantContext
                                {
                                    .RenderPassGuid = renderPassLayer.RenderPassGuid,
                                    .MeshId = renderPassLayer.DrawMeshList[x].MeshId,
                                    .DrawIndex = static_cast<uint32>(x),
                                    .MipLevel = mip,
                                    .MipCount = mipCount,
                                    .RenderPassResolution = renderPass.RenderPassResolution
                                };

                                ShaderPushConstant shaderPushConstant = shaderSystem.FindShaderPushConstant(renderPassLayer.PushConstant.value());
                                pushConstantRegistry.ApplyPushConstantRules(shaderPushConstant, pushConstantContext);
                                vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, shaderPushConstant.PushConstantSize, shaderPushConstant.PushConstantBuffer.data());
                            }

                            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.VertexBuffer, &mesh.VertexOffset);
                            if (mesh.IndexBuffer)
                            {
                                vkCmdBindIndexBuffer(commandBuffer, mesh.IndexBuffer, mesh.FirstIndex * sizeof(uint32), VK_INDEX_TYPE_UINT32);
                                vkCmdDrawIndexed(commandBuffer, mesh.IndexCount, mesh.InstanceCount, mesh.FirstIndex, 0, mesh.StartInstanceIndex);
                            }
                            else
                            {
                                vkCmdDraw(commandBuffer, mesh.VertexCount, mesh.InstanceCount, mesh.FirstVertex, mesh.StartInstanceIndex);
                            }
                        }
                    }

                    if (renderPassLayer.PostDrawCmd)
                    {
                        renderPassLayer.PostDrawCmd(commandBuffer, renderPassLayer);
                    }
                }

                firstSubPass = false;
            }
            if (renderPassNode.PostRenderPassCmd) renderPassNode.PostRenderPassCmd(commandBuffer, renderPassNode);
            EndRenderPass(commandBuffer);
        }
    }
}

void VulkanRenderPass::BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, ivec2 renderPassResolution, uint mipLevel)
{

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[mipLevel],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D
            {
                .x = 0,
                .y = 0
            },
           .extent = VkExtent2D
            {
                .width = static_cast<uint32>(renderPassResolution.x),
                .height = static_cast<uint32>(renderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel)
{
    const uint32 renderPassWidth = std::max(1, renderPass.RenderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, renderPass.RenderPassResolution.y >> mipLevel);
    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[mipLevel],
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
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::BindViewPort(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel)
{
    if (renderPass.RenderPassResolution == ivec2(INT32_MAX, INT32_MAX))
    {
        return;
    }

    const uint32 renderPassWidth = std::max(1, renderPass.RenderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, renderPass.RenderPassResolution.y >> mipLevel);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPassWidth),
        .height = static_cast<float>(renderPassHeight),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D rect2D = VkRect2D
    {
       .offset = VkOffset2D {.x = 0, .y = 0 },
       .extent = VkExtent2D {.width = renderPassWidth, .height = renderPassHeight }
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
}

void VulkanRenderPass::BindViewPort(VkCommandBuffer& commandBuffer, ivec2 renderPassResolution, uint mipLevel)
{
    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPassResolution.x),
        .height = static_cast<float>(renderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D rect2D = VkRect2D
    {
       .offset = VkOffset2D {.x = 0, .y = 0 },
       .extent = VkExtent2D {.width = static_cast<uint32>(renderPassResolution.x), .height = static_cast<uint32>(renderPassResolution.y) }
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
}

void VulkanRenderPass::EndRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}