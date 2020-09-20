#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>

#if !defined(__EMSCRIPTEN__)
#include <Urho3D/Network/Network.h>
#endif

#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Engine/Engine.h>
#include "Loading.h"
#include "../Messages/Achievements.h"
#include "../SceneManager.h"
#include "../Config/ConfigManager.h"
#include "../SceneManagerEvents.h"
#include "../LevelManagerEvents.h"
#include "../NetworkEvents.h"

using namespace Levels;
using namespace SceneManagerEvents;
using namespace LevelManagerEvents;
using namespace NetworkEvents;

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

    SetGlobalVar("PACKET_LIMIT", 10000);
#if !defined(__EMSCRIPTEN__)
    GetSubsystem<Network>()->RegisterRemoteEvent(E_REMOTE_CLIENT_ID);
    if (data_.Contains("StartServer") && data_["StartServer"].GetBool()) {
        SendEvent(E_REGISTER_LOADING_STEP,
                  RegisterLoadingStep::P_NAME, "Starting server",
                  RegisterLoadingStep::P_REMOVE_ON_FINISH, true,
                  RegisterLoadingStep::P_EVENT, "StartServer");
        SubscribeToEvent("StartServer", [&](StringHash eventType, VariantMap &eventData) {
            SendEvent(E_ACK_LOADING_STEP,
                      RegisterLoadingStep::P_EVENT, "StartServer");
            GetSubsystem<Network>()->StartServer(SERVER_PORT);

            SendEvent(E_LOADING_STEP_FINISHED,
                      RegisterLoadingStep::P_EVENT, "StartServer");
        });
    }
#endif

    if (data_.Contains("ConnectServer") && !data_["ConnectServer"].GetString().Empty()) {
        StringVector dependsOn;
        dependsOn.Push("ConnectServer");
        SendEvent(E_REGISTER_LOADING_STEP,
                  RegisterLoadingStep::P_NAME, "Retrieving player data",
                  RegisterLoadingStep::P_REMOVE_ON_FINISH, true,
                  RegisterLoadingStep::P_DEPENDS_ON, dependsOn,
                  RegisterLoadingStep::P_EVENT, "RetrievePlayerData");
        SubscribeToEvent("RetrievePlayerData", [&](StringHash eventType, VariantMap &eventData) {
            SendEvent(E_ACK_LOADING_STEP,
                      RegisterLoadingStep::P_EVENT, "RetrievePlayerData");
            searchPlayerNode_ = true;
        });
        SendEvent(E_REGISTER_LOADING_STEP,
                  RegisterLoadingStep::P_NAME, "Connecting to server",
                  RegisterLoadingStep::P_REMOVE_ON_FINISH, true,
                  RegisterLoadingStep::P_EVENT, "ConnectServer");
        SubscribeToEvent("ConnectServer", [&](StringHash eventType, VariantMap &eventData) {
            SendEvent(E_ACK_LOADING_STEP,
                      RegisterLoadingStep::P_EVENT, "ConnectServer");
#if defined(__EMSCRIPTEN__)
//            GetSubsystem<Network>()->WSConnect("ws://127.0.0.1:9090/ws", GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->WSConnect("wss://playground-server.arnis.dev/ws", GetSubsystem<SceneManager>()->GetActiveScene());
#else
            GetSubsystem<Network>()->Connect(data_["ConnectServer"].GetString(), SERVER_PORT, GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->Connect("192.168.8.107", SERVER_PORT, GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->Connect("playground-sample.frameskippers.com", 30333, GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->WSConnect("wss://playground-server.frameskippers.com/ws", GetSubsystem<SceneManager>()->GetActiveScene());
//            GetSubsystem<Network>()->WSConnect("ws://127.0.0.1:9090/ws", GetSubsystem<SceneManager>()->GetActiveScene());
#endif
        });

        SubscribeToEvent(E_REMOTE_CLIENT_ID, URHO3D_HANDLER(Loading, HandleRemoteClientID));
        SubscribeToEvent(E_LOADING_STEP_TIMED_OUT, URHO3D_HANDLER(Loading, HandleLoadingStepFailed));
        SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(Loading, HandleServerConnected));
        SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(Loading, HandleServerDisconnected));
        SubscribeToEvent(E_NETWORKSCENELOADFAILED, URHO3D_HANDLER(Loading, HandleSceneLoadFailed));
        SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(Loading, HandleConnectFailed));
    }

    statusMessage_ = GetSubsystem<SceneManager>()->GetStatusMessage();
    SubscribeToEvent(E_LOADING_STATUS_UPDATE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace LoadingStatusUpdate;
        statusMessage_ = eventData[P_NAME].GetString();
        UpdateStatusMesage();
    });

    if (data_.Contains("Map")) {
        GetSubsystem<SceneManager>()->LoadScene(data_["Map"].GetString());
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

    status_ = ui->GetRoot()->CreateChild<Text>();
    status_->SetHorizontalAlignment(HA_LEFT);
    status_->SetVerticalAlignment(VA_BOTTOM);
    status_->SetPosition(20, -30);
    status_->SetStyleAuto();
    status_->SetText("Progress: 0%");
    status_->SetTextEffect(TextEffect::TE_STROKE);
    status_->SetFontSize(16);
    status_->SetColor(Color(0.8f, 0.8f, 0.2f));
    status_->SetBringToBack(true);


    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    // Use spline interpolation method
    colorAnimation->SetInterpolationMethod(IM_LINEAR);
    colorAnimation->SetKeyFrame(0.0f, Color(0.9, 0.9, 0.9));
    colorAnimation->SetKeyFrame(1.0f, Color(0.7, 0.7, 0.7));
    colorAnimation->SetKeyFrame(2.0f, Color(0.9, 0.9, 0.9));
    animation->AddAttributeAnimation("Color", colorAnimation);

    status_->SetObjectAnimation(animation);

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
        status_->SetText(
                String((int) (progress * 100)) + "% "
                                                 "" + statusMessage_ + "...");
    }
}

