#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Engine/Engine.h>
#include "Loading.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"
#include "../SceneManager.h"
#include "../Config/ConfigManager.h"

using namespace Levels;

const int SERVER_PORT = 4545;

Loading::Loading(Context* context) :
    BaseLevel(context)
{
}

Loading::~Loading()
{
}

void Loading::RegisterObject(Context* context)
{
    context->RegisterFactory<Loading>();
}

void Loading::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(false);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    SetGlobalVar("PACKET_LIMIT", -1);
    GetSubsystem<Network>()->RegisterRemoteEvent(MyEvents::E_REMOTE_CLIENT_ID);
    if (_data.Contains("StartServer") && _data["StartServer"].GetBool()) {
        SendEvent(MyEvents::E_REGISTER_LOADING_STEP,
                  MyEvents::RegisterLoadingStep::P_NAME, "Starting server",
                  MyEvents::RegisterLoadingStep::P_EVENT, "StartServer");
        SubscribeToEvent("StartServer", [&](StringHash eventType, VariantMap &eventData) {
            SendEvent(MyEvents::E_ACK_LOADING_STEP,
                      MyEvents::RegisterLoadingStep::P_EVENT, "StartServer");
            GetSubsystem<Network>()->StartServer(SERVER_PORT);

            SendEvent(MyEvents::E_LOADING_STEP_FINISHED,
                      MyEvents::RegisterLoadingStep::P_EVENT, "StartServer");
        });
    }

    if (_data.Contains("ConnectServer") && !_data["ConnectServer"].GetString().Empty()) {
        // Register our loading step
        SendEvent(MyEvents::E_REGISTER_LOADING_STEP,
                  MyEvents::RegisterLoadingStep::P_NAME, "Connecting to server",
                  MyEvents::RegisterLoadingStep::P_EVENT, "ConnectServer");
        SubscribeToEvent("ConnectServer", [&](StringHash eventType, VariantMap &eventData) {
            SendEvent(MyEvents::E_ACK_LOADING_STEP,
                      MyEvents::RegisterLoadingStep::P_EVENT, "ConnectServer");
#ifdef __EMSCRIPTEN__
//            GetSubsystem<Network>()->WSConnect("ws://127.0.0.1:9090/ws", GetSubsystem<SceneManager>()->GetActiveScene());
            GetSubsystem<Network>()->WSConnect("wss://playground-server.frameskippers.com/ws", GetSubsystem<SceneManager>()->GetActiveScene());
#else
            GetSubsystem<Network>()->Connect(_data["ConnectServer"].GetString(), SERVER_PORT, GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->WSConnect("wss://playground-server.frameskippers.com/ws", GetSubsystem<SceneManager>()->GetActiveScene());
#endif
        });
        SubscribeToEvent(MyEvents::E_REMOTE_CLIENT_ID, [&](StringHash eventType, VariantMap &eventData) {
            using namespace MyEvents::RemoteClientId;
            _data[P_NODE_ID] = eventData[P_NODE_ID];

            _data[P_PLAYER_ID] = eventData[P_PLAYER_ID];
            URHO3D_LOGINFOF("ClientID %d", eventData["ID"].GetInt());
            SendEvent(MyEvents::E_LOADING_STEP_FINISHED,
                      MyEvents::LoadingStepFinished::P_EVENT, "ConnectServer");
        });
        SubscribeToEvent(E_SERVERCONNECTED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace MyEvents::LoadingStepProgress;
            SendEvent(MyEvents::E_LOADING_STEP_PROGRESS,
                      MyEvents::LoadingStepProgress::P_EVENT, "ConnectServer",
                MyEvents::LoadingStepProgress::P_PROGRESS, 0.5f);
        });
        SubscribeToEvent(E_CONNECTFAILED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace MyEvents::LoadingStepCriticalFail;
            auto localization = GetSubsystem<Localization>();
            VariantMap& data = GetEventDataMap();
            data[P_DESCRIPTION] = localization->Get("CANNOT_CONNECT_TO_SERVER");
            SendEvent(MyEvents::E_LOADING_STEP_CRITICAL_FAIL, data);
        });
    }

    _statusMessage = GetSubsystem<SceneManager>()->GetStatusMessage();
    SubscribeToEvent(MyEvents::E_LOADING_STATUS_UPDATE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MyEvents::LoadingStatusUpdate;
        _statusMessage = eventData[P_NAME].GetString();
        UpdateStatusMesage();
    });

    SubscribeToEvent(E_SERVERDISCONNECTED, [&](StringHash eventType, VariantMap& eventData) {
        auto localization = GetSubsystem<Localization>();
        VariantMap data;
        data["Name"] = "MainMenu";
        data["Message"] = localization->Get("DISCONNECTED_FROM_SERVER");
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    if (_data.Contains("Map")) {
        GetSubsystem<SceneManager>()->LoadScene(_data["Map"].GetString());
    } else {
        GetSubsystem<SceneManager>()->LoadScene("Scenes/Flat.xml");
    }
}

void Loading::CreateScene()
{
    return;
}

void Loading::CreateUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Get the Urho3D fish texture
    auto* decalTex = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
    // Create a new sprite, set it to use the texture
    SharedPtr<Sprite> sprite(new Sprite(context_));
    sprite->SetTexture(decalTex);

    auto* graphics = GetSubsystem<Graphics>();

    if (!graphics) {
        return;
    }

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // The UI root element is as big as the rendering window, set random position within it
    sprite->SetPosition(width - decalTex->GetWidth(), height - decalTex->GetHeight() - 20);

    // Set sprite size & hotspot in its center
    sprite->SetSize(IntVector2(decalTex->GetWidth(), decalTex->GetHeight()));
    sprite->SetHotSpot(IntVector2(decalTex->GetWidth() / 2, decalTex->GetHeight() / 2));

    // Set random rotation in degrees and random scale
    sprite->SetRotation(Random() * 360.0f);


    // Add as a child of the root UI element
    ui->GetRoot()->AddChild(sprite);

    SharedPtr<ObjectAnimation> logoAnimation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> rotation(new ValueAnimation(context_));

    // Use spline interpolation method
    rotation->SetInterpolationMethod(IM_LINEAR);
    // Set spline tension
    rotation->SetSplineTension(0.7f);
    rotation->SetKeyFrame(0.0f, 0.0f);
    rotation->SetKeyFrame(3.0f, 360.0f);
    logoAnimation->AddAttributeAnimation("Rotation", rotation);

    sprite->SetObjectAnimation(logoAnimation);

    _status = ui->GetRoot()->CreateChild<Text>();
    _status->SetHorizontalAlignment(HA_LEFT);
    _status->SetVerticalAlignment(VA_BOTTOM);
    _status->SetPosition(20, -30);
    _status->SetStyleAuto();
    _status->SetText("Progress: 0%");
    _status->SetTextEffect(TextEffect::TE_STROKE);
    _status->SetFontSize(16);
    _status->SetColor(Color(0.8f, 0.8f, 0.2f));
    _status->SetBringToBack(true);


    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    // Use spline interpolation method
    colorAnimation->SetInterpolationMethod(IM_LINEAR);
    colorAnimation->SetKeyFrame(0.0f, Color(0.9, 0.9, 0.9));
    colorAnimation->SetKeyFrame(1.0f, Color(0.7, 0.7, 0.7));
    colorAnimation->SetKeyFrame(2.0f, Color(0.9, 0.9, 0.9));
    animation->AddAttributeAnimation("Color", colorAnimation);

    _status->SetObjectAnimation(animation);

    CreateProgressBar();
}

