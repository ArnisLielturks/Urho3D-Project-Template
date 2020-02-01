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
#include "../MyEvents.h"


Generator::Generator(Context* context) :
    Object(context)
{
    _generatedImage = new Image(context);
    _generatedImage->SetSize(512, 512, 3);
    SubscribeToEvents();
}

Generator::~Generator()
{
}

Image* Generator::GenerateImage(double frequency, int octaves, int seed)
{
    frequency = Clamp(frequency, 0.1, 64.0);
    octaves = Clamp(octaves, 1, 16);

    PerlinNoise perlin(seed);
    for (int x = 0; x < _generatedImage->GetWidth(); x++) {
        for (int y = 0; y < _generatedImage->GetHeight(); y++) {
            float dx = x / frequency;
            float dy = y / frequency;
            auto result = perlin.octaveNoise(dx, dy, octaves);
            result *= 0.4;
            result += 0.4;
            _generatedImage->SetPixel(x, y, Color(result, result, result));
        }
    }

    return _generatedImage;
}

void Generator::SubscribeToEvents()
{
    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "generate_map",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#generate_map",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Perlin noise map generating",
            MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#generate_map", [&](StringHash eventType, VariantMap &eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() < 4) {
            URHO3D_LOGERROR("generate_map expects 3 parameters: frequency(0.1 - 64.0), octaves(1 - 16), seed");
            return;
        }

        const float frequency = ToFloat(params[1]);
        const int octaves     = ToInt(params[2]);
        const int seed        = ToInt(params[3]);

        GenerateImage(frequency, octaves, seed);
        Save();
    });

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "save_heightmap",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#save_heightmap",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Save generated image to file"
    );
    SubscribeToEvent("#save_heightmap", [&](StringHash eventType, VariantMap &eventData) {
        Save();
    });
}

void Generator::Save()
{
    _generatedImage->SavePNG("Data/Textures/HeightMap.png");
}
