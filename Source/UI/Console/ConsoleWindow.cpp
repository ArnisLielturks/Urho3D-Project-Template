#include <Urho3D/Urho3DAll.h>
#include "ConsoleWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"
#include "../../Audio/AudioManagerDefs.h"

static struct nk_rect _rect;
static struct nk_scroll _scroll = {0, 200};

/// Construct.
ConsoleWindow::ConsoleWindow(Context* context) :
    BaseWindow(context),
    _contentLength(0),
    _consoleInputLength(0)
{
    int width = 700;
    int height = 500;
    auto graphics = GetSubsystem<Graphics>();
    _rect.x = graphics->GetWidth() / 2 - width / 2;
    _rect.w = width;
    _rect.y = graphics->GetHeight() / 2 - height / 2;
    _rect.h = height;
    
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
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ConsoleWindow, HandleUpdate));
    SubscribeToEvent(E_LOGMESSAGE, URHO3D_HANDLER(ConsoleWindow, HandleLogMessage));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ConsoleWindow, HandleKeyDown));
}

void ConsoleWindow::AddContent(String message)
{
    if (_lines.Size() > 100) {
        _lines.PopFront();
    }
    message += "\n";

    _scroll.y = 2000;
    _lines.Push(message);

    int offset = 0;
    for (auto it = _lines.Begin(); it != _lines.End(); ++it) {
        memcpy(&_content[offset], (*it).CString(), (nk_size)(*it).Length());
        offset += (*it).Length();
    }
    _contentLength = offset;
}

void ConsoleWindow::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    if (!_active) {
        return;
    }

    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_RETURN) {
        if (!String(_consoleInput).Empty()) {
            AddContent(String(_consoleInput));
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

    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    // ctx->style.window.spacing = nk_vec2(4,4);
    // ctx->style.window.padding = nk_vec2(4,4);
    
    if (nk_begin(nuklear->GetNkContext(), "Console", _rect, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        // nk_group_scrolled_begin(nuklear->GetNkContext(), &scroll, "ConsoleScroll", NK_WINDOW_SCROLL_AUTO_HIDE);
        nk_layout_row_dynamic(nuklear->GetNkContext(), 500, 1);
        if (nk_group_begin(nuklear->GetNkContext(), "ConsoleGroup", NK_WINDOW_NO_SCROLLBAR)) {
            
            nk_layout_row_dynamic(nuklear->GetNkContext(), 450, 1);
            if (nk_group_scrolled_begin(nuklear->GetNkContext(), &_scroll, "ConsoleGroup2", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(nuklear->GetNkContext(), 1540, 1);
                nk_edit_string(nuklear->GetNkContext(), NK_EDIT_READ_ONLY | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD | NK_EDIT_MULTILINE, _content, &_contentLength, CONSOLE_CONTENT_LENGTH, nk_filter_default);
            }
            nk_group_scrolled_end(nuklear->GetNkContext());

            // nk_layout_row_dynamic(nuklear->GetNkContext(), 20, 2);
            nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 20, 2);
            {
                nk_layout_row_push(nuklear->GetNkContext(), 0.9);
                nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, _consoleInput, &_consoleInputLength, 256, nk_filter_default);
                nk_layout_row_push(nuklear->GetNkContext(), 0.1);
                if (nk_button_label(nuklear->GetNkContext(), "Submit")) {
                    AddContent(String(_consoleInput));
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
}

void ConsoleWindow::HandleLogMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace LogMessage;
    String message = eventData[P_MESSAGE].GetString();
    int level = eventData[P_LEVEL].GetInt();
    AddContent(message);
}