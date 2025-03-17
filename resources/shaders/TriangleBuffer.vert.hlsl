cbuffer UniformBlock : register(b0, space1) {
    float4 DynColor : packoffset(c0);
};

struct Input
{
    uint vertex_index : SV_VertexID;
    float3 Position : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.Color = input.Color;
    if(input.vertex_index == 0){
        output.Color.x = DynColor.x;
    }
    output.Position = float4(input.Position, 1.0f);
    return output;
}