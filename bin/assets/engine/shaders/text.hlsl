
#include "common.hlsl"

Texture2D<float3> AtlasTexture : register(t0);
SamplerState DefaultSampler : register(s0);

cbuffer BufferData : register(b0)
{
    float4x4 Projection;
    float PixelRange;
    float FontSize;
    float AtlasSize;
    float _Padding;
};

struct InstanceBuffer
{
    float4 Color;
    float2 UvOffset;
    float2 UvScale;
    float2 Offset;
    float2 Position;
    float FontScale;
    float BlendMode;
    float2 ScreenPosition;
};
StructuredBuffer<InstanceBuffer> InstanceData : register(t1);

void VSMain(in VertexInput input, out PixelInput output, uint instance_ID : SV_InstanceID)
{
    const float4 color = InstanceData[instance_ID].Color;
    const float2 uv_scale = InstanceData[instance_ID].UvScale;
    const float2 uv_offset = InstanceData[instance_ID].UvOffset;
    const float2 offset = InstanceData[instance_ID].Offset;
    const float2 position = InstanceData[instance_ID].Position;
    const float font_scale = InstanceData[instance_ID].FontScale;
    const float blend_mode = InstanceData[instance_ID].BlendMode;
    const float2 screen_position = InstanceData[instance_ID].ScreenPosition;

    float2 p = input.position.xy;
    p *= uv_scale;
    p += offset;
    p *= AtlasSize / FontSize;
    p += position;

    p *= FontSize * font_scale;

    p += screen_position;

    output.position = mul(Projection, float4(p, 0.0, 1.0));
    output.uv = input.uv * uv_scale + uv_offset;
    output.color = float4(color.rgb * color.a, color.a * blend_mode);
}

float screenPxRange(float2 texcoord)
{
    const float2 unitRange = PixelRange / AtlasSize;
    const float2 screenTexSize = 1.0 / fwidth(texcoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void PSMain(in PixelInput input, out PixelOutput output)
{
    const float3 msd = AtlasTexture.Sample(DefaultSampler, input.uv).rgb;
    const float sd = max(min(msd.r, msd.g), min(max(msd.r, msd.g), msd.b));
    const float screenPxDistance = screenPxRange(input.uv) * (sd - 0.5);
    const float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    output.color.rgba = opacity;
    output.color *= input.color;
}
