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

	Controls GetControls();

protected:
    virtual void Init();

private:
    void SubscribeToEvents();
	void UnsubscribeToEvents();
	void RegisterConsoleCommands();

	void LoadConfig();
	void SaveConfig();
	void DefaultConfig();

	void HandleStartInputListeningConsole(StringHash eventType, VariantMap& eventData);
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);
	void HandleStartInputListening(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	HashMap<int, int> _mappedControls;

	int _activeAction;

	Urho3D::HashMap<int, String> _controlMapNames;

	Controls _controls;
	Timer _timer;
};