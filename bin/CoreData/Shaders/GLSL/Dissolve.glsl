#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

varying vec2 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    discard;
    vTexCoord = iTexCoord;
}

void PS()
{
    vec4 diff = texture2D(sDiffMap, vTexCoord);
    gl_FragColor = diff;
    gl_FragColor = vec4(1);
    discard;
}
