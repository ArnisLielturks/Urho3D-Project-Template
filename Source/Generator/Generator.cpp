#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Engine/DebugHud.h>
#include "Generator.h"
#include "../Global.h"
#include "PerlinNoise.h"


Generator::Generator(Context* context) :
    Object(context)
{
    Init();
}

Generator::~Generator()
{
}

void Generator::Init()
{
    SubscribeToEvents();
    const int octaves = 2;
    const double frequency = 4;
    const int width = 100;
    const int height = 100;
    const int seed = 100;

//    PerlinNoise perlin(seed);
//    for (int x = 0; x < width; x++) {
//        for (int y = 0; y < height; y++) {
//            float dx = x / frequency;
//            float dy = y / frequency;
//            auto result = perlin.octaveNoise0_1(dx, dy, octaves);
//            URHO3D_LOGINFOF("X:%d; Y:%d; P:%f", x, y, result);
//        }
//    }
}

void Generator::SubscribeToEvents()
{

}