#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vTexCoord;
varying float vDepth;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = iTexCoord1;
    vDepth = GetDepth(gl_Position);
}

#ifdef COMPILEPS
float AbsoluteDepth(float normalDepth) {
    float clipLength = cFarClipPS - cNearClipPS;
    return cNearClipPS + clipLength * normalDepth;
}
#endif

void PS()
{
    // Screen-door transparency: Discard pixel if below threshold.
    // (Note: pos is pixel position.)
    mat4 thresholdMatrix = mat4(
        1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
        13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
        4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
        16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
    );
    int x = int(mod(vTexCoord.x * 1280, 4));
    int y = int(mod(vTexCoord.y * 720, 4));
    float z = AbsoluteDepth(vDepth);
    float threshold = 10.0;
    if (z / threshold <= thresholdMatrix[x][y]) {
        discard;
    }
}
