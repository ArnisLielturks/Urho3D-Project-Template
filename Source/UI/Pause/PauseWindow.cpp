#include <Urho3D/Urho3DAll.h>
#include "PauseWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
PauseWindow::PauseWindow(Context* context) :
    BaseWindow(context),
    _active(true)
{
    Init();
}

PauseWindow::~PauseWindow()
{
}

void PauseWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void PauseWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void PauseWindow::SubscribeToEvents()
{
}

void PauseWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    if (nk_begin(nuklear->GetNkContext(), "Pause", nk_rect(graphics->GetWidth() / 2 - 100, graphics->GetHeight() / 2 - 60, 200, 160), NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(nuklear->GetNkContext(), "Pause", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Resume")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap data = GetEventDataMap();
                data["Name"] = "PauseWindow";
                SendEvent(MyEvents::E_CLOSE_WINDOW, data);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Return to menu")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap data = GetEventDataMap();

                data["Name"] = "MainMenu";
                SendEvent(MyEvents::E_SET_LEVEL, data);

                data["Name"] = "PauseWindow";
                SendEvent(MyEvents::E_CLOSE_WINDOW, data);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Exit game")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap data = GetEventDataMap();

                data["Name"] = "ExitGame";
                SendEvent(MyEvents::E_SET_LEVEL, data);

                data["Name"] = "PauseWindow";
                SendEvent(MyEvents::E_CLOSE_WINDOW, data);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
    }
    nk_end(ctx);
}