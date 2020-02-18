#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying float vDepth;

#ifdef COMPILEPS
uniform vec2 cSSAOHInvSize;
uniform vec2 cBlurHInvSize;
uniform vec2 cBlurVInvSize;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
}

    #ifdef COMPILEPS

float AbsoluteDepth(float normalDepth) {
    float clipLength = cFarClipPS - cNearClipPS;
    return cNearClipPS + clipLength * normalDepth;
}

vec3 normal_from_depth(float depth, vec2 texcoords) {
    const vec2 offset1 = vec2(0.0, 1.0);
    const vec2 offset2 = vec2(1.0, 0.0);

    float depth1 = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord.xy + offset1 * cSSAOHInvSize.x).rgb));
    float depth2 = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord.xy + offset2 * cSSAOHInvSize.y).rgb));

    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);

    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;

    return normalize(normal);
}

vec2 saturateVec2(vec2 value) {
    return vec2(clamp(value.x, 0.0, 1.0), clamp(value.y, 0.0, 1.0));
}

vec3 saturateVec3(vec3 value) {
    return vec3(clamp(value.x, 0.0, 1.0), clamp(value.y, 0.0, 1.0), clamp(value.z, 0.0, 1.0));
}
    #endif

void PS()
{
    float originalDepth = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord).rgb));

    #ifdef OCCLUDE

    const float total_strength = 1.0;
    const float base = 0.2;

    const float area = 0.75;
    const float falloff = 0.001;
    const float noiseFactor = 7.0;
    const float radius = 0.2;

    const int samples = 16;
    vec3 sample_sphere[samples] = vec3[samples](
    vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
    vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
    vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
    vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
    vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
    vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
    vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
    vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
    );

    vec3 random = normalize(texture2D(sDiffMap, vTexCoord * noiseFactor).rgb);

    vec3 position = vec3(vTexCoord, 0.0);
    vec3 normal = normal_from_depth(originalDepth, vTexCoord);

    float radius_depth = radius / originalDepth;
    float occlusion = 0.0;

    for (int i = 0; i < samples; i++) {
        vec3 ray = radius_depth * reflect(sample_sphere[i], random);
        vec3 hemi_ray = position + sign(dot(ray,normal)) * ray;

        float occ_depth = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, hemi_ray.xy).rgb));
        float difference = originalDepth - occ_depth;

        occlusion += step(falloff, difference) * (1.0 - smoothstep(falloff, area, difference));
    }

    float ao = 1.0 - total_strength * occlusion * (1.0 / samples);

    gl_FragColor.r = clamp(ao + base, 0.0, 1.0);
    gl_FragColor.g = gl_FragColor.r;
    gl_FragColor.b = gl_FragColor.r;
    #endif

    const int blurRadius = 16;
    float blurWeights[16] = float[16](
    100, 95, 90, 85,
    80, 75, 70, 65,
    60, 55, 50, 45,
    40, 35, 30, 25
    );
    float total = 0;
    for (int i = 0; i < blurRadius; i++) {
        total += blurWeights[i];
    }

    // Depth check to avoid bluring out the area around the objects
    float maxDepth = 0.2 * originalDepth;

    #if defined(BLURV) || defined(BLURH)
    vec2 offset = vec2(0.0, 1.0) * cBlurVInvSize.y;
    float up = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord + offset).rgb));
    float down = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord - offset).rgb));

    if (
    abs(originalDepth - up) >= maxDepth
    || abs(originalDepth - down) >= maxDepth
    ) {
        gl_FragColor = texture2D(sDiffMap, vec2(vTexCoord.x, vTexCoord.y));
        return;
    }

    offset = vec2(1.0, 0.0) * cBlurHInvSize.x;
    float right = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord + offset).rgb));
    float left = AbsoluteDepth(DecodeDepth(texture2D(sDepthBuffer, vTexCoord - offset).rgb));

    if (
    abs(originalDepth - right) >= maxDepth
    || abs(originalDepth - left) >= maxDepth
    ) {
        gl_FragColor = texture2D(sDiffMap, vec2(vTexCoord.x, vTexCoord.y));
        return;
    }
        #endif

        #ifdef BLURV
    vec4 sum = vec4(0.0);
    sum += texture2D(sDiffMap, vec2(vTexCoord.x, vTexCoord.y)) * (blurWeights[0] / total);
    for (int i = 1; i < blurRadius; i++) {
        float weight =  blurWeights[i] / total / 2.0;
        sum += texture2D(sNormalMap, vec2(vTexCoord.x, vTexCoord.y + i * cBlurVInvSize.y)) * weight;
        sum += texture2D(sNormalMap, vec2(vTexCoord.x, vTexCoord.y - i * cBlurVInvSize.y)) * weight;
    }
    gl_FragColor = sum;
    #endif

    #ifdef BLURH
    vec4 sum = vec4(0.0);
    sum += texture2D(sDiffMap, vec2(vTexCoord.x, vTexCoord.y)) * (blurWeights[0] / total);
    for (int i = 1; i < blurRadius; i++) {
        float weight =  blurWeights[i] / total / 2.0;
        sum += texture2D(sNormalMap, vec2(vTexCoord.x + i * cBlurHInvSize.x, vTexCoord.y)) * weight;
        sum += texture2D(sNormalMap, vec2(vTexCoord.x - i * cBlurHInvSize.x, vTexCoord.y)) * weight;
    }
    gl_FragColor = sum;
    #endif

    #ifdef OUTPUT
    gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb * texture2D(sDepthBuffer, vTexCoord).rgb;
    #endif
}
