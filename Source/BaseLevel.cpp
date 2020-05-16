#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Engine/EngineEvents.h>
#include "BaseLevel.h"
#include "Input/ControllerInput.h"
#include "SceneManager.h"
#include "Global.h"

BaseLevel::BaseLevel(Context* context) :
Object(context)
{
    SubscribeToBaseEvents();
    _scene = GetSubsystem<SceneManager>()->GetActiveScene();
    SetGlobalVar("CameraFov", 80);
}

BaseLevel::~BaseLevel()
{
    Dispose();
}

void BaseLevel::SubscribeToBaseEvents()
{
    SubscribeToEvent(MyEvents::E_LEVEL_CHANGING_IN_PROGRESS, URHO3D_HANDLER(BaseLevel, HandleStart));

    // How to use lambda (anonymous) functions
    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "gamma", MyEvents::ConsoleCommandAdd::P_EVENT, "gamma", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change gamma", MyEvents::ConsoleCommandAdd::P_OVERWRITE, true);
    SubscribeToEvent("gamma", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() == 2) {
            float value = ToFloat(params[1]);
            GetSubsystem<ConfigManager>()->Set("postprocess", "Gamma", value);
            GetSubsystem<ConfigManager>()->Save(true);
            ApplyPostProcessEffects();
        }
        else {
            URHO3D_LOGERROR("Invalid number of parameters");
        }
    });

    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "clip", MyEvents::ConsoleCommandAdd::P_EVENT, "clip", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change camera far/near clip", MyEvents::ConsoleCommandAdd::P_OVERWRITE, true);
    SubscribeToEvent("clip", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() == 3) {
            float near = ToFloat(params[1]);
            float far = ToFloat(params[2]);
            for (int i = 0; i < _cameras.Size(); i++) {
                _cameras[i]->GetComponent<Camera>()->SetNearClip(near);
                _cameras[i]->GetComponent<Camera>()->SetFarClip(far);
                URHO3D_LOGINFOF("Updating camera %d, near=%f, far=%f", i, near, far);
            }
        }
    });

    SubscribeToEvent("postprocess", [&](StringHash eventType, VariantMap& eventData) {
        ApplyPostProcessEffects();
    });

    SubscribeToEvent(E_SCREENMODE, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED);
    });

    SubscribeToEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED, [&](StringHash eventType, VariantMap& eventData) {
        auto cache = GetSubsystem<ResourceCache>();
        for (int i = 0; i < GetSubsystem<Renderer>()->GetNumViewports(); i++) {
            auto viewport = GetSubsystem<Renderer>()->GetViewport(i);
            SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
            effectRenderPath->SetShaderParameter("ScreenWidth", GetSubsystem<Graphics>()->GetWidth());
            effectRenderPath->SetShaderParameter("ScreenHeight", GetSubsystem<Graphics>()->GetHeight());

            viewport->SetRenderPath(effectRenderPath);
        }
    });
}

void BaseLevel::HandleStart(StringHash eventType, VariantMap& eventData)
{
    _data = eventData;
    Init();
    SubscribeToEvents();

    if (_scene) {
        Node *zoneNode = _scene->CreateChild("Zone", LOCAL);
        _defaultZone = zoneNode->CreateComponent<Zone>();
        _defaultZone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
        _defaultZone->SetAmbientColor(Color(0.3f, 0.3f, 0.3f));
        _defaultZone->SetFogStart(50.0f);
        _defaultZone->SetFogEnd(300.0f);

        if (_data.Contains("Commands")) {
            URHO3D_LOGINFO("Calling map commands");
            StringVector commands = _data["Commands"].GetStringVector();
            for (auto it = commands.Begin(); it != commands.End(); ++it) {
                using namespace ConsoleCommand;
                SendEvent(
                        E_CONSOLECOMMAND,
                        P_ID, "0",
                        P_COMMAND, (*it)
                );
            }
        }
    }
}

void BaseLevel::Run()
{
    if (_scene) {
        _scene->SetUpdateEnabled(true);
    }
}

void BaseLevel::Pause()
{
    if (_scene) {
        _scene->SetUpdateEnabled(false);
    }
}

