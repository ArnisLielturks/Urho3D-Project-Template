#include <Urho3D/Urho3DAll.h>
#include "Loading.h"
#include "../MyEvents.h"

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
	URHO3D_LOGRAW("Starting level: Loading");
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

	Text* text = ui->GetRoot()->CreateChild<Text>();
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

	text->SetObjectAnimation(animation);
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
	data_[MyEvents::E_SET_LEVEL] = "Level";
	SendEvent(MyEvents::E_SET_LEVEL, data_);
	UnsubscribeFromEvent(E_UPDATE);
}