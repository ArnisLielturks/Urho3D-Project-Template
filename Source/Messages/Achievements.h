#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class SingleAchievement : public Animatable
{
    URHO3D_OBJECT(SingleAchievement, Animatable);
public:
    /// Construct.
    SingleAchievement(Context* context);

    virtual ~SingleAchievement();

    static void RegisterObject(Context* context);

    /**
     * Set achievement image
     */
    void SetImage(String image);

    /**
     * Set achievement message
     */
    void SetMessage(String message);

    /**
     * Get achievement message
     */
    String GetMessage();

    void SetVerticalPosition(int position);

    void SetVar(StringHash key, const Variant& value);
    const Variant& GetVar(const StringHash& key) const;
private:
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle attribute animation added.
    void OnAttributeAnimationAdded() override;
    /// Handle attribute animation removed.
    void OnAttributeAnimationRemoved() override;

    float _offset;
    String _message;
    VariantMap vars_;
    int _verticalPos;

    SharedPtr<Window> _baseWindow;
    SharedPtr<Sprite> _sprite;
    SharedPtr<Text> _title;
};

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

    List<SharedPtr<SingleAchievement>> _activeAchievements;
};