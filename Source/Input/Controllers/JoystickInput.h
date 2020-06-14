#pragma once

#include "BaseInput.h"
#include "../../Config/ConfigFile.h"

class JoystickInput : public BaseInput
{
    URHO3D_OBJECT(JoystickInput, BaseInput);

public:
    JoystickInput(Context* context);

    virtual ~JoystickInput();
    virtual String GetActionKeyName(int action) override;

    void SetJoystickAsFirstController(bool enabled);

    bool GetJoystickAsFirstController();

    /**
     * Load joystick config from config.cfg file [joystick] block
     */
    void LoadConfig() override;

protected:
    virtual void Init();

private:
    void SubscribeToEvents();

    void HandleJoystickConnected(StringHash eventType, VariantMap& eventData);
    void HandleJoystickDisconnected(StringHash eventType, VariantMap& eventData);

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleAxisMove(StringHash eventType, VariantMap& eventData);
    void HandleHatMove(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    HashMap<int, Vector2> axisPosition_;

    bool joystickAsFirstController_;

    // x - move left/right
    // y - move forward/bacward
    // z - rotate x 
    // w - rotate y 
    Vector4 joystickMapping_;
};
