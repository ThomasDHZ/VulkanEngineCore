#pragma once
#include "Platform.h"
#include <variant>

using PushConstantVariable = std::variant<int, int64, uint32, uint64, float, double, bool, ivec2, ivec3, ivec4, vec2, vec3, vec4, mat2, mat3, mat4>;
enum ShaderMemberTypeEnum
{
    kShaderMember_Int,
    kShaderMember_Int64,
    kShaderMember_Uint,
    kShaderMember_Uint64,   
    kShaderMember_Float,
    kShaderMember_Double,
    kShaderMember_bool,
    kShaderMember_Ivec2,
    kShaderMember_Ivec3,
    kShaderMember_Ivec4,
    kShaderMember_Vec2,
    kShaderMember_Vec3,
    kShaderMember_Vec4,
    kShaderMember_Mat2,
    kShaderMember_Mat3,
    kShaderMember_Mat4,
    kShaderMember_Undefined
};

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
