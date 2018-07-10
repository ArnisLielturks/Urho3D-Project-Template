#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"


class ControllerInput : public Object
{
    URHO3D_OBJECT(ControllerInput, Object);

public:
    /// Construct.
    ControllerInput(Context* context);

    virtual ~ControllerInput();

protected:
    virtual void Init();

private:
    void SubscribeToEvents();
	void UnsubscribeToEvents();

	void LoadConfig();
	void SaveConfig();
	void DefaultConfig();

	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleStartInputListening(StringHash eventType, VariantMap& eventData);

	HashMap<int, int> _mappedControls;

	int _activeAction;

	Urho3D::HashMap<int, String> _controlMapNames;
};