#pragma once

#include <Urho3D/Urho3DAll.h>

struct SingleConsoleCommand {
    String command;
    String eventToCall;
    String description;
};

class ConsoleHandler : public Object
{
    URHO3D_OBJECT(ConsoleHandler, Object);

public:
    /// Construct.
    ConsoleHandler(Context* context);

    virtual ~ConsoleHandler();

    virtual void Init();

    virtual void Create();

private:

    /**
     * Subscribe console related events
     */
    void SubscribeToEvents();

    /**
     * Toggle console
     */
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    /**
     * Add new console command
     */
    void HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData);

    /**
     * Process incomming console commands
     */
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

    /**
     * Parse incomming command input
     */
    void ParseCommand(String input);

    /**
     * Display help
     */
    void HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData);

    /**
     * Handle configuration change via console
     */
    void HandleConsoleGlobalVariableChange(StringHash eventType, VariantMap& eventData);

    /**
     * Registered console commands
     */
    HashMap<String, SingleConsoleCommand> _registeredConsoleCommands;

    /**
     * Console handler in the engine
     */
    SharedPtr<Console> _console;
};