void Loading::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    float progress = GetSubsystem<SceneManager>()->GetProgress();

    if (searchPlayerNode_) {
        SearchPlayerNode();
    }

    if (!GetSubsystem<Engine>()->IsHeadless()) {
        UpdateStatusMesage();

        if (loadingBar_) {
            loadingBar_->SetWidth(
                    progress * (GetSubsystem<Graphics>()->GetWidth() / GetSubsystem<UI>()->GetScale() - 20));
        }
    }
    if (progress >= 1.0f) {
        SendEvent("EndLoading");
        UnsubscribeFromEvent(E_UPDATE);
    }
}

void Loading::SearchPlayerNode()
{
    if (GetSubsystem<SceneManager>()->GetActiveScene()) {
        using namespace RemoteClientId;
        int nodeID = data_[P_NODE_ID].GetInt();
        auto node = GetSubsystem<SceneManager>()->GetActiveScene()->GetNode(nodeID);
        if (node) {
            searchPlayerNode_ = false;
            SendEvent(E_LOADING_STEP_FINISHED,
                      LoadingStepFinished::P_EVENT, "RetrievePlayerData");
        }
    }
}

void Loading::HandleEndLoading(StringHash eventType, VariantMap& eventData)
{
    UnsubscribeFromEvent(E_UPDATE);

    // Forward event data to the next level
    data_["Name"] = "Level";
    SendEvent(E_SET_LEVEL, data_);
}

