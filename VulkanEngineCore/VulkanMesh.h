#pragma once
#include "Platform.h"
#include "VulkanRenderPass.h"

class VulkanMesh
{
private:
public:
	VulkanMesh();
	~VulkanMesh();

	//void CreateMesh(const String& key, MeshTypeEnum meshtype, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId = VkGuid());

	uint32 MeshId = UINT32_MAX;
	uint32 ParentGameObjectId = UINT32_MAX;
	uint64 SharedAssetId = UINT64_MAX;
	uint32 ObjectDataIndex = UINT32_MAX;
	MeshTypeEnum Type = MeshTypeEnum::kMesh_Undefined;
	vec3 Position = vec3(0.0f);
	vec3 Rotation = vec3(0.0f);
	vec3 Scale = vec3(1.0f);
	VkGuid MaterialId;
	bool IsTransformDirty = true;
	bool IsMaterialDirty = true;
};

