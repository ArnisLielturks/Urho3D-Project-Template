#include <Urho3D/Core/Object.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineEvents.h>
#include <Urho3D/IO/Log.h>
#include "ConsoleHandler.h"
#include "../Config/ConfigManager.h"
#include "ConsoleHandlerEvents.h"
#include "../LevelManagerEvents.h"

using namespace ConsoleHandlerEvents;
using namespace LevelManagerEvents;

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
    // Get default style
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile     = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Create console
    console_ = GetSubsystem<Engine>()->CreateConsole();
    console_->SetDefaultStyle(xmlFile);
    console_->GetBackground()->SetOpacity(0.8f);
    console_->SetNumHistoryRows(50);
    console_->SetNumBufferedRows(100);
    console_->GetBackground()->SetPriority(9999);

    // Hack to hide interpretator DropDownList
    PODVector<UIElement*> elements;
    console_->GetBackground()->GetChildren(elements, true);
    for (auto it = elements.Begin(); it != elements.End(); ++it) {
        if ((*it)->GetType() == "DropDownList") {
            (*it)->SetVisible(false);
        }
    }

    for (auto it = registeredConsoleCommands_.Begin(); it != registeredConsoleCommands_.End(); ++it) {
        console_->AddAutoComplete((*it).first_);
    }

    console_->SetVisible(false);
}

void ConsoleHandler::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ConsoleHandler, HandleKeyDown));
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommand));
    SubscribeToEvent(E_CONSOLE_COMMAND_ADD, URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandAdd));

    VariantMap data;
    data["ConsoleCommandName"]        = "help";
    data["ConsoleCommandEvent"]       = "ConsoleHelp";
    data["ConsoleCommandDescription"] = "Displays all available commands";

    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("ConsoleHelp", URHO3D_HANDLER(ConsoleHandler, HandleConsoleCommandHelp));

    // How to use lambda (anonymous) functions
    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "map", ConsoleCommandAdd::P_EVENT, "#map", ConsoleCommandAdd::P_DESCRIPTION, "Change the map");
    SubscribeToEvent("#map", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of params!");
            return;
        }
        VariantMap& data = GetEventDataMap();
        data["Name"] = "Loading";
        data["Map"]  = "Scenes/" + params[1];
        SendEvent(E_SET_LEVEL, data);
    });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "opacity", ConsoleCommandAdd::P_EVENT, "#opacity", ConsoleCommandAdd::P_DESCRIPTION, "Change the console opacity");
    SubscribeToEvent("#opacity", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Invalid number of params!");
            return;
        }
        float value = ToFloat(params[1]);
        console_->GetBackground()->SetOpacity(value);
    });

}

void ConsoleHandler::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "DeveloperConsole", true)) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_F1) {
            console_->Toggle();
            auto input = GetSubsystem<Input>();
            if (console_->IsVisible()) {
                input->SetMouseVisible(true);
            } else {
                input->ResetMouseVisible();
            }
        }
    } else {
        // If console is still visible when it was disabled via options, hide it
        if (console_->IsVisible()) {
            console_->Toggle();
        }
    }
}

void ConsoleHandler::HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommandAdd;
    String command = eventData[P_NAME].GetString();
    String eventToCall = eventData[P_EVENT].GetString();
    String description = eventData[P_DESCRIPTION].GetString();
    bool overwrite = eventData[P_OVERWRITE].GetBool();
    if (registeredConsoleCommands_.Contains(command) && !overwrite) {
        URHO3D_LOGWARNINGF("Console command '%s' already registered! Skipping console command registration!", command.CString());
        return;
    }

    // Add to autocomplete
    if (console_) {
        console_->AddAutoComplete(command);
    }

    // Register new console command
    SingleConsoleCommand singleConsoleCommand;
    singleConsoleCommand.command = command;
    singleConsoleCommand.description = description;
    singleConsoleCommand.eventToCall = eventToCall;
    registeredConsoleCommands_[command] = singleConsoleCommand;
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
    StringVector params = input.Split(' ', false);
    String command = params[0];
    if (registeredConsoleCommands_.Contains(command)) {
        VariantMap data;
        data["Parameters"] = params;

        // Call the actual event and pass all the parameters
        SendEvent(registeredConsoleCommands_[command].eventToCall, data);
    } else {
        if (GetGlobalVar(command) != Variant::EMPTY) {
            HandleConsoleGlobalVariableChange(params);
        } else {
            URHO3D_LOGERRORF("Command '%s' not registered", command.CString());
        }
    }
}

void ConsoleHandler::HandleConsoleCommandHelp(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("");
    URHO3D_LOGINFO("------- All available (registered) commands -------");
    URHO3D_LOGINFO("-");
    for (auto it = registeredConsoleCommands_.Begin(); it != registeredConsoleCommands_.End(); ++it) {
        SingleConsoleCommand info = (*it).second_;
        URHO3D_LOGINFOF("- '%s' => '%s': %s", info.command.CString(), info.eventToCall.CString(), info.description.CString());
    }
    URHO3D_LOGINFO("-");
    URHO3D_LOGINFO("---------------------------------------------------");
    URHO3D_LOGINFO("");
}

void ConsoleHandler::HandleConsoleGlobalVariableChange(StringVector params)
{
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
        URHO3D_LOGINFO(params[0] + " = " + stringValue);
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
        URHO3D_LOGINFO("Global variable changed'" + params[0] + "' from '" + oldValue + "' to '" + newValue + "'");

        // Let the system know that variable was changed
        using namespace ConsoleGlobalVariableChanged;
        VariantMap& data = GetEventDataMap();
        data[P_NAME]     = params[0];
        data[P_VALUE]    = newValue;

        SendEvent("global_variable_changed_" + params[0], data);
        SendEvent(E_CONSOLE_GLOBAL_VARIABLE_CHANGED, data);
    }
}
