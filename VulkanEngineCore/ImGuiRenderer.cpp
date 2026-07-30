#include "ImGuiRenderer.h"
#include "VulkanSystem.h"

ImGuiRenderer& imGuiSystem = ImGuiRenderer::Get();

void ImGuiRenderer::StartUp()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)vulkanWindow.GetWindowHandle(), true);
	
	m_renderPass = CreateRenderPass();
	m_swapChainFramebuffers = CreateRendererFramebuffers(m_renderPass);

    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
        .poolSizeCount = (uint32)IM_ARRAYSIZE(poolSizes),
        .pPoolSizes = poolSizes
    };
    vkCreateDescriptorPool(vulkan.LogicalDevice(), &pool_info, nullptr, &m_imGuiDescriptorPool);

    ImGui_ImplVulkan_InitInfo init_info =
    {
        .Instance = vulkan.InstanceHandle(),
        .PhysicalDevice = vulkan.PhysicalDevice(),
        .Device = vulkan.LogicalDevice(),
        .QueueFamily = vulkan.Device().GraphicsFamily(),
        .Queue = vulkan.GraphicsQueue(),
        .DescriptorPool = m_imGuiDescriptorPool,
        .MinImageCount = static_cast<uint32>(vulkan.SwapChainImageCount()),
        .ImageCount = static_cast<uint32>(vulkan.SwapChainImageCount()),
        .PipelineCache = VK_NULL_HANDLE,
        .PipelineInfoMain = ImGui_ImplVulkan_PipelineInfo
        {
            .RenderPass = m_renderPass,
            .Subpass = 0,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        },
        .Allocator = nullptr,
        .CheckVkResultFn = ImGuiRenderer::ImGuiResult
    };
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiRenderer::StartFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Button Window");
}

void ImGuiRenderer::EndFrame()
{
    ImGui::End();
    ImGui::Render();
}

void ImGuiRenderer::Draw(VkCommandBuffer& commandBuffer)
{
    std::vector<VkClearValue> clearValues
    {
        VkClearValue {.color = { {0.0f, 0.0f, 0.0f, 1.0f} } }
    };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = m_swapChainFramebuffers[vulkan.Swapchain().ImageIndex()],
        .renderArea
        {
            .offset = { 0, 0 },
            .extent = vulkan.SwapChainResolution(),
        },
        .clearValueCount = 0,
        .pClearValues = nullptr
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}

void ImGuiRenderer::RebuildSwapChain()
{
    // vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &imGuiRenderer.RenderPass);
// vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, imGuiRenderer.SwapChainFramebuffers);
    m_renderPass = CreateRenderPass();
    m_swapChainFramebuffers = CreateRendererFramebuffers(m_renderPass);
}

VkRenderPass ImGuiRenderer::CreateRenderPass()
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkAttachmentDescription colorAttachment
    {
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    VkSubpassDependency dependency
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };


    VkRenderPassCreateInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

Vector<VkFramebuffer> ImGuiRenderer::CreateRendererFramebuffers(const VkRenderPass& renderPass)
{
    Vector<VkFramebuffer> frameBuffers = Vector<VkFramebuffer>(vulkan.SwapChainImageCount());
    for (size_t x = 0; x < vulkan.SwapChainImageCount(); x++)
    {
        std::vector<VkImageView> attachments =
        {
            vulkan.Swapchain().SwapChainImageViews()[x]
        };

        VkFramebufferCreateInfo frameBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = vulkan.SwapChainResolution().width,
            .height = vulkan.SwapChainResolution().height,
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &frameBufferInfo, nullptr, &frameBuffers[x]));
    }
    return frameBuffers;
}

void ImGuiRenderer::ImGuiResult(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0) abort();
}

void ImGuiRenderer::Destroy()
{
    ImGui_ImplVulkan_Shutdown();
    //vulkanSystem.DestroyDescriptorPool(vulkanSystem.Device, &imGuiRenderer.ImGuiDescriptorPool);
    //vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &imGuiRenderer.RenderPass);
    //vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, imGuiRenderer.SwapChainFramebuffers);
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiRenderer::FpsDisplay()
{
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

bool ImGuiRenderer::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt2(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt3(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt4(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat2(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat3(label, v, v_min, v_max, format, flags);
}

bool ImGuiRenderer::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat4(label, v, v_min, v_max, format, flags);
}
