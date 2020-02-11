#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform vec2 cBlurDir;
uniform float cBlurRadius;
uniform float cBlurSigma;
uniform int cScreenWidth;
uniform int cScreenHeight;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

#ifdef COMPILEPS
vec3 normal_from_depth(float depth, vec2 texcoords) {

    const vec2 offset1 = vec2(0.0, 0.01);
    const vec2 offset2 = vec2(0.01, 0.0);

    float depth1 = DecodeDepth(texture2D(sDepthBuffer, vTexCoord + offset1).rgb);
    float depth2 = DecodeDepth(texture2D(sDepthBuffer, vTexCoord + offset2).rgb);

    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);

    vec3 normal = cross(p1, p2);
    normal.z = -normal.z;

    return normalize(normal);
}

vec2 saturateVec2(vec2 value) {
    return vec2(clamp(0.0, 1.0, value.x), clamp(0.0, 1.0, value.y));
}

vec3 saturateVec3(vec3 value) {
    return vec3(clamp(0.0, 1.0, value.x), clamp(0.0, 1.0, value.y), clamp(0.0, 1.0, value.z));
}
#endif

void PS()
{
    float textureSize = 128;
#ifdef OCCLUDE

    const float total_strength = 1.0;
    const float base = 0.2;

    const float area = 0.75;
    const float falloff = 0.000001;

    vec2 noiseFactor = vec2(cScreenWidth / textureSize, cScreenHeight / textureSize);

    const float radius = 0.0006;

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

    for (int i = 0; i < samples; i++) {
//        sample_sphere[i].x = sample_sphere[i].x * 0.001;
//        sample_sphere[i].y = sample_sphere[i].x * 0.001;
//        sample_sphere[i].z = sample_sphere[i].x * 0.001;
    }

    vec3 random = normalize(texture2D(sDiffMap, vTexCoord * noiseFactor).rgb);
//    vec3 random = vec3(Noise(vTexCoord), Noise(vTexCoord * noiseFactor.x));
    random.z = sign(random.z);

    float depth = DecodeDepth(texture2D(sDepthBuffer, vTexCoord).rgb);

    vec3 position = vec3(vTexCoord, depth);
    vec3 normal = normal_from_depth(depth, vTexCoord);

    float radius_depth = radius / depth;
    float occlusion = 0.0;

    for (int i = 0; i < samples; i++) {

        vec3 ray = radius_depth * reflect(sample_sphere[i], random);
        vec3 hemi_ray = position + sign(dot(ray, normal)) * ray;

        float occ_depth = texture2D(sDepthBuffer, saturateVec2(hemi_ray.xy)).r;
        float difference = depth - occ_depth;

        occlusion += step(falloff, difference) * (1.0 - smoothstep(falloff, area, difference));
    }

    float ao = 1.0 - total_strength * occlusion * (1.0 / samples);

    gl_FragColor.r = clamp(0.0, 1.0, ao + base);
    gl_FragColor.g = gl_FragColor.r;
    gl_FragColor.b = gl_FragColor.r;

#endif

    int blurRadius = 8;
#ifdef BLURH
//    gl_FragColor = GaussianBlur(9, vec2(1.0, 0.0), vec2(1.0, 0.0) * 0.005, 2.0, sDiffMap, vTexCoord);
//    gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb;
    float result = 0.0;
    for (int x = -blurRadius; x < blurRadius; ++x)
    {
        vec2 offset = vec2(float(x) * 0.0001, 0.0) * blurRadius;
        result += texture2D(sDiffMap, vTexCoord + offset).r;
    }

    result = result / float(blurRadius * 2);
    gl_FragColor.r = result;
    gl_FragColor.g = result;
    gl_FragColor.b = result;
#endif

#ifdef BLURV
//        gl_FragColor = GaussianBlur(9, vec2(0.0, 1.0), vec2(0.0, 1.0) * 0.005, 2.0, sDiffMap, vTexCoord);
//    gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb;
    float result = 0.0;
    for (int y = -blurRadius; y < blurRadius; ++y)
    {
        vec2 offset = vec2(0.0, float(y) * 0.0001) * blurRadius;
        result += texture2D(sDiffMap, vTexCoord + offset).r;
    }

    result = result / float(blurRadius * 2);
    gl_FragColor.r = result;
    gl_FragColor.g = result;
    gl_FragColor.b = result;
#endif

#ifdef OUTPUT

//    if (vTexCoord.x < 0.33) {
        gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb;
//    } else if (vTexCoord.x < 0.666) {
//    gl_FragColor.rgb = texture2D(sDepthBuffer, vTexCoord).rgb;
//        color
//        gl_FragColor.rgb = strength;
//////////////////    } else {
        gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb * texture2D(sDepthBuffer, vTexCoord).rgb;
//    }
#endif

}
