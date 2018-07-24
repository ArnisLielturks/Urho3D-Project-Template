#include <Urho3D/Urho3DAll.h>
#include "Splash.h"
#include "../MyEvents.h"

using namespace Levels;

static int SPLASH_TIME = 1000;

namespace Levels {
    void CheckThreading(const WorkItem* item, unsigned threadIndex)
    {
        Splash* splashScreen = reinterpret_cast<Splash*>(item->aux_);
        Time::Sleep(1);
        auto* network = splashScreen->context_->GetSubsystem<Network>();
        SharedPtr<HttpRequest> httpRequest_(network->MakeHttpRequest("https://httpbin.org/ip"));

        bool done = false;
        String message_;
        Timer timer;
        while(!done) {
            if (timer.GetMSec(false) > 1000) {
                done = true;
                return;
            }
            // Initializing HTTP request
            if (httpRequest_->GetState() == HTTP_INITIALIZING) {
                done = false;
            }
            // An error has occurred
            else if (httpRequest_->GetState() == HTTP_ERROR) {
                URHO3D_LOGINFO("An error has occurred.");
                done = true;
            }
            // Get message data
            else {
                if (httpRequest_->GetAvailableSize() > 0) {
                    message_ += httpRequest_->ReadLine();
                }
                else if (message_.Length() > 0) {
                    URHO3D_LOGINFO("Processing...");

                    SharedPtr<JSONFile> json(new JSONFile(splashScreen->context_));
                    URHO3D_LOGINFO("message_ " + message_);
                    json->FromString(message_);

                    JSONValue val = json->GetRoot().Get("origin");

                    if (val.IsNull()) {
                        URHO3D_LOGINFO("Invalid string.");
                        //done = true;
                    }
                    else {
                        URHO3D_LOGINFO("Your IP is: " + val.GetString());
                        done = true;
                    }
                }
            }
        }
    }
}

    /// Construct.
Splash::Splash(Context* context) :
    BaseLevel(context)
{
}

Splash::~Splash()
{
}

void Splash::Init()
{
    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

	WorkQueue* workQueue = GetSubsystem<WorkQueue>();
	SharedPtr<WorkItem> item = workQueue->GetFreeItem();
	item->priority_ = M_MAX_UNSIGNED;
	item->workFunction_ = CheckThreading;
	item->aux_ = this;
	// send E_WORKITEMCOMPLETED event after finishing WorkItem
	item->sendEvent_ = true;

	item->start_ = nullptr;// &(*start);
	item->end_ = nullptr;// &(*end);
	workQueue->AddWorkItem(item);
}

void Splash::CreateScene()
{
    return;
}

void Splash::CreateUI()
{
    _timer.Reset();
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Get the Urho3D fish texture
    auto* decalTex = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
    // Create a new sprite, set it to use the texture
    SharedPtr<Sprite> sprite(new Sprite(context_));
    sprite->SetTexture(decalTex);

    auto* graphics = GetSubsystem<Graphics>();

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // The UI root element is as big as the rendering window, set random position within it
    sprite->SetPosition(width / 2, height / 2);

    // Set sprite size & hotspot in its center
    sprite->SetSize(IntVector2(decalTex->GetWidth(), decalTex->GetHeight()));
    sprite->SetHotSpot(IntVector2(decalTex->GetWidth() / 2, decalTex->GetHeight() / 2));

    // Set random rotation in degrees and random scale
    sprite->SetRotation(Random() * 360.0f);
    sprite->SetScale(Random(1.0f) + 0.5f);

    // Set random color and additive blending mode
    sprite->SetColor(Color(Random(0.5f) + 0.5f, Random(0.5f) + 0.5f, Random(0.5f) + 0.5f));
    sprite->SetBlendMode(BLEND_ADD);

    // Add as a child of the root UI element
    ui->GetRoot()->AddChild(sprite);

    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> scale(new ValueAnimation(context_));
    // Use spline interpolation method
    scale->SetInterpolationMethod(IM_SPLINE);
    // Set spline tension
    scale->SetKeyFrame(0.0f, Vector2(1, 1));
    scale->SetKeyFrame(1.0f, Vector2(1.5, 1.5));
    scale->SetKeyFrame(3.0f, Vector2(1, 1));
    animation->AddAttributeAnimation("Scale", scale);

    sprite->SetObjectAnimation(animation);
    
    // Text* text = ui->GetRoot()->CreateChild<Text>();
    // text->SetHorizontalAlignment(HA_RIGHT);
    // text->SetPosition(IntVector2(-20, -20));
    // text->SetVerticalAlignment(VA_BOTTOM);
    // text->SetStyleAuto();
    // text->SetText("Splash...");

    /*SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    // Use spline interpolation method
    colorAnimation->SetInterpolationMethod(IM_SPLINE);
    // Set spline tension
    colorAnimation->SetSplineTension(0.7f);
    colorAnimation->SetKeyFrame(0.0f, IntVector2(-20, -20));
    colorAnimation->SetKeyFrame(1.0f, IntVector2(-20, -40));
    colorAnimation->SetKeyFrame(2.0f, IntVector2(-40, -40));
    colorAnimation->SetKeyFrame(3.0f, IntVector2(-40, -20));
    colorAnimation->SetKeyFrame(4.0f, IntVector2(-20, -20));
    animation->AddAttributeAnimation("Position", colorAnimation);

    text->SetObjectAnimation(animation);
    */
}

void Splash::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Splash, HandleUpdate));
	SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(Splash, HandleWorkItemFinished));
}

void Splash::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }
    if (_timer.GetMSec(false) > SPLASH_TIME) {
        HandleEndSplash();
    }
}

void Splash::HandleEndSplash()
{
	//WorkQueue* workQueue = GetSubsystem<WorkQueue>();
	//workQueue->Complete(100);
	UnsubscribeFromEvent(E_UPDATE);
	VariantMap data = GetEventDataMap();
	data["Name"] = "MainMenu";
    SendEvent(MyEvents::E_SET_LEVEL, data);
}

void Splash::HandleWorkItemFinished(StringHash eventType, VariantMap& eventData)
{
	using namespace WorkItemCompleted;

	WorkItem* item = static_cast<WorkItem*>(eventData[P_ITEM].GetPtr());
	URHO3D_LOGINFO("FISNISHED!!!");
}