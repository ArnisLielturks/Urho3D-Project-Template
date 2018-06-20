#include <Urho3D/Urho3DAll.h>
#include "Notifications.h"

/// Construct.
Notifications::Notifications(Context* context) :
	Object(context)
{
	Init();
}

Notifications::~Notifications()
{
}

void Notifications::Init()
{
	SubscribeToEvents();
}

void Notifications::Create()
{
	UI* ui = GetSubsystem<UI>();

	_baseElement = ui->GetRoot()->CreateChild("Menu");
	SubscribeToEvents();
}

void Notifications::SubscribeToEvents()
{
	SubscribeToEvent("ShowNotification", URHO3D_HANDLER(Notifications, HandleNewNotification));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Notifications, HandleUpdate));
	SubscribeToEvent(StringHash("ClientGameRoundEnd"), URHO3D_HANDLER(Notifications, HandleGameEnd));
}

void Notifications::Dispose()
{
}

void Notifications::HandleNewNotification(StringHash eventType, VariantMap& eventData)
{
	auto* cache = GetSubsystem<ResourceCache>();

	String message = eventData["Message"].GetString();
	// Construct new Text object
	WeakPtr<Text> messageElement(GetSubsystem<UI>()->GetRoot()->CreateChild<Text>());

	// Set String to display
	messageElement->SetText(message);

	// Set font and text color
	messageElement->SetFont(cache->GetResource<Font>("Fonts/PainttheSky-Regular.otf"), 30);
	messageElement->SetColor(Color(0.0f, 1.0f, 0.0f));

	// Align Text center-screen
	messageElement->SetHorizontalAlignment(HA_RIGHT);
	messageElement->SetVerticalAlignment(VA_BOTTOM);


	// Create light animation
	SharedPtr<ObjectAnimation> notificationAnimation(new ObjectAnimation(context_));

	// Create light position animation
	SharedPtr<ValueAnimation> positionAnimation(new ValueAnimation(context_));
	// Use spline interpolation method
	positionAnimation->SetInterpolationMethod(IM_SPLINE);
	// Set spline tension
	positionAnimation->SetSplineTension(0.7f);
	positionAnimation->SetKeyFrame(0.0f, IntVector2(-10, -10));
	positionAnimation->SetKeyFrame(2.0f, IntVector2(-10, -200));
	notificationAnimation->AddAttributeAnimation("Position", positionAnimation);

	messageElement->SetObjectAnimation(notificationAnimation);
	messageElement->SetVar("Lifetime", 1.0f);

	_messages.Push(messageElement);
}

void Notifications::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();
	for (auto it = _messages.Begin(); it != _messages.End(); ++it) {
		if ((*it).Refs() == 0) {
			_messages.Remove((*it));
			return;
		}
		float lifetime = (*it)->GetVar("Lifetime").GetFloat();
		if (lifetime <= 0) {
			(*it)->Remove();
			_messages.Remove((*it));
			return; 
		}
		lifetime -= timeStep;
		(*it)->SetVar("Lifetime", lifetime);
	}
}

void Notifications::HandleGameEnd(StringHash eventType, VariantMap& eventData)
{
	_messages.Clear();
}