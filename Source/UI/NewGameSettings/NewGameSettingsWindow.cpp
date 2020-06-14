#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/JSONFile.h>
#include "NewGameSettingsWindow.h"
#include "../../Global.h"
#include "../../LevelManagerEvents.h"
#include "../WindowEvents.h"
#include "../../SceneManager.h"

static const int BUTTON_HEIGHT = 40;
static const int MARGIN = 10;
static const int IMAGE_SIZE = 200;

using namespace LevelManagerEvents;
using namespace WindowEvents;

NewGameSettingsWindow::NewGameSettingsWindow(Context* context) :
    BaseWindow(context)
{
}

NewGameSettingsWindow::~NewGameSettingsWindow()
{
    baseWindow_->Remove();
}

void NewGameSettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void NewGameSettingsWindow::Create()
{
    auto* localization = GetSubsystem<Localization>();

    baseWindow_ = CreateOverlay()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetLayout(LayoutMode::LM_VERTICAL, MARGIN, IntRect(MARGIN, MARGIN, MARGIN, MARGIN));
    baseWindow_->BringToFront();
    baseWindow_->GetParent()->SetPriority(baseWindow_->GetParent()->GetPriority() + 1000);

    // Create Window 'titlebar' container
    UIElement* titleBar =baseWindow_->CreateChild<UIElement>();
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);
    titleBar->SetLayoutBorder(IntRect(0, 4, 0, 4));
    titleBar->SetFixedHeight(32);

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText(localization->Get("NEW_GAME"));
    windowTitle->SetFont(font, 14);

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");
    buttonClose->SetHorizontalAlignment(HA_RIGHT);

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Apply styles
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    SubscribeToEvent(buttonClose, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "NewGameSettingsWindow";
        SendEvent(E_CLOSE_WINDOW, data);
    });

    CreateLevelSelection();

#ifndef __EMSCRIPTEN__
    startServer_ = CreateCheckbox("Start server");
#endif
    connectServer_ = CreateCheckbox("Connect to server");
#ifdef __EMSCRIPTEN__
    connectServer_->SetChecked(true);
#endif

    titleBar->SetFixedSize(levelSelection_->GetWidth(), 24);
}

void NewGameSettingsWindow::SubscribeToEvents()
{
}

CheckBox* NewGameSettingsWindow::CreateCheckbox(const String& label)
{
    UIElement* options = baseWindow_->CreateChild<UIElement>();
    options->SetLayout(LayoutMode::LM_HORIZONTAL, MARGIN);
    Text* labelElement = options->CreateChild<Text>();
    labelElement->SetStyleAuto();
    labelElement->SetText(label);
    CheckBox* checkBox = options->CreateChild<CheckBox>();
    checkBox->SetStyleAuto();
    return checkBox;
}

Button* NewGameSettingsWindow::CreateButton(UIElement *parent, const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = parent->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(BUTTON_HEIGHT);
    button->SetPosition(position);

    if (!text.Empty()) {
        auto *buttonText = button->CreateChild<Text>();
        buttonText->SetFont(font, 16);
        buttonText->SetAlignment(HA_CENTER, VA_CENTER);
        buttonText->SetText(text);
    }

    return button;
}

void NewGameSettingsWindow::CreateLevelSelection()
{
    levelSelection_ = baseWindow_->CreateChild<UIElement>();
    levelSelection_->SetPosition(0, 0);
    levelSelection_->SetFixedHeight(IMAGE_SIZE);
    levelSelection_->SetLayout(LayoutMode::LM_HORIZONTAL, MARGIN);

    auto cache = GetSubsystem<ResourceCache>();
    auto font = cache->GetResource<Font>(APPLICATION_FONT);

    auto maps = GetSubsystem<SceneManager>()->GetAvailableMaps();

    for (auto it = maps.Begin(); it != maps.End(); ++it) {

        UIElement *mapView = levelSelection_->CreateChild<UIElement>();
        mapView->SetLayout(LayoutMode::LM_VERTICAL, 5);

        auto button = CreateButton(mapView, "", IMAGE_SIZE, IntVector2(0, 0));
        button->SetFixedHeight(IMAGE_SIZE);
        button->SetVar("Map", (*it).map);
        button->SetVar("Commands", (*it).commands);
        button->SetStyle("MapSelection");

        SubscribeToEvent(button, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
            using namespace Released;
            Button* button = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());

            VariantMap& data = GetEventDataMap();
            data["Name"] = "Loading";
            data["Map"] = button->GetVar("Map");
            data["Commands"] = button->GetVar("Commands");
#ifndef __EMSCRIPTEN__
            data["StartServer"] = startServer_->IsChecked();
#endif
            data["ConnectServer"] = connectServer_->IsChecked() && !startServer_->IsChecked() ? "127.0.0.1" : String::EMPTY;
            SendEvent(E_SET_LEVEL, data);
        });

        auto sprite = button->CreateChild<Sprite>();
        sprite->SetFixedHeight(IMAGE_SIZE - MARGIN);
        sprite->SetFixedWidth(IMAGE_SIZE - MARGIN);
        sprite->SetTexture(cache->GetResource<Texture2D>((*it).image));
        sprite->SetHotSpot(sprite->GetWidth() / 2, sprite->GetHeight() / 2);
        sprite->SetAlignment(HA_CENTER, VA_CENTER);

        auto name = mapView->CreateChild<Text>();
        name->SetFont(font, 14);
        name->SetAlignment(HA_CENTER, VA_TOP);
        name->SetFixedWidth(IMAGE_SIZE);
        name->SetWordwrap(true);
        name->SetText((*it).name);

        auto description = mapView->CreateChild<Text>();
        description->SetFont(font, 12);
        description->SetAlignment(HA_CENTER, VA_TOP);
        description->SetFixedWidth(IMAGE_SIZE);
        description->SetWordwrap(true);
        description->SetText((*it).description);
    }
}
