#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

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

class ConsoleWindow : public BaseWindow
{
    URHO3D_OBJECT(ConsoleWindow, BaseWindow);

public:
    /// Construct.
    ConsoleWindow(Context* context);

    virtual ~ConsoleWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleUpdate(StringHash eventType, VariantMap& eventData) override;
    void HandleLogMessage(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    void HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData);
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);
    void ParseCommand(String input);

    void HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData);

    void AddContent(String message, int level);

    char _content[CONSOLE_CONTENT_LENGTH];
    int _contentLength;

    char _consoleInput[256];
    int _consoleInputLength;

    Timer _timer;

    List<SingleLine> _lines;

    HashMap<String, SingleConsoleCommand> _registeredConsoleCommands;
};