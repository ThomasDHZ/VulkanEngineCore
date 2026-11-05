#pragma once
#include "DLL.h"
#include "Platform.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLoader.h"
#include "VulkanTexture.h"

class VulkanRenderPassBuilder
{
private:
public:
	VulkanRenderPassBuilder();
	~VulkanRenderPassBuilder();

	void AddColorAttachment(const String& name, VkFormat format, VkClearColorValue clear = {});
};

