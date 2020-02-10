#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;
varying vec4 vWorldPos;
varying float v_depth;

#ifdef COMPILEPS
uniform vec2 cBlurDir;
uniform float cBlurRadius;
uniform float cBlurSigma;
uniform vec2 cBlurHInvSize;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
    v_depth = -(inverse(cViewInv * cViewProj) * gl_Position).z;
}

#ifdef COMPILEPS
vec3 normal_from_depth(float depth, vec2 texcoords) {

    const vec2 offset1 = vec2(0.0,0.001);
    const vec2 offset2 = vec2(0.001,0.0);

//    float depth1 = texture2D(sDiffMap, texcoords + offset1).r;
//    float depth2 = texture2D(sDiffMap, texcoords + offset2).r;
    float depth1 = DecodeDepth(texture2D(sDepthBuffer, vScreenPos).rgb);
    float depth2 = DecodeDepth(texture2D(sDepthBuffer, vScreenPos).rgb);

    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);

    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;

    return normalize(normal);
}

float saturateFloat(float value) {
    return clamp(value, 0.0, 1.0);
}

vec2 saturateVec2(vec2 value) {
    return vec2(saturateFloat(value.x), saturateFloat(value.y));
}

vec3 saturateVec3(vec3 value) {
    return vec3(saturateFloat(value.x), saturateFloat(value.y), saturateFloat(value.z));
}
#endif

void PS()
{
#ifdef OCCLUDE
    #ifdef BLUR3
        gl_FragColor = GaussianBlur(3, cBlurDir, cBlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR5
        gl_FragColor = GaussianBlur(5, cBlurDir, cBlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR7
        gl_FragColor = GaussianBlur(7, cBlurDir, cBlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

    #ifdef BLUR9
        gl_FragColor = GaussianBlur(9, cBlurDir, cBlurHInvSize * cBlurRadius, cBlurSigma, sDiffMap, vTexCoord);
    #endif

//    gl_FragColor = texture2D(sDiffMap, vTexCoord.xy);

//    float depth = DecodeDepth(texture2D(sDepthBuffer, vScreenPos).rgb);
//    if (depth > 0.005 && depth < 0.01) {
//        gl_FragColor.x = 0.0;
//        gl_FragColor.y = 0.0;
//        gl_FragColor.z = 0.0;
//    }

    const float total_strength = 1.0;
    const float base = 0.2;

    const float area = 0.075;
    const float falloff = 0.000001;

    const float radius = 0.0002;

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

    vec3 random = normalize( texture2D(sDepthBuffer, vTexCoord.xy * 0.4).rgb );

//    float depth = texture2D(sDiffMap, vTexCoord.xy).r;
    float depth = DecodeDepth(texture2D(sDepthBuffer, vScreenPos).rgb);
//
    vec3 position = vec3(vTexCoord.xy, depth);
    vec3 normal = normal_from_depth(depth, vTexCoord.xy);

    float radius_depth = radius/depth;
    float occlusion = 0.0;
    for (int i=0; i < samples; i++) {

        vec3 ray = radius_depth * reflect(sample_sphere[i], random);
        vec3 hemi_ray = position + sign(dot(ray,normal)) * ray;

        float occ_depth = texture2D(sDepthBuffer, saturateVec2(hemi_ray.xy)).r;
        float difference = depth - occ_depth;

        occlusion += step(falloff, difference) * (1.0-smoothstep(falloff, area, difference));
    }
//
    float ao = 1.0 - total_strength * occlusion * (1.0 / samples);
    gl_FragColor.r = saturateFloat(ao + base);
    gl_FragColor.g = gl_FragColor.r;
    gl_FragColor.b = gl_FragColor.r;
#else
    gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord.xy).rgb * texture2D(sDepthBuffer, vTexCoord.xy).rgb;
#endif

}
