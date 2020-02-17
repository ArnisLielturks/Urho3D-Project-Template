#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform float cTonemapExposureBias;
uniform float cTonemapMaxWhite;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

// Based on Filmic Tonemapping Operators http://filmicgames.com/archives/75
vec3 tonemapFilmic(vec3 color) {
    vec3 x = max(vec3(0.0), color - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

    //void PS()
    //{
    ////    #ifdef REINHARDEQ3
    ////    vec3 color = ReinhardEq3Tonemap(max(texture2D(sDiffMap, vScreenPos).rgb * cTonemapExposureBias, 0.0));
    ////    gl_FragColor = vec4(color, 1.0);
    ////    #endif
    //
    ////    #ifdef REINHARDEQ4
    ////    vec3 color = ReinhardEq4Tonemap(max(texture2D(sDiffMap, vScreenPos).rgb * cTonemapExposureBias, 0.0), cTonemapMaxWhite);
    ////    gl_FragColor = vec4(color, 1.0);
    ////    #endif
    //
    //    #ifdef UNCHARTED2
    //    vec3 color = Uncharted2Tonemap(max(texture2D(sDiffMap, vScreenPos).rgb * cTonemapExposureBias, 0.0)) /
    //        Uncharted2Tonemap(vec3(cTonemapMaxWhite, cTonemapMaxWhite, cTonemapMaxWhite));
    //    gl_FragColor = vec4(color, 1.0);
    //    #endif
    //
    ////    #ifdef UNCHARTED2
    ////    vec3 color = Uncharted2Tonemap(max(texture2D(sDiffMap, vScreenPos).rgb * cTonemapExposureBias, 0.0)) /
    ////    Uncharted2Tonemap(vec3(cTonemapMaxWhite, cTonemapMaxWhite, cTonemapMaxWhite));
    ////    gl_FragColor = vec4(tonemapFilmic(texture2D(sDiffMap, vScreenPos).rgb), 1.0);
    ////    #endif
    //}

    #ifdef COMPILEPS
const int MAX_STEPS = 32;
const float MIN_SURFACE_DISTANCE = 0.001;
const float MAX_DISTANCE = 50.0;
const float SPEED = 1.3;

float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float GetDistance(vec3 position)
{
    vec4 sphere = vec4(1, 1, 1, 0.2);
    vec4 sphere2 = vec4(0, 2, 5, 1);
    vec3 plane = vec3(0, 1, 0);

    position = position - vec3(cElapsedTimePS * SPEED, cElapsedTimePS * SPEED, -cElapsedTimePS * SPEED);

    float d1 = distance(mod(position, sphere.x * 2.0), sphere.xyz) - sphere.w;
    return d1;
}

vec3 GetNormal(vec3 position)
{
    float dist = GetDistance(position);
    vec2 offset = vec2(0.01, 0);
    vec3 normal = dist - vec3(
    GetDistance(position - offset.xyy),
    GetDistance(position - offset.yxy),
    GetDistance(position - offset.yyx)
    );

    return normalize(normal);
}

float RayMarch(vec3 position, vec3 direction)
{
    float dist = 0.0;

    int steps = 0;
    for (steps = 0; steps < MAX_STEPS; steps++) {
        float nextDistance = GetDistance(position + direction * dist);
        dist += nextDistance;

        if (nextDistance < MIN_SURFACE_DISTANCE || dist > MAX_DISTANCE) {
            break;
        }
    }
    return dist;
}

float GetLight(vec3 position)
{
//    return length(vec3(0, 0, 0) - position.xyz);
    vec3 light = vec3(1, 10, -1);

    vec3 direction = normalize(light - position);
    vec3 normal = GetNormal(position);

    float dist = RayMarch(position + normal * MIN_SURFACE_DISTANCE * 2.0, direction);

    float brightness = dot(normal, direction);
    brightness = clamp(brightness, 0.0, 1.0);

    if (dist < length(light - position)) {
        brightness  = 0.0;
    }


    return brightness;
}

void PS() {
    vec2 uv = vScreenPos.xy - 0.5;

    vec3 rayOrigin = vec3(0, 1.0, -10);
    vec3 rayDirection = vec3(uv.x, uv.y, 1);
//    rayDirection.x += sin(cElapsedTimePS * SPEED);
//    rayDirection.y += cos(cElapsedTimePS * SPEED);

    float dist = RayMarch(rayOrigin, rayDirection);
    if (dist >= MAX_DISTANCE) {
                gl_FragColor = vec4(vec3(0), 1);
    } else {
        float color = GetLight(rayOrigin + rayDirection * dist);
        gl_FragColor = vec4(vec3(color), 1.0);
        //        gl_FragColor = vec4(GetNormal(rayOrigin + rayDirection * dist), 1.0);
//        gl_FragColor = vec4(texture2D(sDiffMap, vScreenPos).rgb * vec3(color), 1);
    }
//    gl_FragColor = vec4(texture2D(sDiffMap, vScreenPos).rgb * vec3(color), 1);

//        gl_FragColor.rgb = reflect(vec3(0, 0, 1), vec3(0, 1, 0));
}
    #endif
