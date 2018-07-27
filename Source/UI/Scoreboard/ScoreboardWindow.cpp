#include <Urho3D/Urho3DAll.h>
#include "ScoreboardWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"

/// Construct.
ScoreboardWindow::ScoreboardWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

ScoreboardWindow::~ScoreboardWindow()
{
}

void ScoreboardWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ScoreboardWindow::Create()
{
}

void ScoreboardWindow::SubscribeToEvents()
{
}


void ScoreboardWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);
    static struct nk_color color = {20, 200, 50, 255};

    if (nk_begin(nuklear->GetNkContext(), "Pause", nk_rect(graphics->GetWidth() / 2 - 200, graphics->GetHeight() / 2 - 200, 400, 400), NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(nuklear->GetNkContext(), "Scoreboard", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 3);
        nk_label_colored(nuklear->GetNkContext(), "Player", NK_TEXT_LEFT, color);
        nk_label_colored(nuklear->GetNkContext(), "Score", NK_TEXT_CENTERED, color);
        nk_label_colored(nuklear->GetNkContext(), "Ping", NK_TEXT_RIGHT, color);
        
        for (int i = 1; i < 10; i++) {
            nk_layout_row_dynamic(ctx, 30, 3);
            String name = "Player " + String(i);
            nk_label(nuklear->GetNkContext(), name.CString(), NK_TEXT_LEFT);

            String points = String(i * 12);
            nk_label(nuklear->GetNkContext(), points.CString(), NK_TEXT_CENTERED);

            String ping = String(i * 3);
            nk_label(nuklear->GetNkContext(), ping.CString(), NK_TEXT_RIGHT);
        }

    }
    nk_end(ctx);
}