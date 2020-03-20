#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/Log.h>
#include "Generator.h"
#include "../Global.h"
#include "PerlinNoise.h"
#include "../MyEvents.h"

Generator::Generator(Context* context) :
    Object(context)
{
    _generatedImage = new Image(context);
    _generatedImage->SetSize(256, 256, 3);
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


    // Register our loading step
    SendEvent(MyEvents::E_REGISTER_LOADING_STEP,
              MyEvents::RegisterLoadingStep::P_NAME, "Generating world",
              MyEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");

    // Handle our loading step
    SubscribeToEvent("GenerateWorld", [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(MyEvents::E_ACK_LOADING_STEP,
                MyEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");
        GenerateImage(40, 1, Random());
        Save();
        SendEvent(MyEvents::E_LOADING_STEP_FINISHED,
                  MyEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");
    });

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "generate_ui_textures",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#generate_ui_textures",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Generate UI textures"
    );
    SubscribeToEvent("#generate_ui_textures", [&](StringHash eventType, VariantMap &eventData) {
        Image* image = new Image(context_);
        image->SetSize(1, 1, 4);
        image->SetPixel(0, 0, Color(0, 0, 0, 0));
        image->SavePNG("Data/Textures/Blank.png");

        int size = 2;
        Color color(0.1, 0.1, 0.1, 1.0);
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() > 1) {
            color.r_ = ToFloat(params[1]);
            color.g_ = color.r_;
            color.b_ = color.r_;
            size = ToInt(params[1]);
        }
        if (params.Size() > 2) {
            color.a_ = ToFloat(params[2]);
        }
//        image->SetPixel(0, 0, color);
//        image->SavePNG("Data/Textures/Gray.png");


//

        image->SetSize(size, size, 4);
        SetRandomSeed(Time::GetSystemTime());
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                float factor = 1.0;
                Color color(Random(0.0f, factor), Random(0.0f, factor), Random(0.0f, factor), 1.0);
//                color.r_ += factor / 2.0f;
//                color.g_ += factor / 2.0f;
//                color.b_ += factor / 2.0f;
                image->SetPixel(i, j, color);
            }
        }
        image->SavePNG("Data/Textures/Noise.png");

    });
}

void Generator::Save()
{
    _generatedImage->SavePNG("Data/Textures/HeightMap.png");
}
