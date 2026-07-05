#pragma once
#include "Platform.h"
#include "VulkanSystem.h"
#include "ShaderStructs.h"

class DLL_EXPORT VulkanShader
{
private:
	VkShaderModule                               m_shaderModule = VK_NULL_HANDLE;
    VkShaderStageFlagBits                        m_shaderStages;
    ShaderPushConstant                           m_pushConstant;
    Vector<VkVertexInputAttributeDescription>    m_inputVertexAttributeList;
    Vector<VkVertexInputAttributeDescription>    m_outputVertexAttributeList;
    Vector<VkVertexInputBindingDescription>      m_vertexInputBindingList;
    Vector<ShaderDescriptorBinding>              m_descriptorBindingList;
    Vector<SpvReflectSpecializationConstant>     m_specializationConstantList;

    void                                         LoadShader(const Vector<byte>& shaderCode);
    VkVertexInputRate                            LoadVertexInputRate();
    void                                         LoadShaderVertexInputVariables(const SpvReflectShaderModule& module);
    Vector<SpvReflectInterfaceVariable*>         LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module);
    void                                         LoadShaderConstantBufferData(const SpvReflectShaderModule& module);
    void                                         LoadShaderDescriptorBindings(const SpvReflectShaderModule& module);
    void                                         LoadShaderSpecialConstants(const SpvReflectShaderModule& module);
    ShaderStruct                                 LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo);
    Vector<ShaderVariable>                       LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize);

public:
	VulkanShader();
	VulkanShader(const Vector<byte>& shadeCode);
	~VulkanShader();

	VkPipelineShaderStageCreateInfo              GetShader();
    Vector<SpvReflectInterfaceVariable>          GetShaderVertexInputVariables(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectInterfaceVariable>          GetShaderVertexOutputVariables(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectSpecializationConstant>     GetShaderSpecialConstants(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectBlockVariable>              GetShaderPushConstants(const SpvReflectShaderModule& shaderModule);
    Vector<SpvReflectDescriptorBinding>          GetShaderDescriptorBindings(const SpvReflectShaderModule& shaderModule);

    Vector<SpvReflectSpecializationConstant>     SearchShaderSpecialConstants(const String& searchString);

};

