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

protected:
    virtual void Init();

private:
    void SubscribeToEvents();

	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);
	void HandleAxisMove(StringHash eventType, VariantMap& eventData);
	void HandleHatMove(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	Vector2 _axisPosition;
};