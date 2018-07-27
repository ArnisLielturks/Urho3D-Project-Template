#include <Urho3D/Urho3DAll.h>
#include "ConsoleWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../WindowManager.h"

static struct nk_rect _rect;
static struct nk_scroll _scroll = {0, 0};

/// Construct.
ConsoleWindow::ConsoleWindow(Context* context) :
    BaseWindow(context),
    _contentLength(0),
    _consoleInputLength(0)
{
    auto graphics = GetSubsystem<Graphics>();
    int width = graphics->GetWidth();
    int height = graphics->GetHeight() - 100;
    _rect.x = graphics->GetWidth() / 2 - width / 2;
    _rect.w = width;
    _rect.y = 0;// graphics->GetHeight() / 2 - height / 2;
    _rect.h = height;

    _scroll.y = _rect.h;
    
    memset(_consoleInput, 0, sizeof(_consoleInput));

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
}

void ConsoleWindow::SubscribeToEvents()
{
    SubscribeToEvent(E_LOGMESSAGE, URHO3D_HANDLER(ConsoleWindow, HandleLogMessage));
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

void ConsoleWindow::AddContent(String message, int level)
{
    if (_lines.Size() > 100) {
        _lines.PopFront();
    }
    //message += "\n";

    _scroll.y = 2000;
    SingleLine line;
    line.content = message;
    line.logLevel = level;
    _lines.Push(line);

    //int offset = 0;
    //for (auto it = _lines.Begin(); it != _lines.End(); ++it) {
    //    memcpy(&_content[offset], (*it).CString(), (nk_size)(*it).Length());
    //    offset += (*it).Length();
    //}
    //_contentLength = offset;
}

void ConsoleWindow::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_F4) {
        WindowManager* windowManager = GetSubsystem<WindowManager>();
        if (!windowManager->IsWindowOpen("ConsoleWindow")) {
            VariantMap data = GetEventDataMap();
            data["Name"] = "ConsoleWindow";
            SendEvent(MyEvents::E_OPEN_WINDOW, data);
        }
        else {
            VariantMap data = GetEventDataMap();
            data["Name"] = "ConsoleWindow";
            SendEvent(MyEvents::E_CLOSE_WINDOW, data);
        }
    }

    if (!_active) {
        return;
    }

    if (key == KEY_RETURN) {
        if (!String(_consoleInput).Empty()) {
            AddContent(String(_consoleInput), LOG_INFO);
            _consoleInputLength = 0;

            VariantMap data = GetEventDataMap();
            data["Command"] = String(_consoleInput);
            SendEvent("ConsoleCommand", data);

            memset(_consoleInput, 0, sizeof(_consoleInput));
        }
    }
}

void ConsoleWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!_active) {
        return;
    }

    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    ctx->style.window.spacing = nk_vec2(0, 0);
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.group_padding = nk_vec2(1, 1);
    ctx->style.window.group_border = 0;
    
    if (nk_begin(nuklear->GetNkContext(), "Console", _rect, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        // nk_group_scrolled_begin(nuklear->GetNkContext(), &scroll, "ConsoleScroll", NK_WINDOW_SCROLL_AUTO_HIDE);
        nk_layout_row_dynamic(nuklear->GetNkContext(), _rect.h, 1);
        if (nk_group_begin(nuklear->GetNkContext(), "ConsoleGroup", NK_WINDOW_NO_SCROLLBAR)) {
            
            nk_layout_row_dynamic(nuklear->GetNkContext(), _rect.h - 40, 1);
            if (nk_group_scrolled_begin(nuklear->GetNkContext(), &_scroll, "ConsoleGroup2", NK_WINDOW_BORDER)) {
                for (auto it = _lines.Begin(); it != _lines.End(); ++it) {
                    nk_layout_row_dynamic(nuklear->GetNkContext(), 18, 1);
                    struct nk_color color = { 50, 200, 50, 255 };
                    if ((*it).logLevel == LOG_WARNING) {
                        color = { 200, 200, 50, 255 };
                    }
                    else if ((*it).logLevel == LOG_DEBUG) {
                        color = { 200, 150, 50, 255 };
                    }
                    else if ((*it).logLevel == LOG_ERROR) {
                        color = { 200, 50, 50, 255 };
                    }
                    ///nk_edit_string(nuklear->GetNkContext(), NK_EDIT_READ_ONLY | NK_EDIT_MULTILINE, _content, &_contentLength, CONSOLE_CONTENT_LENGTH, nk_filter_default);
                    nk_label_colored_wrap(nuklear->GetNkContext(), (*it).content.CString(), color);
                }
            }
            nk_group_scrolled_end(nuklear->GetNkContext());

            // nk_layout_row_dynamic(nuklear->GetNkContext(), 20, 2);
            nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
            {
                nk_layout_row_push(nuklear->GetNkContext(), 0.9);
                nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, _consoleInput, &_consoleInputLength, 256, nk_filter_default);
                nk_layout_row_push(nuklear->GetNkContext(), 0.1);
                if (nk_button_label(nuklear->GetNkContext(), "Submit")) {
                    AddContent(String(_consoleInput), LOG_INFO);
                    _consoleInputLength = 0;

                    VariantMap data = GetEventDataMap();
                    data["Command"] = String(_consoleInput);
                    SendEvent("ConsoleCommand", data);

                    memset(_consoleInput, 0, sizeof(_consoleInput));
                }
            }
            nk_layout_row_end(nuklear->GetNkContext());
        }
        nk_group_end(nuklear->GetNkContext());

        // nk_group_scrolled_end(nuklear->GetNkContext());

    }
    nk_end(ctx);

    if (_timer.GetMSec(false) > 1000) {
        _timer.Reset();
        URHO3D_LOGINFOF("FPS: %d => %d", (int)(1.0f / timeStep), GetSubsystem<Engine>()->GetMaxFps());
    }
}

void ConsoleWindow::HandleLogMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace LogMessage;
    String message = eventData[P_MESSAGE].GetString();
    int level = eventData[P_LEVEL].GetInt();
    AddContent(message, level);
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