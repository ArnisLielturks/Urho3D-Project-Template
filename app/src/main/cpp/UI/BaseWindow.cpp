#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include "BaseWindow.h"
#include "../CustomEvents.h"

using namespace CustomEvents;

BaseWindow::BaseWindow(Context* context):
    Object(context)
{
    Init();
}

BaseWindow::~BaseWindow()
{
    Dispose();
    if (overlay_) {
        overlay_->Remove();
    }
}

Sprite* BaseWindow::CreateOverlay()
{
    if (overlay_) {
        overlay_->Remove();
    }
    auto* cache = GetSubsystem<ResourceCache>();

    overlay_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Sprite>();
    overlay_->SetEnabled(true);
    overlay_->SetPosition(0, 0);
    overlay_->SetTexture(cache->GetResource<Texture2D>("Textures/Transparent.png"));
    overlay_->SetFixedWidth(GetSubsystem<Graphics>()->GetWidth() / GetSubsystem<UI>()->GetScale());
    overlay_->SetFixedHeight(GetSubsystem<Graphics>()->GetHeight() / GetSubsystem<UI>()->GetScale());
    overlay_->SetBlendMode(BlendMode::BLEND_ALPHA);

    // In cases where resolution is changed, we have to resize the overlay to match the new resolution
    SubscribeToEvent(E_VIDEO_SETTINGS_CHANGED, [&](StringHash eventType, VariantMap& eventData) {
        overlay_->SetFixedWidth(GetSubsystem<Graphics>()->GetWidth() / GetSubsystem<UI>()->GetScale());
        overlay_->SetFixedHeight(GetSubsystem<Graphics>()->GetHeight() / GetSubsystem<UI>()->GetScale());
    });

    return overlay_;
}

void BaseWindow::SetData(VariantMap data)
{
    data_ = data;
}
