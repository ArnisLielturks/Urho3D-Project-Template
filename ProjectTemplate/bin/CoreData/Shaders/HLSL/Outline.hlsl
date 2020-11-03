#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Transform.hlsl"
#include "ScreenPos.hlsl"
#include "PostProcess.hlsl"

uniform float2 vScreenPos;

uniform float4 cOutlineColor;
uniform float2 cOutlineBlurredMaskHInvSize;


void VS(float4 iPos : POSITION,
    out float2 oTexCoord : TEXCOORD0,
    out float2 oScreenPos : TEXCOORD1,
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
    oTexCoord = GetQuadTexCoord(oPos);
    oScreenPos = GetScreenPosPreDiv(oPos);
}

void PS(
    float3 iTexCoord : TEXCOORD0,
    float2 iScreenPos : TEXCOORD1,
    out float4 oColor : OUTCOLOR0)
{
    #ifdef MASK
    oColor = float4(cOutlineColor.rgb, 1);
    #endif

    #ifdef BLURH
        float4 rgba = Sample2D(DiffMap, iScreenPos + float2(0.0, 0.0) * cOutlineBlurredMaskHInvSize);
                  + Sample2D(DiffMap, iScreenPos + float2(-1.0, 0.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(1.0, 0.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(-2.0, 0.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(2.0, 0.0) * cOutlineBlurredMaskHInvSize);
        oColor = rgba * 0.2;
    #endif

    #ifdef BLURV
        float4 rgba = Sample2D(DiffMap, iScreenPos + float2(0.0, 0.0) * cOutlineBlurredMaskHInvSize);
                  + Sample2D(DiffMap, iScreenPos + float2(0.0, -1.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(0.0, 1.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(0.0, -2.0) * cOutlineBlurredMaskHInvSize)
                  + Sample2D(DiffMap, iScreenPos + float2(0.0, 2.0) * cOutlineBlurredMaskHInvSize);
        oColor = rgba * 0.2;
    #endif


    #ifdef OUTPUT
        float4 blurredMask = Sample2D(DiffMap, iScreenPos);
        float4 mask = Sample2D(NormalMap, iScreenPos);
        float4 viewport = Sample2D(SpecMap, iScreenPos);
        blurredMask = clamp(blurredMask - mask.a, 0.0, 1.0);
        blurredMask *= 200.0;
        oColor = viewport * (1.0 - blurredMask.a) + blurredMask;
    #endif


}