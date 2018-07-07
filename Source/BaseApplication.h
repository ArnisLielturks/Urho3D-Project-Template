#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Network/Network.h>
#include "LevelManager.h"
#include "Messages/Message.h"
#include "Messages/Achievements.h"
#include "Messages/Notifications.h"
#include "Mods/ModLoader.h"
#include "UI/WindowManager.h"

using namespace Urho3D;

class BaseApplication : public Application
{
    URHO3D_OBJECT(BaseApplication, Application);

public:
    /// Construct.
    BaseApplication(Context* context);

    /// Setup before engine initialization. Verify that a script file has been specified.
    virtual void Setup() override;
    /// Setup after engine initialization. Load the script and execute its start function.
    virtual void Start() override;
    /// Cleanup after the main loop. Run the script's stop function if it exists.
    virtual void Stop() override;

private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void LoadConfig(String filename, String prefix = "", bool isMain = false);
    void SaveConfig();
    void HandleLoadConfig(StringHash eventType, VariantMap& eventData);
    void HandleSaveConfig(StringHash eventType, VariantMap& eventData);
    void HandleAddConfig(StringHash eventType, VariantMap& eventData);

	void HandleConsoleGlobalVariableChange(StringHash eventType, VariantMap& eventData);

    void RegisterConsoleCommands();

    void SubscribeToEvents();

    void HandleExit(StringHash eventType, VariantMap& eventData);

    /// Flag whether CommandLine.txt was already successfully read.
    bool commandLineRead_;

    SharedPtr<LevelManager> levelManager;
    SharedPtr<Message> _alertMessage;
    SharedPtr<Notifications> _notifications;
    SharedPtr<Achievements> _achievements;
    SharedPtr<ModLoader> _modLoader;
    SharedPtr<WindowManager> _windowManager;
    VariantMap _globalSettings;
};
