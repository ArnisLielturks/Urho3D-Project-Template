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
        auto* cache = GetSubsystem<ResourceCache>();

        // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
        // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
        //cameraNode_ = new Node(context_);
        cameraNode_ = scene_->CreateChild("Camera", LOCAL);
        cameraNode_->SetPosition(Vector3(0, 1, 0));
        // Light* light = cameraNode_->CreateComponent<Light>();
        // light->SetLightType(LightType::LIGHT_POINT);
        Camera* camera = cameraNode_->CreateComponent<Camera>(LOCAL);
        camera->SetFarClip(500.0f);
        camera->SetNearClip(0.1f);
        camera->SetFov(60);
        cameraNode_->CreateComponent<SoundListener>();
        GetSubsystem<Audio>()->SetListener(cameraNode_->GetComponent<SoundListener>());

        Viewport* _viewport = new Viewport(context_, scene_, camera);
        SharedPtr<RenderPath> effectRenderPath = _viewport->GetRenderPath()->Clone();
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
        // Make the bloom mixing parameter more pronounced
        //effectRenderPath->SetShaderParameter("AutoExposureAdaptRate", 0.1);
        effectRenderPath->SetEnabled("AutoExposure", true);
        effectRenderPath->SetEnabled("BloomHDR", true);
        effectRenderPath->SetEnabled("FXAA3", true);
        _viewport->SetRenderPath(effectRenderPath);

        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetViewport(0, _viewport);
        //GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
    }

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    VariantMap data_;

    bool _expired;
};