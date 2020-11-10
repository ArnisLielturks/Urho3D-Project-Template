#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/Font.h>
#include "Achievements.h"
#include "../Globals/GUIDefines.h"

void SingleAchievement::RegisterObject(Context* context)
{
    context->RegisterFactory<SingleAchievement>();
    URHO3D_ATTRIBUTE("Offset", float, offset_, 1, AM_FILE);
}

SingleAchievement::SingleAchievement(Context* context) :
        Animatable(context)
{
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SingleAchievement, HandlePostUpdate));

    auto *cache = GetSubsystem<ResourceCache>();
    auto *font = cache->GetResource<Font>(APPLICATION_FONT);

    baseWindow_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_LEFT, VA_BOTTOM);
    baseWindow_->SetSize(300, 100);
    baseWindow_->BringToFront();

    baseWindow_->SetBringToBack(true);

    sprite_ = baseWindow_->CreateChild<Sprite>();
    sprite_->SetSize(80, 80);
    sprite_->SetAlignment(HA_LEFT, VA_TOP);
    sprite_->SetPosition(10, 10);

    title_ = baseWindow_->CreateChild<Text>();
    title_->SetFont(font, 10);
    title_->SetAlignment(HA_LEFT, VA_CENTER);
    title_->SetPosition(100, 0);
    title_->SetWordwrap(true);
    title_->SetWidth(baseWindow_->GetWidth() - sprite_->GetWidth() - 20);
}

SingleAchievement::~SingleAchievement()
{
    baseWindow_->Remove();
    UnsubscribeFromEvent(E_POSTUPDATE);
}

void SingleAchievement::SetImage(String image)
{
    auto* cache = GetSubsystem<ResourceCache>();
    sprite_->SetTexture(cache->GetResource<Texture2D>(image));
}

void SingleAchievement::SetMessage(String message) {
    message_ = "";
    title_->SetText(message);
}

String SingleAchievement::GetMessage()
{
    return message_;
}

void SingleAchievement::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace PostUpdate;

    UpdateAttributeAnimations(eventData[P_TIMESTEP].GetFloat());

    baseWindow_->SetPosition(offset_, -10);
}

void SingleAchievement::OnAttributeAnimationAdded()
{
    if (attributeAnimationInfos_.Size() == 1)
        SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SingleAchievement, HandlePostUpdate));
}

void SingleAchievement::OnAttributeAnimationRemoved()
{
    if (attributeAnimationInfos_.Empty())
        UnsubscribeFromEvent(E_POSTUPDATE);
}

void SingleAchievement::SetVar(StringHash key, const Variant& value)
{
    vars_[key] = value;
}

const Variant& SingleAchievement::GetVar(const StringHash& key) const
{
    VariantMap::ConstIterator i = vars_.Find(key);
    return i != vars_.End() ? i->second_ : Variant::EMPTY;
}
