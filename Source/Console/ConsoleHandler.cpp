#include <Urho3D/Urho3DAll.h>
#include "ConsoleHandler.h"
#include "../MyEvents.h"
#include "../Config/ConfigManager.h"

/// Construct.
ConsoleHandler::ConsoleHandler(Context* context) :
    Object(context)
{
    Init();
}

ConsoleHandler::~ConsoleHandler()
{
}

void ConsoleHandler::Init()
{
    SubscribeToEvents();
}

void ConsoleHandler::Create()
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
    _console->GetBackground()->SetPriority(9999);

    for (auto it = _registeredConsoleCommands.Begin(); it != _registeredConsoleCommands.End(); ++it) {
        _console->AddAutoComplete((*it).first_);
    }
}

void ConsoleHandler::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ConsoleHandler, HandleKeyDown));
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommand));
    SubscribeToEvent(MyEvents::E_CONSOLE_COMMAND_ADD, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandAdd));
    SubscribeToEvent(MyEvents::E_CONSOLE_GLOBAL_VARIABLE_CHANGE, URHO3D_HANDLER(ConsoleHandler, HandleConsoleGlobalVariableChange));

    VariantMap data;
    data["ConsoleCommandName"] = "help";
    data["ConsoleCommandEvent"] = "ConsoleHelp";
    data["ConsoleCommandDescription"] = "Displays all available commands";
    SendEvent("ConsoleCommandAdd", data);
    SubscribeToEvent("ConsoleHelp", URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandHelp));

}

void ConsoleHandler::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "DeveloperConsole", true)) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_F2) {
            _console->Toggle();
        }
    } else {
        // If console is still visible when it was disabled via options, hide it
        if (_console->IsVisible()) {
            _console->Toggle();
        }
    }
}

void ConsoleHandler::HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::ConsoleCommandAdd;
    String command = eventData[P_NAME].GetString();
    String eventToCall = eventData[P_EVENT].GetString();
    String description = eventData[P_DESCRIPTION].GetString();
    if (_registeredConsoleCommands.Contains(command)) {
        URHO3D_LOGWARNINGF("Console command '%s' already registered! Overwriting it!", command.CString());
    }

    // Add to autocomplete
    if (_console) {
        _console->AddAutoComplete(command);
    }

    // Register new console command
    SingleConsoleCommand singleConsoleCommand;
    singleConsoleCommand.command = command;
    singleConsoleCommand.description = description;
    singleConsoleCommand.eventToCall = eventToCall;
    _registeredConsoleCommands[command] = singleConsoleCommand;
}

void ConsoleHandler::HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    String input = eventData[P_COMMAND].GetString();
    URHO3D_LOGINFO(input);
    ParseCommand(input);
}

void ConsoleHandler::ParseCommand(String input)
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

void ConsoleHandler::HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData)
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

void ConsoleHandler::HandleConsoleGlobalVariableChange(StringHash eventType, VariantMap& eventData)
{
    StringVector params = eventData["Parameters"].GetStringVector();

    const Variant value = GetSubsystem<Engine>()->GetGlobalVar(params[0]);

    // Only show variable
    if (params.Size() == 1 && !value.IsEmpty()) {
        String stringValue;
        if (value.GetType() == VAR_STRING) {
            stringValue = value.GetString();
        }
        if (value.GetType() == VAR_BOOL) {
            stringValue = value.GetBool() ? "1" : "0";
        }
        if (value.GetType() == VAR_INT) {
            stringValue = String(value.GetInt());
        }
        if (value.GetType() == VAR_FLOAT) {
            stringValue = String(value.GetFloat());
        }
        if (value.GetType() == VAR_DOUBLE) {
            stringValue = String(value.GetDouble());
        }
        URHO3D_LOGINFO("Global variable '" + params[0] + "' = " + stringValue);
        return;
    }

    // Read console input parameters and change global variable
    if (params.Size() == 2) {
        String oldValue;
        String newValue;
        if (value.GetType() == VAR_STRING) {
            oldValue = value.GetString();
            SetGlobalVar(params[0], params[1]);
            newValue = params[1];
        }
        if (value.GetType() == VAR_BOOL) {
            oldValue = value.GetBool() ? "1" : "0";
            if (params[1] == "1" || params[1] == "true") {
                SetGlobalVar(params[0], true);
                newValue = "1";
            }
            if (params[1] == "0" || params[1] == "false") {
                SetGlobalVar(params[0], false);
                newValue = "0";
            }
        }
        if (value.GetType() == VAR_INT) {
            oldValue = String(value.GetInt());
            int newIntVal = ToInt(params[1].CString());
            SetGlobalVar(params[0], newIntVal);
            newValue = String(newIntVal);
        }
        if (value.GetType() == VAR_FLOAT) {
            oldValue = String(value.GetFloat());
            float newFloatVal = ToFloat(params[1].CString());
            SetGlobalVar(params[0], newFloatVal);
            newValue = String(newFloatVal);
        }
        if (value.GetType() == VAR_DOUBLE) {
            oldValue = String(value.GetDouble());
            float newFloatVal = ToFloat(params[1].CString());
            SetGlobalVar(params[0],newFloatVal);
            newValue = String(newFloatVal);
        }
        URHO3D_LOGINFO("Changed global variable '" + params[0] + "' from '" + oldValue + "' to '" + newValue + "'");

        // Let others know that configuration was updated, to allow game tweaking accordingly
        using namespace MyEvents::ConsoleGlobalVariableChanged;
        VariantMap data = GetEventDataMap();
        data[P_NAME] = params[0];
        data[P_VALUE] = newValue;
        SendEvent(MyEvents::E_CONSOLE_GLOBAL_VARIABLE_CHANGED, data);
    }
}