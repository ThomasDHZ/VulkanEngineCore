//#include "VulkanMesh.h"
//#include "BufferSystem.h"
//
//VulkanMesh::VulkanMesh()
//{
//}
//
//VulkanMesh::~VulkanMesh()
//{
//}
//
//void VulkanMesh::CreateMesh(MeshTypeEnum meshtype, VertexLayout& vertexData, Vector<uint32>& indexList, VkGuid materialId)
//{
//	VertexBufferId = bufferSystem.CreateStaticVulkanBuffer(vertexData.VertexData, vertexData.VertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//	IndexBufferId = bufferSystem.CreateVulkanBuffer<uint32>(indexList, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true);
//	VertexCount = static_cast<uint>(vertexData.VertexDataSize);
//	IndexCount = static_cast<uint>(indexList.size());
//}
//
//void VulkanMesh::Update(const float& deltaTime)
//{
//}
//
//MeshDrawMessage VulkanMesh::Draw()
//{
//	return MeshDrawMessage
//	{
//		.MeshId = mesh.MeshId,
//		.VertexCount = VertexCount,
//		.IndexCount = IndexCount,
//		.InstanceCount = 1,
//		.FirstIndex = 0,
//		.StartInstanceIndex = 0,
//		.VertexOffset = 0,
//		.VertexBuffer = bufferSystem.FindVulkanBuffer(VertexBufferId).Buffer(),
//		.IndexBuffer = bufferSystem.FindVulkanBuffer(IndexBufferId).Buffer(),
//	};
//}
