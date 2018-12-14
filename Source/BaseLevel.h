#pragma once

#include <Urho3D/Urho3DAll.h>
#include "MyEvents.h"

class BaseLevel : public Object
{
    URHO3D_OBJECT(BaseLevel, Object);
public:
    BaseLevel(Context* context);

    virtual ~BaseLevel();

private:
    void SubscribeToBaseEvents();

    void HandleStart(StringHash eventType, VariantMap& eventData);

    void HandleLevelLoaded(StringHash eventType, VariantMap& eventData);

protected:

    /**
     * Initialize the scene
     */
    virtual void Init() {}

    /**
     * Trigger when level manager has loaded the level
     */
    virtual void OnLoaded() {}

    /**
     * Start scene updates
     */
    virtual void Run();

    /**
     * Pause scene updates
     */
    virtual void Pause();

    void SubscribeToEvents();

    /**
     * Handle FOV change for all cameras
     */
    void HandleFovChange(StringHash eventType, VariantMap& eventData);

    /**
     * Get rid of this level
     */
    virtual void Dispose();

    /**
     * Define rects for splitscreen mode
     */
    Vector<IntRect> InitRects(int count);

    /**
     * Create viewports and cameras based on controller count
     */
    void InitViewports(Vector<int> playerIndexes);

    /**
     * Level scene
     */
    SharedPtr<Scene> scene_;

    /**
     * Data which was passed trough LevelManager
     */
    VariantMap data_;

    /**
     * All available viewports in the scene
     * mapped against specific controller
     */
    HashMap<int, SharedPtr<Viewport>> _viewports;

    /**
     * All available cameras in the scene
     * mapped against specific controller
     */
    HashMap<int, SharedPtr<Node>> _cameras;
};