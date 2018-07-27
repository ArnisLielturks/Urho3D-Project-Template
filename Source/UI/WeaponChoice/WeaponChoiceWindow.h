#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

class WeaponChoiceWindow : public BaseWindow
{
    URHO3D_OBJECT(WeaponChoiceWindow, BaseWindow);

public:
    /// Construct.
    WeaponChoiceWindow(Context* context);

    virtual ~WeaponChoiceWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleUpdate(StringHash eventType, VariantMap& eventData) override;

    bool _active;

    Vector<SharedPtr<Texture2D>> _imageTextures;
};