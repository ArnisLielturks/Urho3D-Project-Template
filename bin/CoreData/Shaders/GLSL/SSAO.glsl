#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;

#ifdef COMPILEPS
uniform vec2 cSSAOInvSize;
uniform vec2 cBlurHInvSize;
uniform vec2 cBlurVInvSize;
uniform float cSSAOStrength;
uniform float cSSAORadius;
uniform float cSSAOBase;
uniform float cSSAOArea;
uniform float cSSAOFalloff;
uniform float cSSAONoiseFactor;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
}

#ifdef COMPILEPS

vec3 normalFromDepth(float depth, vec2 texcoords)
{
    vec2 offset1 = vec2(0.0, cSSAOInvSize.y);
    vec2 offset2 = vec2(cSSAOInvSize.x, 0.0);

    float depth1 = DecodeDepth(texture2D(sDepthBuffer, texcoords + offset1).rgb) * cFarClipPS;
    float depth2 = DecodeDepth(texture2D(sDepthBuffer, texcoords + offset2).rgb) * cFarClipPS;


    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);

    vec3 normal = cross(p1, p2);

    return normalize(normal);
}

vec3 reflection(vec3 v1,vec3 v2)
{
    vec3 result = 2.0 * dot(v2, v1) * v2;
    result = v1 - result;
    return result;
}

#endif


void PS()
{
#ifdef OCCLUDE
//    gl_FragColor.rgb = texture2D(sDepthBuffer, vTexCoord).rgb * cFarClipPS;
//    return;
    vec3 sample_sphere[16] = vec3[](
        vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
        vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
        vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
        vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
        vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
        vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
        vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
        vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
    );

    vec3 random = normalize( texture2D(sDiffMap, vTexCoord * cSSAONoiseFactor).rgb );
    float depth = DecodeDepth(texture2D(sDepthBuffer, vTexCoord).rgb) * cFarClipPS;
    vec3 position = vec3(vTexCoord.x, vTexCoord.y,depth);
    vec3 normal = normalFromDepth(depth, vTexCoord);

    float radiusDepth = cSSAORadius/depth;
    float occlusion = 0.0;
    int iterations = 16;

    for (int j = 0; j < iterations; ++j)
    {
        vec3 ray = radiusDepth * reflection(sample_sphere[j], random);
        vec3 hemiRay = position + sign(dot(ray, normal)) * ray;

        float occDepth = DecodeDepth(texture2D(sDepthBuffer, clamp(hemiRay.xy, 0.0, 1.0)).rgb) * cFarClipPS;
        float difference = depth - occDepth;

        occlusion += step(cSSAOFalloff, difference) * (1.0 - smoothstep(cSSAOFalloff, cSSAOArea, difference));
    }

    float ao = 1.0 - cSSAOStrength * occlusion * (1.0 / iterations);
    float final = clamp(ao + cSSAOBase, 0.0, 1.0);

    gl_FragColor = vec4(vec3(final), 1.0);
#endif

#ifdef BLURV
    gl_FragColor = GaussianBlur(3, vec2(0.0, 1.0), cBlurVInvSize * 2.0, 2.0, sDiffMap, vTexCoord);
#endif

#ifdef BLURH
    gl_FragColor = GaussianBlur(3, vec2(1.0, 0.0), cBlurHInvSize * 2.0, 2.0, sDiffMap, vTexCoord);
#endif

#ifdef OUTPUT
    if (vTexCoord.y < 0.4) {
        gl_FragColor.rgb = texture2D(sDepthBuffer, vTexCoord).rgb;
    } else {
        if (vTexCoord.x < 0.33) {
            gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb * texture2D(sDepthBuffer, vTexCoord).rgb;
        } else if (vTexCoord.x > 0.66) {
            gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb;
        } else {
            float val = 0.33 + 0.33 * (sin(cElapsedTimePS) * 0.5 + 0.5);
            if (vTexCoord.x > val) {
                gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb;
            } else {
                gl_FragColor.rgb = texture2D(sDiffMap, vTexCoord).rgb * texture2D(sDepthBuffer, vTexCoord).rgb;
            }
        }
    }
#endif
}
