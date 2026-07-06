#include "VulkanShader.h"

VulkanShader::VulkanShader()
{
}

VulkanShader::VulkanShader(const Vector<byte>& shaderCode)
{
    SpvReflectShaderModule spvReflectModule;
    SPV_VULKAN_RESULT(spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &spvReflectModule));
    m_shaderStages = static_cast<VkShaderStageFlagBits>(spvReflectModule.shader_stage);

    LoadShader(shaderCode);
    if (spvReflectModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) LoadShaderVertexInputVariables(spvReflectModule);
    LoadShaderSpecialConstants(spvReflectModule);
    LoadShaderConstantBufferData(spvReflectModule);
    LoadShaderDescriptorBindings(spvReflectModule);
    spvReflectDestroyShaderModule(&spvReflectModule);
}

VulkanShader::~VulkanShader()
{
}

VkPipelineShaderStageCreateInfo VulkanShader::GetShader()
{
    return VkPipelineShaderStageCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = m_shaderStages,
        .module = m_shaderModule,
        .pName = "main"
    };
}

void VulkanShader::LoadShaderVertexInputVariables(const SpvReflectShaderModule& spvReflectModule)
{
    Vector<SpvReflectInterfaceVariable> inputs = GetShaderVertexInputVariables(spvReflectModule);
    inputs.erase(std::remove_if(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable input) { return input.built_in != -1; }), inputs.end());
    inputs.shrink_to_fit();

    std::sort(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable a, SpvReflectInterfaceVariable b)
        {
            return a.location < b.location;
        });

    uint32 offset = 0;
    VkVertexInputRate inputRate = LoadVertexInputRate();
    for (int x = 0; x < inputs.size(); x++)
    {
        uint32 binding = 0;
        switch (inputs[x].type_description->op)
        {
            case SpvOpTypeInt:
            {
                m_inputVertexAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x].location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x].format),
                        .offset = offset
                    });
                offset += inputs[x].type_description->traits.numeric.scalar.width / 8;
                break;
            }
            case SpvOpTypeFloat:
            {
                m_inputVertexAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x].location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x].format),
                        .offset = offset
                    });
                offset += inputs[x].type_description->traits.numeric.scalar.width / 8;
                break;
            }
            case SpvOpTypeVector:
            {
                m_inputVertexAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x].location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x].format),
                        .offset = offset
                    });
                offset += (inputs[x].type_description->traits.numeric.scalar.width / 8) * inputs[x].type_description->traits.numeric.vector.component_count;
                break;
            }
            case SpvOpTypeMatrix:
            {
                for (int y = 0; y < inputs[x].type_description->traits.numeric.vector.component_count; y++)
                {
                    m_inputVertexAttributeList.emplace_back(VkVertexInputAttributeDescription
                        {
                            .location = inputs[x].location,
                            .binding = binding,
                            .format = static_cast<VkFormat>(inputs[x].format),
                            .offset = offset
                        });
                    inputs[x].location += 1;
                    offset += (inputs[x].type_description->traits.numeric.scalar.width / 8) * inputs[x].type_description->traits.numeric.vector.component_count;
                }
                break;
            }
        }
        m_vertexInputBindingList.emplace_back(VkVertexInputBindingDescription
            {
               .binding = m_inputVertexAttributeList[x].binding,
               .stride = offset,
               .inputRate = static_cast<VkVertexInputRate>(inputRate)
            });
        if (inputs.size() == x) offset = 0;
    }
}

Vector<SpvReflectInterfaceVariable*> VulkanShader::LoadShaderVertexOutputVariables(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&spvReflectModule, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&spvReflectModule, &outputCount, outputs.data()));
    return outputs;
}

void VulkanShader::LoadShaderConstantBufferData(const SpvReflectShaderModule& spvReflectModule)
{
    Vector<SpvReflectBlockVariable> pushConstants = GetShaderPushConstants(spvReflectModule);
    if (pushConstants.empty()) return;

    size_t bufferSize = 0;
    String pushConstantName(pushConstants.front().name);
    Vector<ShaderVariable> shaderStructVariableList = LoadShaderStructVariables(*pushConstants.front().type_description, bufferSize);
    for (auto& shaderVariable : m_pushConstant.PushConstantVariableList)
    {
        shaderVariable.Value = Vector<byte>(shaderVariable.Size, 0x00);
    }

    m_pushConstant = ShaderPushConstant
    {
       .PushConstantName = pushConstantName,
       .PushConstantSize = bufferSize,
       .ShaderStageFlags = static_cast<VkShaderStageFlags>(spvReflectModule.shader_stage),
       .PushConstantVariableList = shaderStructVariableList,
       .PushConstantBuffer = Vector<byte>(bufferSize, 0x00)
    };
}

