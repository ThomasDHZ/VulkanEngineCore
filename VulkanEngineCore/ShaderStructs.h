#pragma once
#include "Platform.h"
#include "ShaderEnums.h"

struct ShaderVariable
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    Vector<byte>                    Value;
    ShaderMemberTypeEnum            MemberTypeEnum = kShaderMember_Undefined;
    bool                            ConstVariable = false;
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
