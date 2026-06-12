#pragma once
#include "Platform.h"

enum ShaderMemberType
{
    shaderUnknown,
    shaderInt,
    shaderUint,
    shaderFloat,
    shaderIvec2,
    shaderIvec3,
    shaderIvec4,
    shaderVec2,
    shaderVec3,
    shaderVec4,
    shaderMat2,
    shaderMat3,
    shaderMat4,
    shaderbool
};

enum DescriptorBindingPropertiesEnum
{
    kMeshPropertiesDescriptor,
    kTextureDescriptor,
    kMaterialDescriptor,
    kDirectionalLightDescriptor,
    kPointLightDescriptor,
    kSpotLightDescriptor,
    kVertexDescsriptor,
    kIndexDescriptor,
    kTransformDescriptor,
    kSkyBoxDescriptor,
    kIrradianceMapDescriptor,
    kPrefilterMapDescriptor,
    kSubpassInputDescriptor,
    kBRDFMapDescriptor,
    kEnvironmentMapDescriptor,
    kBindlessDataDescriptor,
    kTexture3DDescriptor,
    kSceneDataDescriptor
};

struct ShaderVariable
{
    String                          Name;
    size_t                          Size = 0;
    size_t                          ByteAlignment = 0;
    Vector<byte>                    Value;
    ShaderMemberType                MemberTypeEnum = shaderUnknown;
    bool                            ConstVariable = false;
};

struct ShaderStruct
{
    String                          Name;
    size_t			                ShaderBufferSize = 0;
    Vector<ShaderVariable>          ShaderBufferVariableList;
    int                             ShaderStructBufferId;
    Vector<byte>                    ShaderStructBuffer;
};

struct ShaderDescriptorSet
{
    String                          Name;
    uint32                          Binding;
    VkDescriptorType                DescripterType;
    Vector<ShaderStruct>            ShaderStructList;
};

struct ShaderDescriptorBinding
{
    String                          Name;
    uint32                          DescriptorSet = UINT32_MAX;
    uint32                          Binding = UINT32_MAX;
    size_t                          DescriptorCount;
    VkShaderStageFlags              ShaderStageFlags;
    DescriptorBindingPropertiesEnum DescriptorBindingType;
    VkDescriptorType                DescripterType;
    Vector<VkDescriptorImageInfo>   DescriptorImageInfo;
    Vector<VkDescriptorBufferInfo>  DescriptorBufferInfo;
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

struct ShaderPipelineData
{
    Vector<String>                              ShaderList;
    Vector<ShaderDescriptorBinding>             DescriptorBindingsList;
    Vector<ShaderStruct>                        ShaderStructList;
    Vector<VkVertexInputBindingDescription>     VertexInputBindingList;
    Vector<VkVertexInputAttributeDescription>   VertexInputAttributeList;
    Vector<ShaderPushConstant>                  PushConstantList;
};