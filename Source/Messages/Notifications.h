#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class Notifications : public Object
{
    URHO3D_OBJECT(Notifications, Object);

public:
    /// Construct.
    Notifications(Context* context);

    virtual ~Notifications();

private:
    virtual void Init();

    /**
     * Subscribe to notification events
     */
    void SubscribeToEvents();

    /**
     * Handle ShowNotification event
     */
    void HandleNewNotification(StringHash eventType, VariantMap& eventData);

    /**
     * Handle message displaying and animations
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Handle game end event
     */
    void HandleGameEnd(StringHash eventType, VariantMap& eventData);

    /**
     * List of all active messages
     */
    Vector<SharedPtr<Text>> _messages;
    SharedPtr<ObjectAnimation> notificationAnimation;
    SharedPtr<ValueAnimation> positionAnimation;
    SharedPtr<ValueAnimation> opacityAnimation;
};