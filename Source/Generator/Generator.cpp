#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/Log.h>
#include "Generator.h"
#include "../Global.h"
#include "PerlinNoise.h"
#include "../SceneManagerEvents.h"
#include "../Console/ConsoleHandlerEvents.h"
#include "../Levels/Voxel/Chunk.h"

using namespace ConsoleHandlerEvents;

Generator::Generator(Context* context) :
    Object(context)
{
    generatedImage_ = new Image(context);
    generatedImage_->SetSize(256, 256, 3);
    SubscribeToEvents();
    GenerateTextures();
}

Generator::~Generator()
{
}

Image* Generator::GenerateImage(double frequency, int octaves, int seed)
{
    frequency = Clamp(frequency, 0.1, 64.0);
    octaves = Clamp(octaves, 1, 16);

    PerlinNoise perlin(seed);
    for (int x = 0; x < generatedImage_->GetWidth(); x++) {
        for (int y = 0; y < generatedImage_->GetHeight(); y++) {
            float dx = x / frequency;
            float dy = y / frequency;
            auto result = perlin.octaveNoise(dx, dy, octaves);
            result *= 0.4;
            result += 0.4;
            generatedImage_->SetPixel(x, y, Color(result, result, result));
        }
    }

    return generatedImage_;
}

void Generator::SubscribeToEvents()
{
    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "generate_map",
            ConsoleCommandAdd::P_EVENT, "#generate_map",
            ConsoleCommandAdd::P_DESCRIPTION, "Perlin noise map generating",
            ConsoleCommandAdd::P_OVERWRITE, true
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
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "save_heightmap",
            ConsoleCommandAdd::P_EVENT, "#save_heightmap",
            ConsoleCommandAdd::P_DESCRIPTION, "Save generated image to file"
    );
    SubscribeToEvent("#save_heightmap", [&](StringHash eventType, VariantMap &eventData) {
        Save();
    });


    // Register our loading step
    SendEvent(SceneManagerEvents::E_REGISTER_LOADING_STEP,
              SceneManagerEvents::RegisterLoadingStep::P_NAME, "Generating world",
              SceneManagerEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");

    // Handle our loading step
    SubscribeToEvent("GenerateWorld", [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(SceneManagerEvents::E_ACK_LOADING_STEP,
                SceneManagerEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");
        GenerateImage(40, 1, Random());
        Save();
        SendEvent(SceneManagerEvents::E_LOADING_STEP_FINISHED,
                  SceneManagerEvents::RegisterLoadingStep::P_EVENT, "GenerateWorld");
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "generate_ui_textures",
            ConsoleCommandAdd::P_EVENT, "#generate_ui_textures",
            ConsoleCommandAdd::P_DESCRIPTION, "Generate UI textures"
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
    generatedImage_->SavePNG("Data/Textures/HeightMap.png");
}

void Generator::GenerateTextures()
{
    SharedPtr<Image> combined(new Image(context_));

    struct TextureColors {
        bool singleColor;
        Color colors[6];
    };
    Vector<TextureColors> textures;
    TextureColors stone;
    stone.singleColor = true;
    stone.colors[0] = Color(0.41,0.41,0.41);
    textures.Push(stone);

    TextureColors dirt;
    dirt.singleColor = false;
    dirt.colors[0] =  Color(0.00,0.30,0.10);
    dirt.colors[1] = Color(0.60,0.30,0.00);
    dirt.colors[2] = Color(0.60,0.30,0.00);
    dirt.colors[3] = Color(0.60,0.30,0.00);
    dirt.colors[4] = Color(0.60,0.30,0.00);
    dirt.colors[5] = Color(0.60,0.30,0.00);
    dirt.colors[6] = Color(0.60,0.30,0.00);
    textures.Push(dirt);

    TextureColors sand;
    sand.singleColor = true;
    sand.colors[0] = Color(0.93,0.79,0.69);
    textures.Push(sand);

    combined->SetSize(32 * 6, 32 * textures.Size(), 4);
    int border = 2;
    for (int t = 0; t < textures.Size(); t++) {
        for (int i = 0; i < 6; i++) {
            for (int x = 0; x < 32; x++) {
                for (int y = 0; y < 32; y++) {
                    if (x < border || x >= 32 - border || y < border || y >= 32 - border) {
                        combined->SetPixel(i * 32 + x, t * 32 + y, Color::BLACK);
                    } else {
                        if (textures.At(t).singleColor) {
                            combined->SetPixel(i * 32 + x, t * 32 + y, textures.At(t).colors[0]);
                        } else {
                            combined->SetPixel(i * 32 + x, t * 32 + y, textures.At(t).colors[i]);
                        }
                    }
                }
            }
        }
    }
    combined->SaveFile("Data/Textures/combined.png");
}