#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../UI/NuklearUI.h"

using namespace Urho3D;

class SingleAchievement : public Animatable
{
    URHO3D_OBJECT(SingleAchievement, Animatable);
public:
    /// Construct.
    SingleAchievement(Context* context);

    virtual ~SingleAchievement();

    static void RegisterObject(Context* context);

    void SetImage(String image);
    void SetMessage(String message);

    void SetVar(StringHash key, const Variant& value);
    const Variant& GetVar(const StringHash& key) const;
private:
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle attribute animation added.
    void OnAttributeAnimationAdded() override;
    /// Handle attribute animation removed.
    void OnAttributeAnimationRemoved() override;

    float _size;
    String _message;
    SharedPtr<Texture2D> _imageTexture;
    struct nk_image _image{};
    VariantMap vars_;
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

    Vector<SharedPtr<SingleAchievement>> _activeAchievements;
};