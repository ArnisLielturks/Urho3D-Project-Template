#pragma once

#include <Urho3D/UI/UIElement.h>
#include "BaseInput.h"
#include "../../Config/ConfigFile.h"

class ScreenJoystickInput : public BaseInput
{
    URHO3D_OBJECT(ScreenJoystickInput, BaseInput);

public:
    ScreenJoystickInput(Context* context);

    virtual ~ScreenJoystickInput();

    void SetJoystickAsFirstController(bool enabled);

    bool GetJoystickAsFirstController();

    /**
     * Load joystick config from config.cfg file [joystick] block
     */
    void LoadConfig() override;

    void Show() override;
    void Hide() override;

protected:
    virtual void Init();

private:
    void SubscribeToEvents();

    void HandleJoystickDrag(StringHash eventType, VariantMap& eventData);
    void HandleJumpPress(StringHash eventType, VariantMap& eventData);
    void HandleJumpRelease(StringHash eventType, VariantMap& eventData);
    void HandleSettings(StringHash eventType, VariantMap& eventData);
    void HandleScreenJoystickTouch(StringHash eventType, VariantMap& eventData);
    void HandleScreenJoystickTouchEnd(StringHash eventType, VariantMap& eventData);

    HashMap<int, Vector2> axisPosition_;

    // x - move left/right
    // y - move forward/bacward
    // z - rotate x 
    // w - rotate y 
    Vector4 joystickMapping_;

    WeakPtr<UIElement> leftAxis_;
    WeakPtr<UIElement> leftAxisInner_;
    Vector2 inputValue_;
    SharedPtr<UIElement> screenJoystick_;
    SharedPtr<UIElement> settingsButton_;
    SharedPtr<UIElement> jumpButton_;
};
