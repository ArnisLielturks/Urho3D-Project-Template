#include <Urho3D/Urho3DAll.h>
#include "Notifications.h"
#include "../Global.h"

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

    float fontSize = 12.0f;

    // Create light animation
    notificationAnimation = new ObjectAnimation(context_);

    // Create light position animation
    positionAnimation = new ValueAnimation(context_);
    // Use spline interpolation method
    positionAnimation->SetInterpolationMethod(IM_SPLINE);
    // Set spline tension
    positionAnimation->SetSplineTension(0.7f);
    positionAnimation->SetKeyFrame(0.0f, IntVector2(-10, -300));
    positionAnimation->SetKeyFrame(4.0f, IntVector2(-10, -500));
    notificationAnimation->AddAttributeAnimation("Position", positionAnimation);

    opacityAnimation = new ValueAnimation(context_);
    opacityAnimation->SetKeyFrame(0.0f, 0.0f);
    opacityAnimation->SetKeyFrame(0.2f, 1.0f);
    opacityAnimation->SetKeyFrame(3.0f, 1.0f);
    opacityAnimation->SetKeyFrame(3.2f, 0.0f);
    opacityAnimation->SetKeyFrame(10.0f, 0.0f);
    notificationAnimation->AddAttributeAnimation("Opacity", opacityAnimation);
}

void Notifications::SubscribeToEvents()
{
    SubscribeToEvent("ShowNotification", URHO3D_HANDLER(Notifications, HandleNewNotification));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Notifications, HandleUpdate));
    SubscribeToEvent(StringHash("ClientGameRoundEnd"), URHO3D_HANDLER(Notifications, HandleGameEnd));
}

void Notifications::HandleNewNotification(StringHash eventType, VariantMap& eventData)
{
    float fontSize = 12.0f;
    auto* cache = GetSubsystem<ResourceCache>();

    String message = eventData["Message"].GetString();
    // Construct new Text object
    SharedPtr<Text> messageElement(GetSubsystem<UI>()->GetRoot()->CreateChild<Text>());
    // Set String to display
    messageElement->SetText(message);
    messageElement->SetTextEffect(TextEffect::TE_STROKE);
    messageElement->SetStyleAuto();

    auto *font = cache->GetResource<Font>(APPLICATION_FONT);
    messageElement->SetColor(Color(0.0f, 1.0f, 0.0f));
    messageElement->SetFont(font, fontSize);

    // Align Text center-screen
    messageElement->SetHorizontalAlignment(HA_RIGHT);
    messageElement->SetVerticalAlignment(VA_BOTTOM);

    messageElement->SetObjectAnimation(notificationAnimation);
    messageElement->SetVar("Lifetime", 4.0f);

    _messages.Push(messageElement);
}

void Notifications::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    for (auto it = _messages.Begin(); it != _messages.End(); ++it) {
        if (!(*it)) {
            _messages.Erase(it);
            return;
        }
        float lifetime = (*it)->GetVar("Lifetime").GetFloat();
        if (lifetime <= 0) {
            (*it)->Remove();
            _messages.Erase(it);
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