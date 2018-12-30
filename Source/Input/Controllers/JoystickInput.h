#pragma once

#include <Urho3D/Urho3DAll.h>
#include "BaseInput.h"
#include "../../Config/ConfigFile.h"

class JoystickInput : public BaseInput
{
    URHO3D_OBJECT(JoystickInput, BaseInput);

public:
    /// Construct.
    JoystickInput(Context* context);

    virtual ~JoystickInput();
	virtual String GetActionKeyName(int action);

    void SetJoystickAsFirstController(bool enabled);

	/**
     * Load joystick config from config.cfg file [joystick] block
     */
    void LoadConfig();

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

	HashMap<int, Vector2> _axisPosition;

    bool _joystickAsFirstController;

    // x - move left/right
    // y - move forward/bacward
    // z - rotate x 
    // w - rotate y 
    Vector4 _joystickMapping;
};