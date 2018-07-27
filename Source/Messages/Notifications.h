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

    void Create();

    void Dispose();

    void HandleNewNotification(StringHash eventType, VariantMap& eventData);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void HandleGameEnd(StringHash eventType, VariantMap& eventData);

protected:
    virtual void Init();

private:

    void SubscribeToEvents();

    UIElement* _baseElement;

    List<WeakPtr<Text>> _messages;
};