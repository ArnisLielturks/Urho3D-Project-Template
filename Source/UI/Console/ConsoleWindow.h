#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

#ifndef CONSOLE_CONTENT_LENGTH
#define CONSOLE_CONTENT_LENGTH 1024 * 10
#endif

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

    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLogMessage(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    void AddContent(String message);

    char _content[CONSOLE_CONTENT_LENGTH];
    int _contentLength;

    char _consoleInput[256];
    int _consoleInputLength;

    List<String> _lines;
};