#include <Urho3D/Urho3DAll.h>
#include "Splash.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"

using namespace Levels;

static int SPLASH_TIME = 3000;

    /// Construct.
Splash::Splash(Context* context) :
    BaseLevel(context),
    _logoIndex(0)
{
    // List of different logos that multiple splash screens will show
    _logos.Reserve(3);
    _logos.Push("Textures/UrhoIcon.png");
    _logos.Push("Textures/Achievements/lunar-module.png");
    _logos.Push("Textures/Achievements/retro-controller.png");
}

Splash::~Splash()
{
}

void Splash::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(false);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
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

    // Get the current logo index
    _logoIndex = data_["LogoIndex"].GetInt();

    // Get the Urho3D fish texture
    auto* decalTex = cache->GetResource<Texture2D>(_logos[_logoIndex]);
    // Create a new sprite, set it to use the texture
    SharedPtr<Sprite> sprite(new Sprite(context_));
    sprite->SetTexture(decalTex);

    auto* graphics = GetSubsystem<Graphics>();

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth() / GetSubsystem<UI>()->GetScale();
    auto height = (float)graphics->GetHeight() / GetSubsystem<UI>()->GetScale();

    // The UI root element is as big as the rendering window, set random position within it
    sprite->SetPosition(width / 2, height / 2);

    // Avoid having too large logos
    // We assume here that the logo image is a regular rectangle
    if (decalTex->GetWidth() <= 256 && decalTex->GetHeight() <= 256) {
        // Set sprite size & hotspot in its center
        sprite->SetSize(IntVector2(decalTex->GetWidth(), decalTex->GetHeight()));
        sprite->SetHotSpot(IntVector2(decalTex->GetWidth() / 2, decalTex->GetHeight() / 2));
    } else {
        sprite->SetSize(IntVector2(256, 256));
        sprite->SetHotSpot(IntVector2(128, 128));
    }

    sprite->SetPriority(0);

    // Add as a child of the root UI element
    ui->GetRoot()->AddChild(sprite);

    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> scale(new ValueAnimation(context_));
    // Use spline interpolation method
    scale->SetInterpolationMethod(IM_SPLINE);
    // Set spline tension
    scale->SetKeyFrame(0.0f, Vector2(1, 1));
    scale->SetKeyFrame(SPLASH_TIME / 1000 / 2, Vector2(1.5, 1.5));
    scale->SetKeyFrame(SPLASH_TIME / 1000, Vector2(1, 1));
    scale->SetKeyFrame(SPLASH_TIME / 1000 * 2, Vector2(1, 1));
    animation->AddAttributeAnimation("Scale", scale);

    sprite->SetObjectAnimation(animation);
}

void Splash::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Splash, HandleUpdate));
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
    _logoIndex++;
	if (_logoIndex >= _logos.Size()) {
        data["Name"] = "MainMenu";
    } else {
	    // We still have logos to show, inform next Splash screen to use the next logo from the `_logos` vector
        data["Name"] = "Splash";
        data["LogoIndex"] = _logoIndex;
	}
    SendEvent(MyEvents::E_SET_LEVEL, data);
}