void VulkanShader::LoadShaderDescriptorBindings(const SpvReflectShaderModule& spvReflectModule)
{
    Vector<SpvReflectDescriptorBinding> descriptorSetBindings = GetShaderDescriptorBindings(spvReflectModule);
    for (auto& descriptorBinding : descriptorSetBindings)
    {
        String name(descriptorBinding.name);
        m_descriptorBindingList.emplace_back(ShaderDescriptorBinding
            {
                .Name = name,
                .DescriptorSet = descriptorBinding.set,
                .Binding = descriptorBinding.binding,
                .ShaderStageFlags = static_cast<VkShaderStageFlags>(spvReflectModule.shader_stage),
                .DescriptorBindingType = descriptorBinding.descriptor_type,
                .DescripterType = static_cast<VkDescriptorType>(descriptorBinding.descriptor_type)
            });
    }
    std::sort(m_descriptorBindingList.begin(), m_descriptorBindingList.end(), [](const ShaderDescriptorBinding& a, const ShaderDescriptorBinding& b)
        {
            return a.Binding < b.Binding;
        });
}

void VulkanShader::LoadShaderSpecialConstants(const SpvReflectShaderModule& spvReflectModule)
{
    m_specializationConstantList = GetShaderSpecialConstants(spvReflectModule);
}

ShaderStruct VulkanShader::LoadShaderPipelineStructs(const SpvReflectTypeDescription& shaderInfo)
{
    size_t bufferSize = 0;
    Vector<ShaderVariable> structVariableList = LoadShaderStructVariables(shaderInfo, bufferSize);
    ShaderStruct shaderStruct =
    {
        .Name = String(shaderInfo.type_name),
        .ShaderBufferVariableList = structVariableList
    };
    return shaderStruct;
}

Vector<ShaderVariable> VulkanShader::LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize)
{
    Vector<ShaderVariable> shaderVariables;
    Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(shaderInfo.members, shaderInfo.members + shaderInfo.member_count);
    for (auto& variable : shaderVariableList)
    {
        uint memberSize = 0;
        size_t byteAlignment = 0;
        size_t arraySize = variable.traits.array.dims[0];
        ShaderMemberTypeEnum memberType;
        switch (variable.op)
        {
        case SpvOpTypeInt:
        {
            memberSize = variable.traits.numeric.scalar.width / 8;
            memberType = variable.traits.numeric.scalar.signedness ? kShaderMember_Uint : kShaderMember_Int;
            byteAlignment = 4;
            break;
        }
        case SpvOpTypeFloat:
        {
            memberSize = variable.traits.numeric.scalar.width / 8;
            memberType = kShaderMember_Float;
            byteAlignment = 4;
            break;
        }
        case SpvOpTypeVector:
        {
            memberSize = (variable.traits.numeric.scalar.width / 8) * variable.traits.numeric.vector.component_count;
            switch (variable.traits.numeric.vector.component_count)
            {
            case 2:
                memberType = kShaderMember_Vec2;
                byteAlignment = 8;
                break;
            case 3:
                memberType = kShaderMember_Vec3;
                byteAlignment = 16;
                break;
            case 4:
                memberType = kShaderMember_Vec4;
                byteAlignment = 16;
                break;
            }
            break;
        }
        case SpvOpTypeMatrix:
        {
            uint32 rowCount = variable.traits.numeric.matrix.row_count;
            uint32 colCount = variable.traits.numeric.matrix.column_count;
            memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
            if (rowCount == 2 && colCount == 2)
            {
                memberType = kShaderMember_Mat2;
                byteAlignment = 8;
            }
            else if (rowCount == 3 && colCount == 3)
            {
                memberType = kShaderMember_Mat3;
                byteAlignment = 16;
            }
            else if (rowCount == 4 && colCount == 4)
            {
                memberType = kShaderMember_Mat4;
                byteAlignment = 16;
            }
            else
            {
                std::cerr << "Unsupported matrix size: " << rowCount << "x" << colCount << std::endl;
                byteAlignment = -1;
            }
            break;
        }
        case SpvOpTypeArray:
        {
            uint32 rowCount = variable.traits.numeric.matrix.row_count;
            uint32 colCount = variable.traits.numeric.matrix.column_count;
            memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
            if (rowCount == 2 && colCount == 2)
            {
                memberType = kShaderMember_Mat2;
                byteAlignment = 8;
            }
            else if (rowCount == 3 && colCount == 3)
            {
                memberType = kShaderMember_Mat3;
                byteAlignment = 16;
            }
            else if (rowCount == 4 && colCount == 4)
            {
                memberType = kShaderMember_Mat4;
                byteAlignment = 16;
            }
            else
            {
                std::cerr << "Unsupported matrix size: " << rowCount << "x" << colCount << std::endl;
                byteAlignment = -1;
            }
            break;
        }
        }

        shaderVariables.emplace_back(ShaderVariable
            {
                .Name = String(variable.struct_member_name),
                .Size = arraySize == 0 ? memberSize : memberSize * arraySize,
                .ByteAlignment = byteAlignment,
                .Value = Vector<byte>(),
                .MemberTypeEnum = memberType,
            });
        size_t alignment = byteAlignment;
        returnBufferSize = (returnBufferSize + alignment - 1) & ~(alignment - 1);
        returnBufferSize += memberSize;
    }
    return shaderVariables;
}

