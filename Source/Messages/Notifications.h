#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

struct NotificationData {
    String message;
    Color color;
};

class Notifications : public Object
{
    URHO3D_OBJECT(Notifications, Object);

public:
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

    void CreateNewNotification(NotificationData data);

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
    Vector<SharedPtr<UIElement>> messages_;
    SharedPtr<ObjectAnimation> notificationAnimation_;
    SharedPtr<ValueAnimation> positionAnimation_;
    SharedPtr<ValueAnimation> opacityAnimation_;
    List<NotificationData> messageQueue_;
    Timer timer_;
};
