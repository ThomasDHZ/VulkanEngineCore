#include "ImGuiSystem.h"
#include "VulkanSystem.h"

ImGuiSystem& imGuiSystem = ImGuiSystem::Get();

void ImGuiSystem::StartUp()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)vulkanWindow.GetWindowHandle(), true);
	
	CreateRenderPass();
	CreateRendererFramebuffers();
    CreateDescriptorPool();

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
        .CheckVkResultFn = ImGuiSystem::ImGuiResult
    };
    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiSystem::StartFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Button Window");
}

void ImGuiSystem::EndFrame()
{
    ImGui::End();
    ImGui::Render();
}

void ImGuiSystem::Draw(VkCommandBuffer& commandBuffer)
{
    Vector<VkClearValue> clearValues { VkClearValue {.color = { {0.0f, 0.0f, 0.0f, 1.0f} } } };
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

void ImGuiSystem::RebuildSwapChain()
{
    for (auto& frameBuffer : m_swapChainFramebuffers)
    {
        if (frameBuffer) vkDestroyFramebuffer(vulkan.LogicalDevice(), frameBuffer, nullptr);
        frameBuffer = VK_NULL_HANDLE;
    }
    if (m_renderPass)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    CreateRenderPass();
    CreateRendererFramebuffers();
}

void ImGuiSystem::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment
    {
        .format = vulkan.Swapchain().SwapChainImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &m_renderPass));
}

void ImGuiSystem::CreateDescriptorPool()
{
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
}

void ImGuiSystem::CreateRendererFramebuffers()
{
    m_swapChainFramebuffers.resize(vulkan.SwapChainImageCount());
    for (size_t x = 0; x < vulkan.SwapChainImageCount(); x++)
    {
        Vector<VkImageView> attachments = { vulkan.Swapchain().SwapChainImageViews()[x] };
        VkFramebufferCreateInfo frameBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_renderPass,
            .attachmentCount = static_cast<uint32>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = vulkan.SwapChainResolution().width,
            .height = vulkan.SwapChainResolution().height,
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &frameBufferInfo, nullptr, &m_swapChainFramebuffers[x]));
    }
}

void ImGuiSystem::ImGuiResult(VkResult err)
{
    if (err == 0) return;
    printf("VkResult %d\n", err);
    if (err < 0) abort();
}

void ImGuiSystem::Destroy()
{
    ImGui_ImplVulkan_Shutdown();
    if (m_imGuiDescriptorPool)
    {
        vkDestroyDescriptorPool(vulkan.LogicalDevice(), m_imGuiDescriptorPool, nullptr);
        m_imGuiDescriptorPool = VK_NULL_HANDLE;
    }
    for (auto& frameBuffer : m_swapChainFramebuffers)
    {
        if (frameBuffer) vkDestroyFramebuffer(vulkan.LogicalDevice(), frameBuffer, nullptr);
        frameBuffer = VK_NULL_HANDLE;
    }
    if (m_renderPass)
    {
        vkDestroyRenderPass(vulkan.LogicalDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiSystem::FpsDisplay()
{
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

bool ImGuiSystem::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt2(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt3(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderInt4(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat2(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat3(label, v, v_min, v_max, format, flags);
}

bool ImGuiSystem::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderFloat4(label, v, v_min, v_max, format, flags);
}

void ImGuiSystem::Separator()
{
    ImGui::Separator();
}

void ImGuiSystem::SameLine(float offset_from_start_x, float spacing)
{
    ImGui::SameLine(offset_from_start_x, spacing);
}

void ImGuiSystem::NewLine()
{
    ImGui::NewLine();
}

bool ImGuiSystem::Button(const char* label, const ImVec2& size)
{
    return ImGui::Button(label, size);
}

bool ImGuiSystem::Checkbox(const char* label, bool* v)
{
    return ImGui::Checkbox(label, v);
}

bool ImGuiSystem::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    return ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
}

bool ImGuiSystem::InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return ImGui::InputFloat(label, v, step, step_fast, format, flags);
}

bool ImGuiSystem::InputFloat2(const char* label, float v[2], const char* format, ImGuiInputTextFlags flags)
{
    return ImGui::InputFloat2(label, v, format, flags);
}

bool ImGuiSystem::InputFloat3(const char* label, float v[3], const char* format, ImGuiInputTextFlags flags)
{
    return ImGui::InputFloat3(label, v, format, flags);
}

bool ImGuiSystem::InputFloat4(const char* label, float v[4], const char* format, ImGuiInputTextFlags flags)
{
    return ImGui::InputFloat4(label, v, format, flags);
}

bool ImGuiSystem::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
{
    return ImGui::InputInt(label, v, step, step_fast, flags);
}

bool ImGuiSystem::InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags)
{
    return ImGui::InputInt2(label, v, flags);
}

bool ImGuiSystem::InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags)
{
    return ImGui::InputInt3(label, v, flags);
}

bool ImGuiSystem::InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags)
{
    return ImGui::InputInt4(label, v, flags);
}

bool ImGuiSystem::InputDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags)
{
    return ImGui::InputDouble(label, v, step, step_fast, format, flags);
}

bool ImGuiSystem::IsKeyDown(ImGuiKey key)
{
    return ImGui::IsKeyDown(key);
}

bool ImGuiSystem::IsKeyPressed(ImGuiKey key, bool repeat)
{
    return ImGui::IsKeyPressed(key, repeat);
}

bool ImGuiSystem::IsKeyReleased(ImGuiKey key)
{
    return ImGui::IsKeyReleased(key);
}

bool ImGuiSystem::IsItemActive()
{
    return ImGui::IsItemActive();
}

bool ImGuiSystem::IsItemFocused()
{
    return ImGui::IsItemFocused();
}

void ImGuiSystem::Text(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

bool ImGuiSystem::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    return ImGui::Begin(name, p_open, flags);
}

bool ImGuiSystem::BeginChild(const char* str_id, const ImVec2& size, bool border, ImGuiWindowFlags flags)
{
    return ImGui::BeginChild(str_id, size, border, flags);
}

void ImGuiSystem::TextColored(const ImVec4& col, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(col, fmt, args);
    va_end(args);
}

void ImGuiSystem::TextWrapped(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImGui::TextWrappedV(fmt, args);
    va_end(args);
}

float ImGuiSystem::GetScrollY()
{
    return ImGui::GetScrollY();
}

float ImGuiSystem::GetScrollMaxY()
{
    return ImGui::GetScrollMaxY();
}

void ImGuiSystem::SetScrollHereY(float center_y_ratio)
{
    ImGui::SetScrollHereY(center_y_ratio);
}

void ImGuiSystem::EndChild()
{
    ImGui::EndChild();
}

void ImGuiSystem::PushItemWidth(float item_width)
{
    ImGui::PushItemWidth(item_width);
}

void ImGuiSystem::PopItemWidth()
{
    ImGui::PopItemWidth();
}

bool ImGuiSystem::IsWindowAppearing()
{
    return ImGui::IsWindowAppearing();
}

void ImGuiSystem::SetKeyboardFocusHere(int offset)
{
    ImGui::SetKeyboardFocusHere(offset);
}

void ImGuiSystem::End()
{
    ImGui::End();
}

float ImGuiSystem::GetFrameHeightWithSpacing()
{
    return ImGui::GetFrameHeightWithSpacing();
}