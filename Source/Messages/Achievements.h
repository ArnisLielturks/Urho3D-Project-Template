#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class Achievements : public Object
{
    URHO3D_OBJECT(Achievements, Object);

public:
    /// Construct.
    Achievements(Context* context);

    virtual ~Achievements();

    void Create();

    void Dispose();

    void HandleNewAchievement(StringHash eventType, VariantMap& eventData);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void HandleGameEnd(StringHash eventType, VariantMap& eventData);

    void LoadAchievementList();

protected:
    virtual void Init();

private:

    void SubscribeToEvents();

    WeakPtr<UIElement> _baseElement;

    Vector<SharedPtr<UIElement>> _messages;
};