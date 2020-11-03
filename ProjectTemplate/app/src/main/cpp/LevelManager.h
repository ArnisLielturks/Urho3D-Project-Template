#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/List.h>
#include <Urho3D/UI/Window.h>

using namespace Urho3D;

class LevelManager : public Object
{
    URHO3D_OBJECT(LevelManager, Object);
public:
    LevelManager(Context* context);
    ~LevelManager();

    static void RegisterObject(Context* context);

    const String& GetCurrentLevel() const { return currentLevel_; }
private:
    /**
     * Level changing handler
     */
    void HandleSetLevelQueue(StringHash eventType, VariantMap& eventData);

    /**
     * Fade status update
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Create the fade layer
     */
    void AddFadeLayer();

    /**
     * Level queue
     */
    List<String> level_queue_;

    /**
     * Currently active level
     */
    SharedPtr<Object> level_;

    /**
     * UI element for the fading
     */
    SharedPtr<Window> fade_window_;

    /**
     * Event data which is passed when changing levels
     */
    VariantMap data_;

    /**
     * Current fade time
     */
    float fade_time_;

    /**
     * Current fade status
     */
    int fade_status_;

    /**
     * How fast the fade in/out effect should last
     */
    const float MAX_FADE_TIME = 0.3f;

    /**
     * Name of the current level
     */
    String currentLevel_;

    /**
     * Name of the previous level
     */
    String previousLevel_;
};
