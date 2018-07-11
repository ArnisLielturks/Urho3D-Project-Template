#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"
#include "../Config/ConfigFile.h"

class ControllerInput : public Object
{
    URHO3D_OBJECT(ControllerInput, Object);

public:
    /// Construct.
    ControllerInput(Context* context);

    virtual ~ControllerInput();

	Controls GetControls();

	HashMap<int, String> GetControlNames();

	String GetActionKeyName(int action);

protected:
    virtual void Init();

private:
    void SubscribeToEvents();
	void UnsubscribeToEvents();
	void RegisterConsoleCommands();

	void CreateConfigMaps();
	void LoadConfig();
	void SaveConfig();
	void DefaultConfig();

	void ReleaseConfiguredKey(int key, int action);
	void SetConfiguredKey(int action, int key, String controller);

	void HandleStartInputListeningConsole(StringHash eventType, VariantMap& eventData);
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);

	void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData);
	void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

	void HandleStartInputListening(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	// Control against keyboard key map
	HashMap<int, int> _mappedKeyboardControlsToKeys;
	// Keyboard key against control map
	HashMap<int, int> _mappedKeyboardKeysToControls;

	// Control against mouse key map
	HashMap<int, int> _mappedMouseControlsToKeys;
	// Mouse key against control map
	HashMap<int, int> _mappedMouseKeysToControls;


	// Control names
	HashMap<int, String> _controlMapNames;

	int _activeAction;
	SharedPtr<ConfigFile> _configFile;

	Controls _controls;
	Timer _timer;
};