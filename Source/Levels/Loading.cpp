#include <Urho3D/Urho3DAll.h>
#include "Loading.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"

using namespace Levels;

    /// Construct.
Loading::Loading(Context* context) :
    BaseLevel(context)
{
}

Loading::~Loading()
{
}

void Loading::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(false);

    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
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

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // The UI root element is as big as the rendering window, set random position within it
    sprite->SetPosition(width - decalTex->GetWidth(), height - decalTex->GetHeight());

    // Set sprite size & hotspot in its center
    sprite->SetSize(IntVector2(decalTex->GetWidth(), decalTex->GetHeight()));
    sprite->SetHotSpot(IntVector2(decalTex->GetWidth() / 2, decalTex->GetHeight() / 2));

    // Set random rotation in degrees and random scale
    sprite->SetRotation(Random() * 360.0f);

    // Set random color and additive blending mode
    sprite->SetColor(Color(Random(0.5f) + 0.5f, Random(0.5f) + 0.5f, Random(0.5f) + 0.5f));
    sprite->SetBlendMode(BLEND_ADD);


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

    /*Text* text = ui->GetRoot()->CreateChild<Text>();
    text->SetHorizontalAlignment(HA_RIGHT);
    text->SetPosition(IntVector2(-20, -20));
    text->SetVerticalAlignment(VA_BOTTOM);
    text->SetStyleAuto();
    text->SetText("Loading...");

    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
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

    text->SetObjectAnimation(animation);*/
}

void Loading::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Loading, HandleUpdate));
    SubscribeToEvent(StringHash("EndLoading"), URHO3D_HANDLER(Loading, HandleEndLoading));
}

void Loading::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    if (timer.GetMSec(false) > 3000) {
        SendEvent("EndLoading");
        UnsubscribeFromEvent(E_UPDATE);
    }
}

void Loading::HandleEndLoading(StringHash eventType, VariantMap& eventData)
{
	UnsubscribeFromEvent(E_UPDATE);
	VariantMap data = GetEventDataMap();
	data["Name"] = "Level";
    SendEvent(MyEvents::E_SET_LEVEL, data);
}