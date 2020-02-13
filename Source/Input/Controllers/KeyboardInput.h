#pragma once

#include "BaseInput.h"
#include "../../Config/ConfigFile.h"

class KeyboardInput : public BaseInput
{
    URHO3D_OBJECT(KeyboardInput, BaseInput);

public:
    /// Construct.
    KeyboardInput(Context* context);

    virtual ~KeyboardInput();
	virtual String GetActionKeyName(int action) override;

protected:
    virtual void Init();

private:
    void SubscribeToEvents();

	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);
};