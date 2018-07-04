#include <Urho3D/Urho3DAll.h>
#include "ScoreboardWindow.h"
#include "../../MyEvents.h"

/// Construct.
ScoreboardWindow::ScoreboardWindow(Context* context) :
    BaseWindow(context, IntVector2(300, 300))
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
    UI* ui = GetSubsystem<UI>();

    {
        Text* text = _base->CreateChild<Text>();
        text->SetText("Scoreboard");
        text->SetStyleAuto();
        text->SetColor(Color(0.2f, 0.8f, 0.2f));
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetPosition(IntVector2(0, 10));
        text->SetFontSize(20);
    }

    {
        Text* text = _base->CreateChild<Text>();
        text->SetText("Name");
        text->SetStyleAuto();
        text->SetColor(Color(0.8f, 0.8f, 0.2f));
        text->SetAlignment(HA_LEFT, VA_TOP);
        text->SetPosition(IntVector2(20, 40));
        text->SetFontSize(14);
    }

    {
        Text* text = _base->CreateChild<Text>();
        text->SetText("Ping");
        text->SetStyleAuto();
        text->SetColor(Color(0.8f, 0.8f, 0.2f));
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetPosition(IntVector2(0, 40));
        text->SetFontSize(14);
    }

    {
        Text* text = _base->CreateChild<Text>();
        text->SetText("Score");
        text->SetStyleAuto();
        text->SetColor(Color(0.8f, 0.8f, 0.2f));
        text->SetAlignment(HA_RIGHT, VA_TOP);
        text->SetPosition(IntVector2(-20, 40));
        text->SetFontSize(14);
    }

    for (int i = 0; i < 10; i++) {
        {
            Text* text = _base->CreateChild<Text>();
            text->SetText("Player"  + String(i));
            text->SetStyleAuto();
            text->SetColor(Color(0.8f, 0.2f, 0.2f));
            text->SetAlignment(HA_LEFT, VA_TOP);
            text->SetPosition(IntVector2(20, 60 + i * 20));
            text->SetFontSize(14);
        }
        {
            Text* text = _base->CreateChild<Text>();
            text->SetText(String(i * 12));
            text->SetStyleAuto();
            text->SetColor(Color(0.8f, 0.2f, 0.2f));
            text->SetAlignment(HA_CENTER, VA_TOP);
            text->SetPosition(IntVector2(0, 60 + i * 20));
            text->SetFontSize(14);
        }
        {
            Text* text = _base->CreateChild<Text>();
            text->SetText(String(i * 35));
            text->SetStyleAuto();
            text->SetColor(Color(0.8f, 0.2f, 0.2f));
            text->SetAlignment(HA_RIGHT, VA_TOP);
            text->SetPosition(IntVector2(-20, 60 + i * 20));
            text->SetFontSize(14);
        }
    }
}

void ScoreboardWindow::SubscribeToEvents()
{
    SubscribeToEvent(_closeButton, E_RELEASED, URHO3D_HANDLER(ScoreboardWindow, HandleClose));
}

void ScoreboardWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Name"] = "ScoreboardWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}