#pragma once

#include <Urho3D/Urho3DAll.h>
#include "BaseWindow.h"

using namespace Urho3D;

class WindowManager : public Object
{
    URHO3D_OBJECT(WindowManager, Object);

public:
    /// Construct.
    WindowManager(Context* context);

    virtual ~WindowManager();

    /**
     * Is specific window is already opened
     */
    bool IsWindowOpen(String windowName);

private:

    void RegisterAllFactories();

    void SubscribeToEvents();

    /**
     * Create specific window via event
     */
    void HandleOpenWindow(StringHash eventType, VariantMap& eventData);

    /**
     * Prepare window for closing. Adds window to the queue for windows which should
     * be destroyed in the next frame
     */
    void HandleCloseWindow(StringHash eventType, VariantMap& eventData);

    /**
     * Close all active windows
     */
    void HandleCloseAllWindows(StringHash eventType, VariantMap& eventData);

    /**
     * Check whether there are windows which should be closed.
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Destroy specific window
     */
    void CloseWindow(String windowName);

    /**
     * List of all active windows objects
     */
    List<SharedPtr<Object>> _windowList;

    /**
     * List of opened windows
     */
    List<String> _openedWindows;

    /**
     * Window list which should be destroyed in the next frame
     */
    List<String> _closeQueue;
};