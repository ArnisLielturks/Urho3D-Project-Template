#pragma once

#include <Urho3D/Urho3DAll.h>

#ifndef CONSOLE_CONTENT_LENGTH
#define CONSOLE_CONTENT_LENGTH 1024 * 10
#endif

struct SingleLine {
    String content;
    int logLevel;
};

struct SingleConsoleCommand {
    String command;
    String eventToCall;
    String description;
};

class ConsoleWindow : public Object
{
    URHO3D_OBJECT(ConsoleWindow, Object);

public:
    /// Construct.
    ConsoleWindow(Context* context);

    virtual ~ConsoleWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    void HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData);
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);
    void ParseCommand(String input);

    void HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData);

    Timer _timer;
    HashMap<String, SingleConsoleCommand> _registeredConsoleCommands;

    Console* _console;
};