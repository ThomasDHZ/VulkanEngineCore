#pragma once
#include "Platform.h"
#include "ShaderEnums.h"
#include <variant>

using PushConstantVariable = std::variant<int, uint32, float, bool, ivec2, ivec3, ivec4, vec2, vec3, vec4, mat2, mat3, mat4>;
struct ShaderVariable
{
    String                          Name;
    PushConstantVariable            Value;
};

struct ShaderStruct
{
    String                          Name;
    Vector<ShaderVariable>          ShaderBufferVariableList;
};

struct ShaderDescriptorSet
{
    String                          Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStruct>            ShaderStructList;
};

struct ShaderPushConstant
{
    String                          PushConstantName;
    size_t			                PushConstantSize = 0;
    VkShaderStageFlags              ShaderStageFlags;
    Vector<ShaderVariable>          PushConstantVariableList;
    Vector<byte>                    PushConstantBuffer;
    bool			                GlobalPushConstant = false;
};

struct ShaderDescriptorBinding
{
    String                          Name;
    uint32                          DescriptorSet = UINT32_MAX;
    uint32                          Binding = UINT32_MAX;
    size_t                          DescriptorCount;
    VkShaderStageFlags              ShaderStageFlags;
    SpvReflectDescriptorType        DescriptorBindingType;
    VkDescriptorType                DescripterType;
    Vector<VkDescriptorImageInfo>   DescriptorImageInfo;
    Vector<VkDescriptorBufferInfo>  DescriptorBufferInfo;
};
