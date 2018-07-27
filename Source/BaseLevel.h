#pragma once

#include <Urho3D/Urho3DAll.h>
#include "MyEvents.h"

class BaseLevel : public Object
{
    URHO3D_OBJECT(BaseLevel, Object);
public:
    BaseLevel(Context* context) :
        Object(context),
        _expired(false)
    {
        SubscribeToBaseEvents();
    }

    virtual ~BaseLevel()
    {
        Dispose();
    }

    void SetExpired()
    {
        _expired = true;
    }

    bool GetExpired()
    {
        return _expired;
    }

private:
    void SubscribeToBaseEvents()
    {
        SubscribeToEvent("LevelStart", URHO3D_HANDLER(BaseLevel, HandleStart));
        SubscribeToEvent(MyEvents::E_LEVEL_CHANGING_FINISHED, URHO3D_HANDLER(BaseLevel, HandleLevelLoaded));
    }

    void HandleStart(StringHash eventType, VariantMap& eventData)
    {
        data_ = eventData;
        Init();
        SubscribeToEvents();
    }

    void HandleLevelLoaded(StringHash eventType, VariantMap& eventData)
    {
        OnLoaded();
    }

protected:

    virtual void Init() {
    }

    virtual void OnLoaded()
    {

    }

    virtual void Run()
    {
        if (scene_) {
            scene_->SetUpdateEnabled(true);
        }
    }

    virtual void Pause()
    {
        if (scene_) {
            scene_->SetUpdateEnabled(false);
        }
    }

    void SubscribeToEvents()
    {
        SubscribeToEvent("FovChange", URHO3D_HANDLER(BaseLevel, HandleFovChange));

        using namespace MyEvents::ConsoleCommandAdd;
        VariantMap data = GetEventDataMap();
        data[P_NAME] = "fov";
        data[P_EVENT] = "FovChange";
        data[P_DESCRIPTION] = "Show/Change camera fov";
        SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
    }

    void HandleFovChange(StringHash eventType, VariantMap& eventData)
    {
        StringVector params = eventData["Parameters"].GetStringVector();

        if (!_cameras.Empty()) {
            for (auto it = _cameras.Begin(); it != _cameras.End(); ++it) {
                Node* cameraNode = (*it).second_;
                if (params.Size() == 1) {
                    URHO3D_LOGINFOF("Current fov value: %f", cameraNode->GetComponent<Camera>()->GetFov());
                }
                else if (params.Size() == 2) {
                    float previousValue = cameraNode->GetComponent<Camera>()->GetFov();
                    float value = ToFloat(params.At(1));
                    if (value <= 0) {
                        value = 60;
                    }
                    cameraNode->GetComponent<Camera>()->SetFov(value);
                    URHO3D_LOGINFOF("Camera fov changed from '%f' to '%f'", previousValue, value);
                }
                else {
                    URHO3D_LOGERROR("Invalid number of parameters!");
                }
            }
        }
        else {
            URHO3D_LOGERROR("No camera available!");
        }
    }

    virtual void Dispose()
    {
        // Pause the scene, remove all contents from the scene, then remove the scene itself.
        if (scene_) {
            scene_->SetUpdateEnabled(false);
            scene_->Clear();
            scene_->Remove();
        }
        // if (cameraNode_) {
        //  cameraNode_->Remove();
        // }

        // Remove all UI elements from UI sub-system
        if (GetSubsystem<UI>()) {
            GetSubsystem<UI>()->GetRoot()->RemoveAllChildren();
        }
    }

    virtual void CreateCamera()
    {
    }

    Vector<IntRect> InitRects(int count)
    {
        auto* graphics = GetSubsystem<Graphics>();
        Vector<IntRect> rects;
        if (count == 1) {
            // whole screen
            rects.Push(IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight()));
        }
        else if (count == 2) {
            rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight()));
            rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight()));
        }
        else if (count == 3) {
            rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight() / 2));
            rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));
            rects.Push(IntRect(0, graphics->GetHeight() / 2, graphics->GetWidth(), graphics->GetHeight()));
        }
        else if (count == 4) {
            rects.Push(IntRect(0, 0, graphics->GetWidth() / 2, graphics->GetHeight() / 2));
            rects.Push(IntRect(graphics->GetWidth() / 2, 0, graphics->GetWidth(), graphics->GetHeight() / 2));
            rects.Push(IntRect(0, graphics->GetHeight() / 2, graphics->GetWidth() / 2, graphics->GetHeight()));
            rects.Push(IntRect(graphics->GetWidth() / 2, graphics->GetHeight() / 2, graphics->GetWidth(), graphics->GetHeight()));
        }

        return rects;
    }

    void InitViewports(Vector<int> playerIndexes)
    {
        auto* graphics = GetSubsystem<Graphics>();
        auto* cache = GetSubsystem<ResourceCache>();

        Vector<IntRect> rects = InitRects(playerIndexes.Size());

        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetNumViewports(0);
        _viewports.Clear();
        _cameras.Clear();

        for (int i = 0; i < playerIndexes.Size(); i++) {
            // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
            // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
            //cameraNode_ = new Node(context_);
            SharedPtr<Node> cameraNode(scene_->CreateChild("Camera", LOCAL));
            cameraNode->SetPosition(Vector3(0, 1, 0));
            // Light* light = cameraNode_->CreateComponent<Light>();
            // light->SetLightType(LightType::LIGHT_POINT);
            Camera* camera = cameraNode->CreateComponent<Camera>(LOCAL);
            camera->SetFarClip(500.0f);
            camera->SetNearClip(0.1f);
            camera->SetFov(60);
            cameraNode->CreateComponent<SoundListener>();
            GetSubsystem<Audio>()->SetListener(cameraNode->GetComponent<SoundListener>());

            SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera, rects[i]));
            SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
            effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
            // Make the bloom mixing parameter more pronounced
            //effectRenderPath->SetShaderParameter("AutoExposureAdaptRate", 0.1);
            effectRenderPath->SetEnabled("AutoExposure", GetGlobalVar("AutoExposure").GetBool());
            effectRenderPath->SetEnabled("BloomHDR", GetGlobalVar("BloomHDR").GetBool());
            effectRenderPath->SetEnabled("FXAA3", GetGlobalVar("FXAA3").GetBool());
            viewport->SetRenderPath(effectRenderPath);

            Renderer* renderer = GetSubsystem<Renderer>();
            renderer->SetViewport(i, viewport);

            _viewports[playerIndexes[i]] = viewport;
            _cameras[playerIndexes[i]] = cameraNode;
        }
    }

    SharedPtr<Scene> scene_;
    VariantMap data_;

    bool _expired;

    HashMap<int, SharedPtr<Viewport>> _viewports;
    HashMap<int, SharedPtr<Node>> _cameras;
};