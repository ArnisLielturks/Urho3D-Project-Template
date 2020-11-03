#include "Uniforms.hlsl"
#include "Transform.hlsl"
#include "Samplers.hlsl"
#include "ScreenPos.hlsl"
#include "PostProcess.hlsl"

#ifndef D3D11

// D3D9 uniforms
uniform float cGamma;

#else

// D3D11 constant buffers
#ifdef COMPILEPS
cbuffer CustomPS : register(b6)
{
    float cGamma;
}
#endif

#endif


void VS(float4 iPos : POSITION,
    out float2 oScreenPos : TEXCOORD0,
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
    oScreenPos = GetScreenPosPreDiv(oPos);
}

void PS(float2 iScreenPos : TEXCOORD0,
    out float4 oColor : OUTCOLOR0)
{
    float3 color = Sample2D(DiffMap, iScreenPos).rgb;
    oColor = float4(ToGamma(color, cGamma), 1.0);
}