void VulkanShader::LoadShader(const Vector<byte>& shaderCode)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderCode.size(),
        .pCode = (const uint32*)shaderCode.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateShaderModule(vulkan.LogicalDevice(), &shaderModuleCreateInfo, nullptr, &m_shaderModule));
}

VkVertexInputRate VulkanShader::LoadVertexInputRate()
{
    Vector<SpvReflectSpecializationConstant> specialConstantResult = SearchShaderSpecialConstants("VertexAttributeLocation");
    if (specialConstantResult.size())
    {
       return *static_cast<VkVertexInputRate*>(specialConstantResult[0].default_value);
    }
    return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
}

Vector<SpvReflectInterfaceVariable> VulkanShader::GetShaderVertexInputVariables(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 inputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&spvReflectModule, &inputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&spvReflectModule, &inputCount, inputs.data()));

    Vector<SpvReflectInterfaceVariable> vertexInputVariableList;
    for (auto inputVertexVariable : inputs)
    {
        vertexInputVariableList.emplace_back(*inputVertexVariable);
    }
    return vertexInputVariableList;
}

Vector<SpvReflectInterfaceVariable> VulkanShader::GetShaderVertexOutputVariables(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&spvReflectModule, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&spvReflectModule, &outputCount, outputs.data()));

    Vector<SpvReflectInterfaceVariable> vertexOutputVariableList;
    for (auto outputVertexVariable : outputs)
    {
        vertexOutputVariableList.emplace_back(*outputVertexVariable);
    }
    return vertexOutputVariableList;
}

Vector<SpvReflectSpecializationConstant> VulkanShader::GetShaderSpecialConstants(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 specializationConstantCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateSpecializationConstants(&spvReflectModule, &specializationConstantCount, nullptr));
    Vector<SpvReflectSpecializationConstant*> specializationConstants(specializationConstantCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateSpecializationConstants(&spvReflectModule, &specializationConstantCount, specializationConstants.data()));

    Vector<SpvReflectSpecializationConstant> specializationConstantList;
    for (auto specializationConstant : specializationConstants)
    {
        specializationConstantList.emplace_back(*specializationConstant);
    }
    return specializationConstantList;
}

Vector<SpvReflectBlockVariable> VulkanShader::GetShaderPushConstants(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 pushConstCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&spvReflectModule, &pushConstCount, nullptr));
    Vector<SpvReflectBlockVariable*> pushConstants(pushConstCount);
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&spvReflectModule, &pushConstCount, pushConstants.data()));

    Vector<SpvReflectBlockVariable> pushConstantVariableList;
    for (auto pushConstantVariable : pushConstants)
    {
        pushConstantVariableList.emplace_back(*pushConstantVariable);
    }
    return pushConstantVariableList;
}

Vector<SpvReflectDescriptorBinding> VulkanShader::GetShaderDescriptorBindings(const SpvReflectShaderModule& spvReflectModule)
{
    uint32 descriptorBindingsCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&spvReflectModule, &descriptorBindingsCount, nullptr));
    Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&spvReflectModule, &descriptorBindingsCount, descriptorSetBindings.data()));

    Vector<SpvReflectDescriptorBinding> descriptorBindingList;
    for (auto descriptorBinding : descriptorSetBindings)
    {
        descriptorBindingList.emplace_back(*descriptorBinding);
    }
    return descriptorBindingList;
}

Vector<SpvReflectSpecializationConstant> VulkanShader::SearchShaderSpecialConstants(const String& searchString)
{
    Vector<SpvReflectSpecializationConstant> results;
    for (auto constant : m_specializationConstantList)
    {
        String nameStr(constant.name);
        if (nameStr.find(searchString) != String::npos)
        {
            results.push_back(constant);
        }
    }
    return results;
}

VkShaderModule VulkanShader::ShaderModule()                                         const { return m_shaderModule; }
VkShaderStageFlagBits VulkanShader::ShaderStages()                                  const { return m_shaderStages; }
ShaderPushConstant VulkanShader::PushConstant()                                     const { return m_pushConstant; }
Vector<VkVertexInputAttributeDescription> VulkanShader::InputVertexAttributeList()  const { return m_inputVertexAttributeList; }
Vector<VkVertexInputAttributeDescription> VulkanShader::OutputVertexAttributeList() const { return m_outputVertexAttributeList; }
Vector<VkVertexInputBindingDescription> VulkanShader::VertexInputBindingList()      const { return m_vertexInputBindingList; }
Vector<ShaderDescriptorBinding> VulkanShader::DescriptorBindingList()               const { return m_descriptorBindingList; }
Vector<SpvReflectSpecializationConstant> VulkanShader::SpecializationConstantList() const { return m_specializationConstantList; }
