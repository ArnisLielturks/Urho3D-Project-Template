#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/JSONFile.h>
#include "NewGameSettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Global.h"

static const int BUTTON_HEIGHT = 40;
static const int MARGIN = 10;
static const int IMAGE_SIZE = 200;

NewGameSettingsWindow::NewGameSettingsWindow(Context* context) :
    BaseWindow(context)
{
}

NewGameSettingsWindow::~NewGameSettingsWindow()
{
    _baseWindow->Remove();
}

void NewGameSettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void NewGameSettingsWindow::Create()
{
    auto* localization = GetSubsystem<Localization>();

    _baseWindow = CreateOverlay()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetLayout(LayoutMode::LM_VERTICAL, MARGIN, IntRect(MARGIN, MARGIN, MARGIN, MARGIN));
    _baseWindow->BringToFront();
    _baseWindow->GetParent()->SetPriority(_baseWindow->GetParent()->GetPriority() + 1000);

    // Create Window 'titlebar' container
    UIElement* titleBar =_baseWindow->CreateChild<UIElement>();
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
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    CreateLevelSelection();

    _startServer = CreateCheckbox("Start server");
    _connectServer = CreateCheckbox("Connect to server");

    titleBar->SetFixedSize(_levelSelection->GetWidth(), 24);
}

void NewGameSettingsWindow::SubscribeToEvents()
{
}

CheckBox* NewGameSettingsWindow::CreateCheckbox(const String& label)
{
    UIElement* options = _baseWindow->CreateChild<UIElement>();
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
    _levelSelection = _baseWindow->CreateChild<UIElement>();
    _levelSelection->SetPosition(0, 0);
    _levelSelection->SetFixedHeight(IMAGE_SIZE);
    _levelSelection->SetLayout(LayoutMode::LM_HORIZONTAL, MARGIN);

    auto cache = GetSubsystem<ResourceCache>();
    auto font = cache->GetResource<Font>(APPLICATION_FONT);

    auto maps = LoadMaps();

    for (auto it = maps.Begin(); it != maps.End(); ++it) {

        UIElement *mapView = _levelSelection->CreateChild<UIElement>();
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
            data["StartServer"] = _startServer->IsChecked();
            data["ConnectServer"] = _connectServer->IsChecked() ? "127.0.0.1" : String::EMPTY;
            SendEvent(MyEvents::E_SET_LEVEL, data);
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

Vector<MapInfo> NewGameSettingsWindow::LoadMaps()
{
    Vector<MapInfo> maps;
    auto configFile = GetSubsystem<ResourceCache>()->GetResource<JSONFile>("Config/Maps.json");

    JSONValue value = configFile->GetRoot();
    if (value.IsArray()) {
        URHO3D_LOGINFOF("Loading map list: %u", value.Size());
        for (int i = 0; i < value.Size(); i++) {
            JSONValue mapInfo = value[i];
            if (mapInfo.Contains("Map")
                && mapInfo["Map"].IsString()
                && mapInfo.Contains("Name")
                && mapInfo["Name"].IsString()
               && mapInfo.Contains("Image")
               && mapInfo["Image"].IsString()
                && mapInfo.Contains("Description")
                && mapInfo["Description"].IsString()) {
                MapInfo map;
                map.map         = mapInfo["Map"].GetString();
                map.name        = mapInfo["Name"].GetString();
                map.description = mapInfo["Description"].GetString();
                map.image       = mapInfo["Image"].GetString();

                if (mapInfo.Contains("Commands") && mapInfo["Commands"].IsArray()) {
                    for (int c = 0; c < mapInfo["Commands"].Size(); c++) {
                        JSONValue command = mapInfo["Commands"][c];
                        if (command.IsString()) {
                            map.commands.Push(mapInfo["Commands"][c].GetString());
                            URHO3D_LOGINFOF("Adding map command %s", map.commands[map.commands.Size() - 1].CString());
                        }
                    }
                }
                maps.Push(map);
            }
            else {
                URHO3D_LOGERRORF("Map record doesnt contain all the information! Index: %u", i);
            }
        }
    }
    else {
        URHO3D_LOGERROR("Data/Config/Maps.json must be an array");
    }

    return maps;
}
