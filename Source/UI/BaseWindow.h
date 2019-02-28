#pragma once

#include <Urho3D/Urho3DAll.h>

class BaseWindow : public Object
{
    URHO3D_OBJECT(BaseWindow, Object);
public:
    BaseWindow(Context* context);

    virtual ~BaseWindow();

    /**
     * Initialize the view
     */
    virtual void Init() {}

private:
    /**
     * Create the UI
     */
    virtual void Create() {}

protected:

    /**
     * Get rid of the window
     */
    virtual void Dispose() {}

    /**
     * Creates transparent Sprite in the back
     * All windows should be created as a child for this overlay
     */
    Sprite* CreateOverlay();

    /**
     * Transparent overlay object
     */
    SharedPtr<Sprite> _overlay;
};