void Loading::CreateProgressBar()
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "ShowProgressBar", true)) {
        UI *ui = GetSubsystem<UI>();
        ResourceCache *cache = GetSubsystem<ResourceCache>();
        auto *progressBarTexture = cache->GetResource<Texture2D>("Textures/Loading.png");
        loadingBar_ = ui->GetRoot()->CreateChild<Sprite>();
        loadingBar_->SetTexture(progressBarTexture);
        auto *graphics = GetSubsystem<Graphics>();
        auto height = (float) graphics->GetHeight() / GetSubsystem<UI>()->GetScale();
        loadingBar_->SetPosition(10, height - 30);
        loadingBar_->SetSize(0, 20);

        SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
        SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
        // Use spline interpolation method
        colorAnimation->SetInterpolationMethod(IM_LINEAR);
        colorAnimation->SetKeyFrame(0.0f, Color(0.9, 0.9, 0.9));
        colorAnimation->SetKeyFrame(1.0f, Color(0.7, 0.7, 0.7));
        colorAnimation->SetKeyFrame(2.0f, Color(0.9, 0.9, 0.9));
        animation->AddAttributeAnimation("Color", colorAnimation);

        loadingBar_->SetObjectAnimation(animation);

        // Reposition loading bar when screen is resized (mostly for web platform)
        SubscribeToEvent(E_SCREENMODE, [&](StringHash eventType, VariantMap& eventData) {
            auto *graphics = GetSubsystem<Graphics>();
            auto height = (float) graphics->GetHeight() / GetSubsystem<UI>()->GetScale();
            loadingBar_->SetPosition(10, height - 30);
        });
    }
}

void Loading::HandleServerDisconnected(StringHash eventType, VariantMap& eventData)
{
    auto localization = GetSubsystem<Localization>();
    VariantMap data;
    data["Name"] = "MainMenu";
    data["Message"] = localization->Get("DISCONNECTED_FROM_SERVER");
    SendEvent(E_SET_LEVEL, data);
}

void Loading::HandleSceneLoadFailed(StringHash eventType, VariantMap& eventData)
{
#if !defined(__EMSCRIPTEN__)
    UnsubscribeFromEvent(E_SERVERDISCONNECTED);
    if (GetSubsystem<Network>() && GetSubsystem<Network>()->GetServerConnection()) {
        GetSubsystem<Network>()->Disconnect(200);
        auto localization = GetSubsystem<Localization>();
        VariantMap data;
        data["Name"] = "MainMenu";
        data["Message"] = localization->Get("SCENE_LOAD_FAILED");
        SendEvent(E_SET_LEVEL, data);
    }
#endif
}

void Loading::HandleConnectFailed(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepCriticalFail;
    auto localization = GetSubsystem<Localization>();
    VariantMap& data = GetEventDataMap();
    data[P_DESCRIPTION] = localization->Get("CANNOT_CONNECT_TO_SERVER");
    SendEvent(E_LOADING_STEP_CRITICAL_FAIL, data);
}

void Loading::HandleServerConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepProgress;
    SendEvent(E_LOADING_STEP_PROGRESS,
              LoadingStepProgress::P_EVENT, "ConnectServer",
              LoadingStepProgress::P_PROGRESS, 0.5f);
}

void Loading::HandleRemoteClientID(StringHash eventType, VariantMap& eventData)
{
    using namespace RemoteClientId;
    data_[P_NODE_ID] = eventData[P_NODE_ID];
    data_[P_PLAYER_ID] = eventData[P_PLAYER_ID];
    URHO3D_LOGINFOF("Remote node ID=%d received", eventData[P_NODE_ID].GetInt());
    GetSubsystem<SceneManager>()->GetActiveScene()->SetUpdateEnabled(true);
    SendEvent(E_LOADING_STEP_FINISHED,
              LoadingStepFinished::P_EVENT, "ConnectServer");
}

void Loading::HandleLoadingStepFailed(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepCriticalFail;
    auto localization = GetSubsystem<Localization>();
    if (eventData[P_EVENT].GetString() == "RetrievePlayerData") {
#if !defined(__EMSCRIPTEN__)
        UnsubscribeFromEvent(E_SERVERDISCONNECTED);
        if (GetSubsystem<Network>() && GetSubsystem<Network>()->GetServerConnection()) {
            GetSubsystem<Network>()->Disconnect(200);
        }
        VariantMap& data = GetEventDataMap();
        data[P_EVENT] = "RetrievePlayerData";
        data[P_DESCRIPTION] = localization->Get("FAILED_TO_RETRIEVE_PLAYER_DATA");
        SendEvent(E_LOADING_STEP_CRITICAL_FAIL, data);
#endif
    }
}
