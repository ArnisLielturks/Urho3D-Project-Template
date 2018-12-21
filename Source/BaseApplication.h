#pragma once

#include <Urho3D/Urho3DAll.h>
#include "LevelManager.h"
#include "Messages/Message.h"
#include "Messages/Achievements.h"
#include "Messages/Notifications.h"
#include "Mods/ModLoader.h"
#include "UI/WindowManager.h"
#include "Config/ConfigManager.h"

using namespace Urho3D;

class BaseApplication : public Application
{
    URHO3D_OBJECT(BaseApplication, Application);

public:
    BaseApplication(Context* context);

    /**
     * Setup before engine initialization
     */
    virtual void Setup() override;
    /**
     * Setup after engine initialization
     */
    virtual void Start() override;

    /**
     * Cleanup after the main loop
     */
    virtual void Stop() override;

private:

    /**
     * Apply specific graphics settings which are not configurable automatically
     */
    void ApplyGraphicsSettings();

    /**
     * Load configuration files
     */
	void LoadINIConfig(String filename);

	/**
	 * Load custom configuration file
	 */
    void LoadConfig(String filename, String prefix = "", bool isMain = false);


    /**
     * Handle event for config loading
     */
    void HandleLoadConfig(StringHash eventType, VariantMap& eventData);

    /**
     * Add global config
     */
    void HandleAddConfig(StringHash eventType, VariantMap& eventData);

	/**
	 * Subscribe to console commands
	 */
    void RegisterConsoleCommands();

    /**
     * Subscribe to config events
     */
    void SubscribeToEvents();

    /**
     * Set engine parameter and make it globally available
     */
    void SetEngineParameter(String parameter, Variant value);

    /**
     * Handle exit via events
     */
    void HandleExit(StringHash eventType, VariantMap& eventData);

    /**
     * Global configuration
     */
    VariantMap _globalSettings;

    /**
     * Main configuration file
     */
    String _configurationFile;
};