void BaseLevel::SubscribeToEvents()
{
    SubscribeToEvent("FovChange", URHO3D_HANDLER(BaseLevel, HandleFovChange));

    using namespace MyEvents::ConsoleCommandAdd;
    VariantMap& data = GetEventDataMap();
    data[P_NAME] = "fov";
    data[P_EVENT] = "FovChange";
    data[P_DESCRIPTION] = "Show/Change camera fov";
    data[P_OVERWRITE] = true;
    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "ambient_light",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#ambient_light",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change scene ambient light",
            MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#ambient_light", [&](StringHash eventType, VariantMap &eventData) {
        if (!_scene) {
            URHO3D_LOGWARNING("No scene to update");
            return;
        }
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() < 4) {
            URHO3D_LOGERROR("ambient_light expects 3 float values: r g b ");
            return;
        }

        const float r = ToFloat(params[1]);
        const float g = ToFloat(params[2]);
        const float b = ToFloat(params[3]);
        _defaultZone->SetAmbientColor(Color(r, g, b));
    });

    SendEvent(
            MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "fog",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#fog",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change custom scene fog",
            MyEvents::ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#fog", [&](StringHash eventType, VariantMap &eventData) {
        if (!_scene) {
            URHO3D_LOGWARNING("No scene to update");
            return;
        }
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() < 3) {
            URHO3D_LOGERROR("fog expects 2 parameters: fog_start fog_end ");
            return;
        }

        const float start = ToFloat(params[1]);
        const float end = ToFloat(params[2]);
        _defaultZone->SetFogStart(start);
        _defaultZone->SetFogEnd(end);
    });
}

void BaseLevel::HandleFovChange(StringHash eventType, VariantMap& eventData)
{
    StringVector params = eventData["Parameters"].GetStringVector();

    if (params.Size() == 1) {
        URHO3D_LOGINFOF("Current fov value: %f", GetGlobalVar("CameraFov").GetFloat());
    }
    else if (params.Size() == 2) {
        float previousValue = GetGlobalVar("CameraFov").GetFloat();
        float value = ToFloat(params.At(1));
        if (value < 60) {
            value = 60;
        }
        if (value > 160) {
            value = 160;
        }
        if (!_cameras.Empty()) {
            for (auto it = _cameras.Begin(); it != _cameras.End(); ++it) {
                Node* cameraNode = (*it).second_;
                cameraNode->GetComponent<Camera>()->SetFov(value);
                SetGlobalVar("CameraFov", value);
            }
        }
        URHO3D_LOGINFOF("Camera fov changed from '%f' to '%f'", previousValue, value);
    }
    else {
        URHO3D_LOGERROR("Invalid number of parameters!");
    }
}

void BaseLevel::Dispose()
{
    // Pause the scene, remove all contents from the scene, then remove the scene itself.
    if (_scene) {
        _scene->SetUpdateEnabled(false);
        _scene->Clear();
        _scene->Remove();
    }

    // Remove all UI elements from UI sub-system
    if (GetSubsystem<UI>()) {
        GetSubsystem<UI>()->GetRoot()->RemoveAllChildren();
    }
}

/**
 * Define rects for splitscreen mode
 */
