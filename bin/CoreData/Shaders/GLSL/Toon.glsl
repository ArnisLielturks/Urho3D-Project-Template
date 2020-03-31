#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
}

void PS()
{
    float levels = 10;
    vec4 fragColor = texture2D(sDiffMap, vTexCoord);
    float greyscale = max(fragColor.r, max(fragColor.g, fragColor.b));
    float lower     = floor(greyscale * levels) / levels;
    float lowerDiff = abs(greyscale - lower);
    float upper     = ceil(greyscale * levels) / levels;
    float upperDiff = abs(upper - greyscale);
    float level      = lowerDiff <= upperDiff ? lower : upper;
    float adjustment = level / greyscale;
    gl_FragColor = fragColor * adjustment;
}
