//#pragma once
//#include "Platform.h"
//#include "VulkanRenderPass.h"
//
//struct VertexLayout
//{
//    uint64 VertexDataSize = UINT64_MAX;
//    void* VertexData = nullptr;
//};
//
//class VulkanMesh
//{
//private:
//public:
//	VulkanMesh();
//	~VulkanMesh();
//
//	void CreateMesh(MeshTypeEnum meshtype, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId = VkGuid());
//    void Update(const float& deltaTime);
//    MeshDrawMessage Draw();
//
//    uint32 VertexBufferId = UINT32_MAX;
//    uint32 IndexBufferId = UINT32_MAX;
//    uint32 VertexCount = 0;
//    uint32 IndexCount = 0;
//    uint32 VertexStride = 0; 
//    MeshTypeEnum Type = MeshTypeEnum::kMesh_None;
//};
//
