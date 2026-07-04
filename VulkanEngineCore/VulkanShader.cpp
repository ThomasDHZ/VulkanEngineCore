#include "VulkanShader.h"

VulkanShader::VulkanShader()
{
}

VulkanShader::VulkanShader(Vector<byte> shaderData, VkShaderStageFlagBits shaderStages)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderData.size(),
        .pCode = (const uint32*)shaderData.data()
    };
    m_shaderStages = shaderStages;
    VULKAN_THROW_IF_FAIL(vkCreateShaderModule(vulkan.LogicalDevice(), &shaderModuleCreateInfo, nullptr, &m_shaderModule));
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

void VulkanShader::LoadShaderVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList)
{
    uint32 offset = 0;

    Vector<SpvReflectInterfaceVariable> inputs = GetShaderVertexInputVariables(module);
    inputs.erase(std::remove_if(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable* input) { return input->built_in != -1; }), inputs.end());
    inputs.shrink_to_fit();

    std::sort(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b)
        {
            return a->location < b->location;
        });

    VkVertexInputRate inputRate = LoadVertexInputRate();
    for (int x = 0; x < inputs.size(); x++)
    {
        uint32 binding = 0;
        switch (inputs[x].type_description->op)
        {
            case SpvOpTypeInt:
            {
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
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
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
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
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
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
                    vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
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
        vertexInputBindingList.emplace_back(VkVertexInputBindingDescription
            {
               .binding = vertexInputAttributeList[x].binding,
               .stride = offset,
               .inputRate = static_cast<VkVertexInputRate>(inputRate)
            });
        if (inputs.size() == x) offset = 0;
    }
}

Vector<SpvReflectInterfaceVariable*> VulkanShader::LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module)
{
    uint32 outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data()));
    return outputs;
}

void VulkanShader::LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList)
{
    Vector<SpvReflectBlockVariable> pushConstants = GetShaderPushConstants(module);
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
       .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
       .PushConstantVariableList = shaderStructVariableList,
       .PushConstantBuffer = Vector<byte>(bufferSize, 0x00)
    };
}


void VulkanShader::LoadShaderDescriptorBindings(const SpvReflectShaderModule& module)
{
    Vector<SpvReflectDescriptorBinding> descriptorSetBindings = GetShaderDescriptorBindings(module);
    for (auto& descriptorBinding : descriptorSetBindings)
    {
        String name(descriptorBinding.name);
        m_descriptorBindingList.emplace_back(ShaderDescriptorBinding
            {
                .Name = name,
                .DescriptorSet = descriptorBinding.set,
                .Binding = descriptorBinding.binding,
                .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                .DescriptorBindingType = descriptorBinding.descriptor_type,
                .DescripterType = static_cast<VkDescriptorType>(descriptorBinding.descriptor_type)
            });
    }
    std::sort(m_descriptorBindingList.begin(), m_descriptorBindingList.end(), [](const ShaderDescriptorBinding& a, const ShaderDescriptorBinding& b)
        {
            return a.Binding < b.Binding;
        });
}

void VulkanShader::LoadShaderSpecialConstants(const SpvReflectShaderModule& module)
{
    m_specializationConstantList = GetShaderSpecialConstants(module);
}

ShaderStruct VulkanShader::LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo)
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

VkVertexInputRate VulkanShader::LoadVertexInputRate()
{
    Vector<SpvReflectSpecializationConstant> specialConstantResult = SearchShaderSpecialConstants("VertexAttributeLocation");
    if (specialConstantResult.size())
    {
       return *static_cast<VkVertexInputRate*>(specialConstantResult[0].default_value);
    }
    return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
}

Vector<SpvReflectInterfaceVariable> VulkanShader::GetShaderVertexInputVariables(const SpvReflectShaderModule& shaderModule)
{
    uint32 inputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&shaderModule, &inputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&shaderModule, &inputCount, inputs.data()));

    Vector<SpvReflectInterfaceVariable> vertexInputVariableList;
    for (auto inputVertexVariable : inputs)
    {
        vertexInputVariableList.emplace_back(*inputVertexVariable);
    }
    return vertexInputVariableList;
}

Vector<SpvReflectInterfaceVariable> VulkanShader::GetShaderVertexOutputVariables(const SpvReflectShaderModule& shaderModule)
{
    uint32 outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&shaderModule, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&shaderModule, &outputCount, outputs.data()));

    Vector<SpvReflectInterfaceVariable> vertexOutputVariableList;
    for (auto outputVertexVariable : outputs)
    {
        vertexOutputVariableList.emplace_back(*outputVertexVariable);
    }
    return vertexOutputVariableList;
}

Vector<SpvReflectSpecializationConstant> VulkanShader::GetShaderSpecialConstants(const SpvReflectShaderModule& shaderModule)
{
    uint32 specializationConstantCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateSpecializationConstants(&shaderModule, &specializationConstantCount, nullptr));
    Vector<SpvReflectSpecializationConstant*> specializationConstants(specializationConstantCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateSpecializationConstants(&shaderModule, &specializationConstantCount, specializationConstants.data()));

    Vector<SpvReflectSpecializationConstant> specializationConstantList;
    for (auto specializationConstant : specializationConstants)
    {
        specializationConstantList.emplace_back(*specializationConstant);
    }
    return specializationConstantList;
}

Vector<SpvReflectBlockVariable> VulkanShader::GetShaderPushConstants(const SpvReflectShaderModule& shaderModule)
{
    uint32 pushConstCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&shaderModule, &pushConstCount, nullptr));
    Vector<SpvReflectBlockVariable*> pushConstants(pushConstCount);
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&shaderModule, &pushConstCount, pushConstants.data()));

    Vector<SpvReflectBlockVariable> pushConstantVariableList;
    for (auto pushConstantVariable : pushConstants)
    {
        pushConstantVariableList.emplace_back(*pushConstantVariable);
    }
    return pushConstantVariableList;
}

Vector<SpvReflectDescriptorBinding> VulkanShader::GetShaderDescriptorBindings(const SpvReflectShaderModule& shaderModule)
{
    uint32 descriptorBindingsCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&shaderModule, &descriptorBindingsCount, nullptr));
    Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&shaderModule, &descriptorBindingsCount, descriptorSetBindings.data()));

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
