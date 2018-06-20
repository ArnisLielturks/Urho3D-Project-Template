#include <Urho3D/Urho3DAll.h>
#include "Splash.h"
#include "../MyEvents.h"

using namespace Levels;

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
	URHO3D_LOGRAW("Starting level: Splash");
	BaseLevel::Init();

	// Create the scene content
	CreateScene();

	// Create the UI content
	CreateUI();

	// Subscribe to global events for camera movement
	SubscribeToEvents();

	VariantMap data;
	data["Message"] = "Saw splash screen";
	SendEvent("NewAchievement", data);
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

	Text* text = ui->GetRoot()->CreateChild<Text>();
	text->SetHorizontalAlignment(HA_RIGHT);
	text->SetPosition(IntVector2(-20, -20));
	text->SetVerticalAlignment(VA_BOTTOM);
	text->SetStyleAuto();
	text->SetText("Splash...");

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

	text->SetObjectAnimation(animation);
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
	if (_timer.GetMSec(false) > 2000) {
		HandleEndSplash();
	}
}

void Splash::HandleEndSplash()
{
	data_[MyEvents::E_SET_LEVEL] = "MainMenu";
	SendEvent(MyEvents::E_SET_LEVEL, data_);
	UnsubscribeFromEvent(E_UPDATE);
}