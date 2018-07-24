#include <Urho3D/Urho3DAll.h>
#include "MainMenu.h"
#include "../MyEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../UI/NuklearUI.h"

using namespace Levels;

    /// Construct.
MainMenu::MainMenu(Context* context) :
    BaseLevel(context)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::Init()
{
    if (data_.Contains("Message")) {
        //SharedPtr<Urho3D::MessageBox> messageBox(new Urho3D::MessageBox(context_, data_["Message"].GetString(), "Oh crap!"));
        VariantMap data = GetEventDataMap();
        data["Title"] = "Error!";
        data["Message"] = data_["Message"].GetString();
        SendEvent("ShowAlertMessage", data);
    }
    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
    VariantMap data = GetEventDataMap();
    data["Message"] = "Entered menu!";
    SendEvent("NewAchievement", data);

    data["Title"] = "Hey!";
    data["Message"] = "Seems like everything is ok!";
    SendEvent("ShowAlertMessage", data);
}

void MainMenu::CreateScene()
{
    
}

void MainMenu::CreateUI()
{
    UI* ui = GetSubsystem<UI>();

    {
        _startButton = ui->GetRoot()->CreateChild<Button>();
        _startButton->SetSize(IntVector2(100, 30));
        _startButton->SetStyleAuto();

        _startButton->SetPosition(IntVector2(-20, -140));
        _startButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
        _startButton->SetFocusMode(FM_RESETFOCUS);

        Text* text = _startButton->CreateChild<Text>();
        text->SetText("Start");
        text->SetStyleAuto();
        text->SetAlignment(HA_CENTER, VA_CENTER);
    }
    {
        _settingsButton = ui->GetRoot()->CreateChild<Button>();
        _settingsButton->SetSize(IntVector2(100, 30));
        _settingsButton->SetStyleAuto();
        _settingsButton->SetPosition(IntVector2(-20, -100));
        _settingsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
        _settingsButton->SetFocusMode(FM_RESETFOCUS);

        Text* text = _settingsButton->CreateChild<Text>();
        text->SetText("Settings");
        text->SetStyleAuto();
        text->SetAlignment(HA_CENTER, VA_CENTER);
    }
	{
		_creditsButton = ui->GetRoot()->CreateChild<Button>();
		_creditsButton->SetSize(IntVector2(100, 30));
		_creditsButton->SetStyleAuto();
		_creditsButton->SetPosition(IntVector2(-20, -60));
		_creditsButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
		_creditsButton->SetFocusMode(FM_RESETFOCUS);

		Text* text = _creditsButton->CreateChild<Text>();
		text->SetText("Credits");
		text->SetStyleAuto();
		text->SetAlignment(HA_CENTER, VA_CENTER);
	}
    {
        _exitButton = ui->GetRoot()->CreateChild<Button>();
        _exitButton->SetSize(IntVector2(100, 30));
        _exitButton->SetStyleAuto();
        _exitButton->SetPosition(IntVector2(-20, -20));
        _exitButton->SetAlignment(HA_RIGHT, VA_BOTTOM);
        _exitButton->SetFocusMode(FM_RESETFOCUS);

        Text* text = _exitButton->CreateChild<Text>();
        text->SetText("Exit");
        text->SetStyleAuto();
        text->SetAlignment(HA_CENTER, VA_CENTER);
    }

    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void MainMenu::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MainMenu, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MainMenu, HandleKeyDown));
    SubscribeToEvent(_startButton, E_RELEASED, URHO3D_HANDLER(MainMenu, HandleStartGame));
    SubscribeToEvent(_settingsButton, E_RELEASED, URHO3D_HANDLER(MainMenu, HandleShowSettings));
	SubscribeToEvent(_creditsButton, E_RELEASED, URHO3D_HANDLER(MainMenu, HandleShowCredits));
    SubscribeToEvent(_exitButton, E_RELEASED, URHO3D_HANDLER(MainMenu, HandleExit));
}

void MainMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    auto nuklear = GetSubsystem<NuklearUI>();
    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;
    static int i =  20;
    static size_t progress = 1;

    if (nk_begin(nuklear->GetNkContext(), "Show", nk_rect(50, 50, 400, 400),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        /* fixed widget pixel width */
        nk_layout_row_static(nuklear->GetNkContext(), 30, 80, 1);
        if (nk_button_label(nuklear->GetNkContext(), "button")) {
            /* event handling */
        }

        /* fixed widget window ratio width */
        nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 2);
        if (nk_option_label(nuklear->GetNkContext(), "easy", op == EASY)) op = EASY;
        if (nk_option_label(nuklear->GetNkContext(), "hard", op == HARD)) op = HARD;

        /* custom widget pixel width */
        nk_layout_row_begin(nuklear->GetNkContext(), NK_STATIC, 30, 2);
        {
            nk_layout_row_push(nuklear->GetNkContext(), 50);
            nk_label(nuklear->GetNkContext(), "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(nuklear->GetNkContext(), 110);
            nk_slider_float(nuklear->GetNkContext(), 0, &value, 1.0f, 0.05f);
        }
        nk_layout_row_end(nuklear->GetNkContext());

        nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
        {
            nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
            nk_progress(nuklear->GetNkContext(), &progress, 1000, NK_MODIFIABLE);
        }
        progress += 1;
    }
    nk_end(nuklear->GetNkContext());
}

void MainMenu::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();
}

void MainMenu::HandleStartGame(StringHash eventType, VariantMap& eventData)
{
    {
        using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
    VariantMap& levelEventData = GetEventDataMap();
    levelEventData["Name"] = "Loading";
    SendEvent(MyEvents::E_SET_LEVEL, levelEventData);
}

void MainMenu::HandleShowSettings(StringHash eventType, VariantMap& eventData)
{
    {
        using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
    VariantMap data = GetEventDataMap();
    data["Name"] = "SettingsWindow";
    SendEvent(MyEvents::E_OPEN_WINDOW, data);
}

void MainMenu::HandleExit(StringHash eventType, VariantMap& eventData)
{
    {
        using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
    VariantMap data = GetEventDataMap();;
    data["Name"] = "ExitGame";
    SendEvent(MyEvents::E_SET_LEVEL, data);
}

void MainMenu::HandleShowCredits(StringHash eventType, VariantMap& eventData)
{
    {
        using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
	VariantMap data = GetEventDataMap();;
	data["Name"] = "Credits";
	SendEvent(MyEvents::E_SET_LEVEL, data);
}