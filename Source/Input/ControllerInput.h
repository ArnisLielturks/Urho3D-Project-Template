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
	 * If multiple controller support is disabled, only the
	 * controls with index 0 will be returned
	 */
	Controls GetControls(int index = 0);

	/**
	 * Get a vector of all the controller indexes
	 * 0: mouse/keyboard/first joystick
	 * 1 - N: All the additional joysticks or other controllers
	 */
	Vector<int> GetControlIndexes();

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
	void SetActionState(int action, bool active, int index = 0);

	/**
	 * Update Yaw value for specific controller
	 */
	void UpdateYaw(float yaw, int index = 0);

    /**
     * Update Pitch value for specific controller
     */
	void UpdatePitch(float pitch, int index = 0);

	/**
	 * Create new controls for specific controller
	 * This allows multiple `Controls` objects to be created for each controller
	 */
	void CreateController(int controllerIndex);

	/**
	 * Destroy created controller slot
	 */
	void DestroyController(int controllerIndex);

	/**
	 * Clear action and key from configuration mapping
	 * Used before assigning new key to control
	 */
	void ReleaseConfiguredKey(int key, int action);

	/**
	 * Map specific key to specific action
	 */
	void SetConfiguredKey(int action, int key, String controller);

	/**
	 * Enable/disable multiple controls support
	 * This defines if each joystick should have it's own controls or not
	 */
	void SetMultipleControllerSupport(bool enabled);

	/**
	 * Get Multiple controller support
	 */
	bool GetMultipleControllerSupport() { return _multipleControllerSupport; }

	/**
	 * Detect if mapping is in progress
	 */
	bool IsMappingInProgress();

    /**
    * Load INI configuration file from config.cfg
    */
    void LoadConfig();

    /**
     * Set joystick as first controller
     * This means that 1st joystick and keyboard/mouse shares the same controlled entity
     * Ignored if `SetMultipleControllerSupport` is set as `false`
     */
    void SetJoystickAsFirstController(bool enabled);

	/**
     * Is joystick set as first controller
     */
	bool GetJoystickAsFirstController();

    /**
     * Set inverted X axis for specific controller
     */
    void SetInvertX(bool enabled, int controller);

    /**
     * Get inverted X axis value for specific controller
     */
    bool GetInvertX(int controller);

    /**
     * Set inverted Y axis for specific controller
     */
    void SetInvertY(bool enabled, int controller);

    /**
     * Get inverted Y axis value for specific controller
     */
    bool GetInvertY(int controller);

    /**
     * Set X axis sensitivity for specific controller
     */
    void SetSensitivityX(float value, int controller);

    /**
     * Set Y axis sensitivity for specific controller
     */
    void SetSensitivityY(float value, int controller);

    /**
     * Get X axis sensitivity for specific controller
     */
    float GetSensitivityX(int controller);

    /**
     * Get Y axis sensitivity for specific controller
     */
    float GetSensitivityY(int controller);

    /**
     * Stop input mapping process if it has been started
     */
    void StopInputMapping();

protected:
    virtual void Init();

private:
	/**
	 * Subscribe for input mapping events
	 */
    void SubscribeToEvents();

	/**
	 * Register input mapping via console
	 */
	void RegisterConsoleCommands();

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

	/**
	 * Action key to string map
	 */
	HashMap<int, String> _controlMapNames;

	/**
	 * Active controls
	 */
	HashMap<int, Controls> _controls;

	/**
	 * All input handlers
	 */
	HashMap<int, SharedPtr<BaseInput>> _inputHandlers;

	/**
	 * Multiple controller support
	 */
	bool _multipleControllerSupport;

	/**
	 * Currently active action for input mapping
	 */
	int _activeAction;
};