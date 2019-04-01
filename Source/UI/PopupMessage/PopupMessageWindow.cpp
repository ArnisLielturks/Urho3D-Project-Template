#include <Urho3D/Urho3DAll.h>
#include "PopupMessageWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Global.h"

/// Construct.
PopupMessageWindow::PopupMessageWindow(Context* context) :
    BaseWindow(context)
{
}

PopupMessageWindow::~PopupMessageWindow()
{
    _okButton->Remove();
    _baseWindow->Remove();
}

void PopupMessageWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void PopupMessageWindow::Create()
{
    auto* localization = GetSubsystem<Localization>();

    _baseWindow = CreateOverlay()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(300, 200);
    _baseWindow->BringToFront();

    _okButton = CreateButton(localization->Get("OK"), 80, IntVector2(20, 0));
    _okButton->SetAlignment(HA_CENTER, VA_BOTTOM);
    _okButton->SetPosition(0, -20);

    if (_data.Contains("Type")) {
        if (_data["Type"].GetString() == "info") {
            _baseWindow->SetColor(Color::GREEN);
        } else if (_data["Type"].GetString() == "warning") {
            _baseWindow->SetColor(Color::YELLOW);
        } else if (_data["Type"].GetString() == "error") {
            _baseWindow->SetColor(Color::RED);
        } else {
            _baseWindow->SetColor(Color::GREEN);
        }
    }

    auto title = CreateLabel(_data["Title"].GetString());
    title->SetAlignment(HA_CENTER, VA_TOP);
    title->SetPosition(0, 10);
    title->SetFontSize(16);

    auto message = CreateLabel(_data["Message"].GetString());
    message->SetAlignment(HA_CENTER, VA_CENTER);
    //message->SetPosition(0, 10);
    message->SetFontSize(12);


    SubscribeToEvent(_okButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PopupMessageWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);

//        String type;
//        if (_data["Type"].GetString() == "info") {
//            type = "warning";
//        } else if (_data["Type"].GetString() == "warning") {
//            type = "error";
//        } else if (_data["Type"].GetString() == "error") {
//            type = "info";
//        }
//        data["Title"] = type;
//        data["Message"] = "Testing out " + type;
//        data["Name"] = "PopupMessageWindow";
//        data["Type"] = type;
//        data["ClosePrevious"] = true;
//        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });
}

void PopupMessageWindow::SubscribeToEvents()
{
}


Button* PopupMessageWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = _baseWindow->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(30);
    button->SetPosition(position);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

Text* PopupMessageWindow::CreateLabel(const String& text)
{
    String message = "";

    // Split longer messages into multiple lines
    String line;
    auto words = text.Split(' ', false);
    for (auto it = words.Begin(); it != words.End(); ++it) {
        if (line.Length() + (*it).Length() > 30) {
            message += line + "\n";
            line = "";
        }
        line += (*it) + " ";
    }
    if (!line.Empty()) {
        message += line;
    }

    auto *cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create log element to view latest logs from the system
    auto *label = _baseWindow->CreateChild<Text>();
    label->SetFont(font, 12);
    label->SetText(message);

    return label;
}