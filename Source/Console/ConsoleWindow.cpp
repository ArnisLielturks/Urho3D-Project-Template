#include <Urho3D/Urho3DAll.h>
#include "ConsoleWindow.h"
#include "../MyEvents.h"

/// Construct.
ConsoleWindow::ConsoleWindow(Context* context) :
    Object(context)
{
    Init();
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ConsoleWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
    // Get default style
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Create console
    _console = GetSubsystem<Engine>()->CreateConsole();
    _console->SetDefaultStyle(xmlFile);
    _console->GetBackground()->SetOpacity(0.8f);
    _console->SetNumHistoryRows(1000);
    _console->SetNumBufferedRows(100);
}

void ConsoleWindow::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ConsoleWindow, HandleKeyDown));
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(ConsoleWindow, HandleConsoleCommand));
    SubscribeToEvent(MyEvents::E_CONSOLE_COMMAND_ADD, URHO3D_HANDLER(ConsoleWindow, HandleConsoleCommandAdd));

    VariantMap data;
    data["ConsoleCommandName"] = "help";
    data["ConsoleCommandEvent"] = "ConsoleHelp";
    data["ConsoleCommandDescription"] = "Displays all available commands";
    SendEvent("ConsoleCommandAdd", data);
    SubscribeToEvent("ConsoleHelp", URHO3D_HANDLER(ConsoleWindow, HandleConsoleCommandHelp));

}

void ConsoleWindow::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_F2) {
        _console->Toggle();
    }

}

void ConsoleWindow::HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ConsoleCommandAdd;
    String command = eventData[P_NAME].GetString();
    String eventToCall = eventData[P_EVENT].GetString();
    String description = eventData[P_DESCRIPTION].GetString();
    if (_registeredConsoleCommands.Contains(command)) {
        URHO3D_LOGERRORF("Console command '%s' already registered!", command.CString());
    }

    // Register new console command
    SingleConsoleCommand singleConsoleCommand;
    singleConsoleCommand.command = command;
    singleConsoleCommand.description = description;
    singleConsoleCommand.eventToCall = eventToCall;
    _registeredConsoleCommands[command] = singleConsoleCommand;
}

void ConsoleWindow::HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    String input = eventData[P_COMMAND].GetString();
    ParseCommand(input);
}

void ConsoleWindow::ParseCommand(String input)
{
    if (input.Empty()) {
        return;
    }
    StringVector commands = input.Split(' ', false);
    String command = commands[0];
    if (_registeredConsoleCommands.Contains(command)) {
        VariantMap data;
        data["Parameters"] = commands;

        // Call the actual event and pass all the parameters
        SendEvent(_registeredConsoleCommands[command].eventToCall, data);
    }
}

void ConsoleWindow::HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("");
    URHO3D_LOGINFO("###### All available (registered) commands ######");
    URHO3D_LOGINFO("#");
    for (auto it = _registeredConsoleCommands.Begin(); it != _registeredConsoleCommands.End(); ++it) {
        SingleConsoleCommand info = (*it).second_;
        URHO3D_LOGINFOF("# '%s' => '%s': %s", info.command.CString(), info.eventToCall.CString(), info.description.CString());
    }
    URHO3D_LOGINFO("#");
    URHO3D_LOGINFO("#########################");
    URHO3D_LOGINFO("");
}