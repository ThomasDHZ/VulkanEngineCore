using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    using GlmSharp;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Text.Json;
    using System.Text.Json.Serialization;
    using System.Threading.Tasks;
    using VulkanEngineCoreCS.Models;

    namespace VulkanEngineCS
    {
        public enum PushConstantKind
        {
            ConstVariable = 0,
            Ref = 1
        }
        public enum TextureVarTypeEnum
        {
            kVar_TextureSize,
        };

        public abstract class PushConstant
        {
            public PushConstantKind PushConstantType { get; set; }
        }

        public class PushConstantConstVariable : PushConstant
        {
            public PushConstantConstVariable()
            {
            }

            public object? Value { get; set; }
            public string Variable { get; set; } = string.Empty;
            public ShaderMemberTypeEnum VariableType { get; set; }
        }

        public class PushConstantRef : PushConstant
        {
            public PushConstantRef()
            {
            }

            public Guid AssetId { get; set; } = Guid.Empty;
            public TextureVarTypeEnum TextureVar { get; set; }
            public object? Value { get; set; }
        }

        public struct PushConstantUpdateRule
        {
            public string Variable { get; set; } = string.Empty;
            public PushConstant? PushConstant { get; set; }

            public PushConstantUpdateRule()
            {
            }
        }

        public class PushConstantConverter : JsonConverter<PushConstant>
        {
            public override PushConstant? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                using var doc = JsonDocument.ParseValue(ref reader);
                var root = doc.RootElement;

                int typeValue = root.GetProperty("PushConstantType").GetInt32();
                var type = (PushConstantKind)typeValue;

                return type switch
                {
                    PushConstantKind.ConstVariable => ReadConst(root),
                    PushConstantKind.Ref => ReadRef(root),
                    _ => throw new JsonException($"Unknown PushConstantType {typeValue}")
                };
            }

            private static PushConstantConstVariable ReadConst(JsonElement root)
            {
                var model = new PushConstantConstVariable
                {
                    PushConstantType = PushConstantKind.ConstVariable,
                    Variable = root.TryGetProperty("Variable", out var v) ? v.GetString() ?? string.Empty : string.Empty,
                    VariableType = (ShaderMemberTypeEnum)root.GetProperty("VariableType").GetInt32()
                };

                if (root.TryGetProperty("Value", out var valueEl))
                    model.Value = ParseVariable(valueEl, model.VariableType);

                return model;
            }

            private static PushConstantRef ReadRef(JsonElement root)
            {
                var assetIdStr = root.GetProperty("AssetId").GetString() ?? string.Empty;
                return new PushConstantRef
                {
                    PushConstantType = PushConstantKind.Ref,
                    AssetId = Guid.TryParse(assetIdStr, out var g) ? g : Guid.Empty,
                    TextureVar = (TextureVarTypeEnum)(root.TryGetProperty("TextureVar", out var tv) ? tv.GetInt32() : 0),
                    Value = root.TryGetProperty("Value", out var v) ? ParseSimpleValue(v) : null
                };
            }

            private static object ParseVariable(JsonElement value, ShaderMemberTypeEnum variableType)
            {
                return variableType switch
                {
                    ShaderMemberTypeEnum.kShaderMember_Int => value[0].GetInt32(),
                    ShaderMemberTypeEnum.kShaderMember_Uint => value[0].GetUInt32(),
                    ShaderMemberTypeEnum.kShaderMember_Float => value[0].GetSingle(),
                    ShaderMemberTypeEnum.kShaderMember_Ivec2 => new ivec2(value[0].GetInt32(), value[1].GetInt32()),
                    ShaderMemberTypeEnum.kShaderMember_Ivec3 => new ivec3(value[0].GetInt32(), value[1].GetInt32(), value[2].GetInt32()),
                    ShaderMemberTypeEnum.kShaderMember_Ivec4 => new ivec4(value[0].GetInt32(), value[1].GetInt32(), value[2].GetInt32(), value[3].GetInt32()),
                    ShaderMemberTypeEnum.kShaderMember_Vec2 => new vec2(value[0].GetSingle(), value[1].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_Vec3 => new vec3(value[0].GetSingle(), value[1].GetSingle(), value[2].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_Vec4 => new vec4(value[0].GetSingle(), value[1].GetSingle(), value[2].GetSingle(), value[3].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_Mat2 => new mat2(value[0].GetSingle(), value[1].GetSingle(),
                                                                        value[2].GetSingle(), value[3].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_Mat3 => new mat3(value[0].GetSingle(), value[1].GetSingle(), value[2].GetSingle(),
                                                                        value[3].GetSingle(), value[4].GetSingle(), value[5].GetSingle(),
                                                                        value[6].GetSingle(), value[7].GetSingle(), value[8].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_Mat4 => new mat4(value[0].GetSingle(), value[1].GetSingle(), value[2].GetSingle(), value[3].GetSingle(),
                                                                        value[4].GetSingle(), value[5].GetSingle(), value[6].GetSingle(), value[7].GetSingle(),
                                                                        value[8].GetSingle(), value[9].GetSingle(), value[10].GetSingle(), value[11].GetSingle(),
                                                                        value[12].GetSingle(), value[13].GetSingle(), value[14].GetSingle(), value[15].GetSingle()),
                    ShaderMemberTypeEnum.kShaderMember_bool => value[0].GetBoolean(),
                    _ => throw new JsonException($"Unknown VariableType {variableType}")
                };
            }

            private static object ParseSimpleValue(JsonElement value) => value.ValueKind switch
            {
                JsonValueKind.Number when value.TryGetInt32(out var i) => i,
                JsonValueKind.Number => value.GetSingle(),
                JsonValueKind.True or JsonValueKind.False => value.GetBoolean(),
                JsonValueKind.String => value.GetString()!,
                _ => value.Clone()
            };

            public override void Write(Utf8JsonWriter writer, PushConstant value, JsonSerializerOptions options)
            {
                writer.WriteStartObject();
                writer.WriteNumber("PushConstantType", (int)value.PushConstantType);

                switch (value)
                {
                    case PushConstantConstVariable c:
                        writer.WriteString("Variable", c.Variable);
                        writer.WriteNumber("VariableType", (int)c.VariableType);
                        if (c.Value != null)
                        {
                            writer.WritePropertyName("Value");
                            WriteVariable(writer, c.Value, c.VariableType);
                        }
                        break;

                    case PushConstantRef r:
                        writer.WriteString("AssetId", r.AssetId.ToString());
                        writer.WriteNumber("TextureVar", (int)r.TextureVar);
                        if (r.Value != null)
                        {
                            writer.WritePropertyName("Value");
                            WriteSimpleValue(writer, r.Value);
                        }
                        break;
                }

                writer.WriteEndObject();
            }

            private static void WriteVariable(Utf8JsonWriter writer, object value, ShaderMemberTypeEnum variableType)
            {
                if (variableType is ShaderMemberTypeEnum.kShaderMember_Int or
                                    ShaderMemberTypeEnum.kShaderMember_Uint or
                                    ShaderMemberTypeEnum.kShaderMember_Float or
                                    ShaderMemberTypeEnum.kShaderMember_bool)
                {
                    writer.WriteStartArray();
                    WriteSimpleValue(writer, value);
                    writer.WriteEndArray();
                    return;
                }

                switch (value)
                {
                    case ivec2 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y);
                        writer.WriteEndArray();
                        break;
                    case ivec3 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y); writer.WriteNumberValue(v.z);
                        writer.WriteEndArray();
                        break;
                    case ivec4 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y); writer.WriteNumberValue(v.z); writer.WriteNumberValue(v.w);
                        writer.WriteEndArray();
                        break;
                    case vec2 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y);
                        writer.WriteEndArray();
                        break;
                    case vec3 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y); writer.WriteNumberValue(v.z);
                        writer.WriteEndArray();
                        break;
                    case vec4 v:
                        writer.WriteStartArray();
                        writer.WriteNumberValue(v.x); writer.WriteNumberValue(v.y); writer.WriteNumberValue(v.z); writer.WriteNumberValue(v.w);
                        writer.WriteEndArray();
                        break;
                    case mat2 m:
                        writer.WriteStartArray();
                        for (int i = 0; i < 4; i++) writer.WriteNumberValue(m[i]);
                        writer.WriteEndArray();
                        break;
                    case mat3 m:
                        writer.WriteStartArray();
                        for (int i = 0; i < 9; i++) writer.WriteNumberValue(m[i]);
                        writer.WriteEndArray();
                        break;
                    case mat4 m:
                        writer.WriteStartArray();
                        for (int i = 0; i < 16; i++) writer.WriteNumberValue(m[i]);
                        writer.WriteEndArray();
                        break;
                    default:
                        JsonSerializer.Serialize(writer, value);
                        break;
                }
            }

            private static void WriteSimpleValue(Utf8JsonWriter writer, object value)
            {
                switch (value)
                {
                    case int i: writer.WriteNumberValue(i); break;
                    case uint u: writer.WriteNumberValue(u); break;
                    case float f: writer.WriteNumberValue(f); break;
                    case bool b: writer.WriteBooleanValue(b); break;
                    case string s: writer.WriteStringValue(s); break;
                    default: JsonSerializer.Serialize(writer, value); break;
                }
            }
        }
    }

}
