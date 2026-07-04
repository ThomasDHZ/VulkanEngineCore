#pragma once
#include "Platform.h"
#include "VulkanSystem.h"
#include "ShaderStructs.h"

class VulkanShader
{
private:
	VkShaderModule                               m_shaderModule;
    VkShaderStageFlagBits                        m_shaderStages;
    ShaderPushConstant                           m_pushConstant;
    Vector<VkVertexInputAttributeDescription>    m_inputVertexAttributeList;
    Vector<VkVertexInputAttributeDescription>    m_outputVertexAttributeList;
    Vector<ShaderDescriptorBinding>              m_descriptorBindingList;
    Vector<SpvReflectSpecializationConstant>     m_specializationConstantList;

    VkVertexInputRate                            LoadVertexInputRate();
    void                                         LoadShaderVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList);
    Vector<SpvReflectInterfaceVariable*>         LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module);
    void                                         LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList);
    void                                         LoadShaderDescriptorBindings(const SpvReflectShaderModule& module);
    void                                         LoadShaderSpecialConstants(const SpvReflectShaderModule& module);
    ShaderStruct                                 LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo);
    Vector<ShaderVariable>                       LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize);

public:
	VulkanShader();
	VulkanShader(Vector<byte> shaderData, VkShaderStageFlagBits shaderStages);
	~VulkanShader();

	VkPipelineShaderStageCreateInfo              GetShader();
    Vector<SpvReflectInterfaceVariable>          GetShaderVertexInputVariables(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectInterfaceVariable>          GetShaderVertexOutputVariables(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectSpecializationConstant>     GetShaderSpecialConstants(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectBlockVariable>              GetShaderPushConstants(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectDescriptorBinding>          GetShaderDescriptorBindings(const SpvReflectShaderModule& shaderModule);

    Vector<SpvReflectSpecializationConstant>     SearchShaderSpecialConstants(const String& searchString);

};

