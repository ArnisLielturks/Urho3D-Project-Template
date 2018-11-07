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

protected:
    virtual void Init();

private:

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
    List<WeakPtr<Text>> _messages;
};