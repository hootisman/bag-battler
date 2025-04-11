cbuffer UniformBlock : register(b0, space1) {
    float4x4 Translation: packoffset(c0);
};

struct Input
{
    float3 Position : TEXCOORD0;
    float3 Normal: TEXCOORD1;
    float2 TexCoords: TEXCOORD2;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.Color = float4(0.0f, 1.0f,0.0f, 1.0f);
    output.Position = mul(Translation, float4(input.Position, 1.0f));
    return output;
}