#include <Urho3D/Urho3DAll.h>
#include "NewGameSettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
NewGameSettingsWindow::NewGameSettingsWindow(Context* context) :
    BaseWindow(context),
    _active(true)
{
    Init();
    auto graphics = GetSubsystem<Graphics>();
    int width = 200;
    int height = 200;
    _rect.x = graphics->GetWidth() / 2 - width / 2;
    _rect.w = width;
    _rect.y = graphics->GetHeight() / 2 - height / 2;
    _rect.h = height;
}

NewGameSettingsWindow::~NewGameSettingsWindow()
{
}

void NewGameSettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void NewGameSettingsWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void NewGameSettingsWindow::SubscribeToEvents()
{
}

void NewGameSettingsWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    if (nk_begin(nuklear->GetNkContext(), "NewGame", _rect, NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(nuklear->GetNkContext(), "New game", NK_TEXT_CENTERED);

        nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
        {
            ctx->style.button.border_color = { 200, 50, 50, 255 };
            nk_layout_row_push(ctx, 0.51f);
            if (nk_button_label(nuklear->GetNkContext(), "Yes")) {
                if (_active) {
                    VariantMap data = GetEventDataMap();
                    /*data["Name"] = "ExitGame";
                    SendEvent(MyEvents::E_SET_LEVEL, data);*/
                    data["Name"] = "Loading";
                    SendEvent(MyEvents::E_SET_LEVEL, data);
                    _active = false;
                }
            }
            nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

            nk_style_default(ctx);
            nk_layout_row_push(ctx, 0.5f);
            if (nk_button_label(nuklear->GetNkContext(), "No")) {
                if (_active) {
                    //SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);
                    using namespace MyEvents::CloseWindow;
                    VariantMap& data = GetEventDataMap();
                    data[P_NAME] = "NewGameSettingsWindow";
                    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
                    _active = false;
                }
            }
            nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}