Vector<IntRect> BaseLevel::InitRects(int count)
{
    auto* graphics = GetSubsystem<Graphics>();
    Vector<IntRect> rects;
    if (count == 1) {
        // whole screen
        rects.Push(IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight()));
    }

        // 2 players - split vertically
    else if (count == 2) {
        // Left
        rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight()));
        // Right
        rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight()));
    }

    else if (count == 3) {

        // player 1 - top left corner
        rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight() / 2));
        // player 2 - top right corner
        rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));
        // player 3 - bottom
        rects.Push(IntRect(0, graphics->GetHeight() / 2, graphics->GetWidth(), graphics->GetHeight()));
    }
    else if (count == 4) {
        // split screen into 4 rectangles
        // Top left
        rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight() / 2));
        // Top right
        rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));
        // Bottom left
        rects.Push(IntRect(0, graphics->GetHeight() / 2, graphics->GetWidth() / 2, graphics->GetHeight()));
        // Bottom right
        rects.Push(IntRect(graphics->GetWidth() / 2, graphics->GetHeight() / 2, graphics->GetWidth(), graphics->GetHeight()));
    }
    else if (count == 5) {
        // split screen into 5 rectangles
        // Top left
        rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight() / 2));
        // Top right
        rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));

        int width = graphics->GetWidth() / 3;
        int top = graphics->GetHeight() / 2;
        // Bottom left
        rects.Push(IntRect(0, top, width, graphics->GetHeight()));
        // Bottom middle
        rects.Push(IntRect(width, top, width * 2, graphics->GetHeight()));
        // Bottom right
        rects.Push(IntRect(width * 2, top, graphics->GetWidth(), graphics->GetHeight()));
    }
    else if (count == 6) {
        // split screen into 5 rectangles
        int width = graphics->GetWidth() / 3;
        // Top left
        rects.Push(IntRect(0, 0, width, graphics->GetHeight() / 2));
        // Top middle
        rects.Push(IntRect(width, 0, width * 2, graphics->GetHeight() / 2));
        // Top right
        rects.Push(IntRect(width * 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));

        int top = graphics->GetHeight() / 2;
        // Bottom left
        rects.Push(IntRect(0, top, width, graphics->GetHeight()));
        // Bottom middle
        rects.Push(IntRect(width, top, width * 2, graphics->GetHeight()));
        // Bottom right
        rects.Push(IntRect(width * 2, top, graphics->GetWidth(), graphics->GetHeight()));
    } else if (count == 7) {
        int width = graphics->GetWidth() / 3;
        rects.Push(IntRect(0, 0, width, graphics->GetHeight() / 2));
        // Top middle
        rects.Push(IntRect(width, 0, width * 2, graphics->GetHeight() / 2));
        // Top right
        rects.Push(IntRect(width * 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));

        width = graphics->GetWidth() / 4;
        int top = graphics->GetHeight() / 2;
        rects.Push(IntRect(0, top, width, graphics->GetHeight()));
        rects.Push(IntRect(width, top, width * 2, graphics->GetHeight()));
        rects.Push(IntRect(width * 2, top, width * 3, graphics->GetHeight()));
        rects.Push(IntRect(width * 3, top, graphics->GetWidth(), graphics->GetHeight()));
    }
    else if (count == 8) {
        int width = graphics->GetWidth() / 4;
        int top = 0;
        rects.Push(IntRect(0, top, width, graphics->GetHeight() / 2));
        rects.Push(IntRect(width, top, width * 2, graphics->GetHeight() / 2));
        rects.Push(IntRect(width * 2, top, width * 3, graphics->GetHeight() / 2));
        rects.Push(IntRect(width * 3, top, graphics->GetWidth(), graphics->GetHeight() / 2));

        top = graphics->GetHeight() / 2;
        rects.Push(IntRect(0, top, width, graphics->GetHeight()));
        rects.Push(IntRect(width, top, width * 2, graphics->GetHeight()));
        rects.Push(IntRect(width * 2, top, width * 3, graphics->GetHeight()));
        rects.Push(IntRect(width * 3, top, graphics->GetWidth(), graphics->GetHeight()));
    }

    return rects;
}

/**
 * Create viewports based on controller count
 */
void BaseLevel::InitViewports(Vector<int> playerIndexes)
{
    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->SetNumViewports(playerIndexes.Size());
    _viewports.Clear();
    _cameras.Clear();

    if (!_scene) {
        return;
    }

    for (unsigned int i = 0; i < playerIndexes.Size(); i++) {
        CreateSingleCamera(i, playerIndexes.Size(), playerIndexes.At(i));
    }

    ApplyPostProcessEffects();
}

