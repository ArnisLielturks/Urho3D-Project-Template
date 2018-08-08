#include <Urho3D/Urho3DAll.h>
#include "QuitConfirmationWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
QuitConfirmationWindow::QuitConfirmationWindow(Context* context) :
    BaseWindow(context),
    _active(true)
{
    Init();
}

QuitConfirmationWindow::~QuitConfirmationWindow()
{
}

void QuitConfirmationWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void QuitConfirmationWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void QuitConfirmationWindow::SubscribeToEvents()
{
}

void QuitConfirmationWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    if (nk_begin(nuklear->GetNkContext(), "Really quit", nk_rect(graphics->GetWidth() / 2 - 100, graphics->GetHeight() / 2 - 50, 200, 100), NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(nuklear->GetNkContext(), "Really quit?", NK_TEXT_CENTERED);

        nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
        {
            ctx->style.button.border_color = { 200, 50, 50, 255 };
            nk_layout_row_push(ctx, 0.51f);
            if (nk_button_label(nuklear->GetNkContext(), "Yes")) {
                if (_active) {
                    VariantMap data = GetEventDataMap();
                    data["Name"] = "ExitGame";
                    SendEvent(MyEvents::E_SET_LEVEL, data);
                    _active = false;
                }
            }
            nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

            nk_style_default(ctx);
            nk_layout_row_push(ctx, 0.5f);
            if (nk_button_label(nuklear->GetNkContext(), "No")) {
                if (_active) {
                    SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);
                    _active = false;
                }
            }
            nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}