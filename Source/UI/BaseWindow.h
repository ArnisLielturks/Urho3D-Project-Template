#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/UI/Sprite.h>

using namespace Urho3D;

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

    /**
     * Set the window parameters
     */
    void SetData(VariantMap data);

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
    SharedPtr<Sprite> overlay_;

    /**
     * Data which was passed when window was opened
     */
    VariantMap data_;
};