void BaseLevel::CreateSingleCamera(int index, int totalCount, int controllerIndex)
{
    Vector<IntRect> rects = InitRects(totalCount);

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    SharedPtr<Node> cameraNode(_scene->CreateChild("Camera", LOCAL));
    cameraNode->SetPosition(Vector3(0, 1, 0));
    Camera* camera = cameraNode->CreateComponent<Camera>(LOCAL);
    camera->SetFarClip(1000.0f);
    camera->SetNearClip(0.1f);
    camera->SetFov(GetGlobalVar("CameraFov").GetFloat());
    cameraNode->CreateComponent<SoundListener>();
    camera->SetViewMask(1 << controllerIndex);

    //TODO only the last camera will be the sound listener
    GetSubsystem<Audio>()->SetListener(cameraNode->GetComponent<SoundListener>());
    //GetSubsystem<Audio>()->SetListener(nullptr);

    SharedPtr<Viewport> viewport(new Viewport(context_, _scene, camera, rects[index]));

    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(index, viewport);

    auto cache = GetSubsystem<ResourceCache>();
    viewport->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/ForwardDepth.xml"));

    _viewports[controllerIndex] = viewport;
    _cameras[controllerIndex] = cameraNode;
}

void BaseLevel::ApplyPostProcessEffects()
{
    auto cache = GetSubsystem<ResourceCache>();
    for (int i = 0; i < GetSubsystem<Renderer>()->GetNumViewports(); i++) {
        auto viewport = GetSubsystem<Renderer>()->GetViewport(i);
        SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
        if (!effectRenderPath->IsAdded("AutoExposure")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
        }
        if (!effectRenderPath->IsAdded("Bloom")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
        }
        if (!effectRenderPath->IsAdded("BloomHDR")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
        }
        if (!effectRenderPath->IsAdded("FXAA2")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
        }
        if (!effectRenderPath->IsAdded("FXAA3")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
        }

#if !defined(__EMSCRIPTEN__)
        if (!effectRenderPath->IsAdded("GammaCorrection")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
        }
#endif

        if (!effectRenderPath->IsAdded("ColorCorrection")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/ColorCorrection.xml"));
        }
        if (!effectRenderPath->IsAdded("SSAO")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/SSAO.xml"));
        }
        if (!effectRenderPath->IsAdded("Toon")) {
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Toon.xml"));
        }

//        if (!effectRenderPath->IsAdded("RayMarch")) {
//            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/RayMarch.xml"));
//            effectRenderPath->SetEnabled("RayMarch", true);
//        }

        effectRenderPath->SetEnabled("AutoExposure",
                                     GetSubsystem<ConfigManager>()->GetBool("postprocess", "AutoExposure", false));
        effectRenderPath->SetShaderParameter("AutoExposureAdaptRate",
                                             GetSubsystem<ConfigManager>()->GetFloat("postprocess", "AutoExposureAdaptRate",
                                                                                     0.1f));
        effectRenderPath->SetEnabled("Bloom", GetSubsystem<ConfigManager>()->GetBool("postprocess", "Bloom", false));
        effectRenderPath->SetEnabled("BloomHDR", GetSubsystem<ConfigManager>()->GetBool("postprocess", "BloomHDR", false));
        effectRenderPath->SetEnabled("FXAA2", GetSubsystem<ConfigManager>()->GetBool("postprocess", "FXAA2", false));
        effectRenderPath->SetEnabled("FXAA3", GetSubsystem<ConfigManager>()->GetBool("postprocess", "FXAA3", false));

#if !defined(__EMSCRIPTEN__)
        effectRenderPath->SetEnabled("GammaCorrection",
                                     GetSubsystem<ConfigManager>()->GetBool("postprocess", "GammaCorrection", true));
#endif

        effectRenderPath->SetEnabled("ColorCorrection",
                                     GetSubsystem<ConfigManager>()->GetBool("postprocess", "ColorCorrection", false));
        float gamma = Clamp(GAMMA_MAX_VALUE - GetSubsystem<ConfigManager>()->GetFloat("postprocess", "Gamma", 1.0f), 0.05f,
                            GAMMA_MAX_VALUE);
        effectRenderPath->SetShaderParameter("Gamma", gamma);

        effectRenderPath->SetEnabled("SSAO", GetSubsystem<ConfigManager>()->GetBool("postprocess", "SSAO", false));

        effectRenderPath->SetEnabled("Toon", GetSubsystem<ConfigManager>()->GetBool("postprocess", "Toon", false));
        viewport->SetRenderPath(effectRenderPath);
    }
}