void Loading::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Loading, HandleUpdate));
    SubscribeToEvent(StringHash("EndLoading"), URHO3D_HANDLER(Loading, HandleEndLoading));
}

void Loading::UpdateStatusMesage()
{
    float progress = GetSubsystem<SceneManager>()->GetProgress();
    if (!GetSubsystem<Engine>()->IsHeadless()) {
        _status->SetText(
                String((int) (progress * 100)) + "% "
                                                 "" + _statusMessage + "...");
    }
}

void Loading::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    float progress = GetSubsystem<SceneManager>()->GetProgress();

    if (!GetSubsystem<Engine>()->IsHeadless()) {
        UpdateStatusMesage();

        if (_loadingBar) {
            _loadingBar->SetWidth(
                    progress * (GetSubsystem<Graphics>()->GetWidth() / GetSubsystem<UI>()->GetScale() - 20));
        }
    }
    if (progress >= 1.0f) {
        SendEvent("EndLoading");
        UnsubscribeFromEvent(E_UPDATE);
    }
}

void Loading::HandleEndLoading(StringHash eventType, VariantMap& eventData)
{
    UnsubscribeFromEvent(E_UPDATE);

    // Forward event data to the next level
    _data["Name"] = "Level";
    SendEvent(MyEvents::E_SET_LEVEL, _data);
}

void Loading::CreateProgressBar()
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "ShowProgressBar", true)) {
        UI *ui = GetSubsystem<UI>();
        ResourceCache *cache = GetSubsystem<ResourceCache>();
        auto *progressBarTexture = cache->GetResource<Texture2D>("Textures/Loading.png");
        _loadingBar = ui->GetRoot()->CreateChild<Sprite>();
        _loadingBar->SetTexture(progressBarTexture);
        auto *graphics = GetSubsystem<Graphics>();
        auto height = (float) graphics->GetHeight() / GetSubsystem<UI>()->GetScale();
        _loadingBar->SetPosition(10, height - 30);
        _loadingBar->SetSize(0, 20);

        SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
        SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
        // Use spline interpolation method
        colorAnimation->SetInterpolationMethod(IM_LINEAR);
        colorAnimation->SetKeyFrame(0.0f, Color(0.9, 0.9, 0.9));
        colorAnimation->SetKeyFrame(1.0f, Color(0.7, 0.7, 0.7));
        colorAnimation->SetKeyFrame(2.0f, Color(0.9, 0.9, 0.9));
        animation->AddAttributeAnimation("Color", colorAnimation);

        _loadingBar->SetObjectAnimation(animation);

        // Reposition loading bar when screen is resized (mostly for web platform)
        SubscribeToEvent(E_SCREENMODE, [&](StringHash eventType, VariantMap& eventData) {
            auto *graphics = GetSubsystem<Graphics>();
            auto height = (float) graphics->GetHeight() / GetSubsystem<UI>()->GetScale();
            _loadingBar->SetPosition(10, height - 30);
        });
    }
}
