#pragma once

#include <Urho3D/Urho3DAll.h>
#include "MyEvents.h"

class LevelManager : public Object
{
    URHO3D_OBJECT(LevelManager, Object);
public:
    LevelManager(Context* context);

    ~LevelManager();

private:
    void RegisterAllFactories();

    void HandleSetLevelQueue(StringHash eventType, VariantMap& eventData);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void DrawFade(float opacity);

    List<String> level_queue_;
    SharedPtr<Object> level_;
    VariantMap data_;
    float fade_time_;
    int fade_status_;
    const float MAX_FADE_TIME = 1.0f;
    String currentLevel_;
    String previousLevel_;
};