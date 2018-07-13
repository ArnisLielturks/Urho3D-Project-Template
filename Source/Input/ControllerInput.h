#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../Config/ConfigManager.h"
#include "Controllers/BaseInput.h"

/**
 * Different types of input handlers
 */
enum ControllerType {
	KEYBOARD,
	MOUSE,
	JOYSTICK
};

class ControllerInput : public Object
{
    URHO3D_OBJECT(ControllerInput, Object);

public:
    /// Construct.
    ControllerInput(Context* context);

    virtual ~ControllerInput();

	/**
	 * Get the already processed controls
	 */
	Controls GetControls();

	/**
	 * Get mapping against all controls
	 */
	HashMap<int, String> GetControlNames();

	/**
	 * Retrieve key name for specific action
	 */
	String GetActionKeyName(int action);

	/**
	 * Set the controls action
	 */
	void SetActionState(int action, bool active);

	/**
	 * Clear action and key from configuration mapping
	 * Used before assigning new key to control
	 */
	void ReleaseConfiguredKey(int key, int action);

	/**
	 * Map specific key to specific action
	 */
	void SetConfiguredKey(int action, int key, String controller);

protected:
    virtual void Init();

private:
    void SubscribeToEvents();
	void UnsubscribeToEvents();
	void RegisterConsoleCommands();

	/**
	 * Load INI configuration file
	 */
	void LoadConfig();

	/**
	 * Save INIT configuration file
	 */
	void SaveConfig();

	/**
	 * Start mapping specific action from console command
	 */
	void HandleStartInputListeningConsole(StringHash eventType, VariantMap& eventData);

	/**
	 * Start mapping specific action
	 */
	void HandleStartInputListening(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	/**
	 * Action key to string map
	 */
	HashMap<int, String> _controlMapNames;

	/**
	 * Loaded configuration file
	 */
	SharedPtr<ConfigManager> _configManager;

	/**
	 * Active controls
	 */
	Controls _controls;

	/**
	 * All input handlers
	 */
	HashMap<int, SharedPtr<BaseInput>> _inputHandlers;

	/**
	 * Filepath + filename to the configuration file
	 */
	String _configurationFile;
};