#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include "../BaseWindow.h"

struct MapInfo {
    String map;
    String name;
    String description;
};

class NewGameSettingsWindow : public BaseWindow
{
    URHO3D_OBJECT(NewGameSettingsWindow, BaseWindow);

public:
    NewGameSettingsWindow(Context* context);

    virtual ~NewGameSettingsWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    Button* CreateButton(UIElement *parent, const String& text, int width, IntVector2 position);

    Vector<MapInfo> LoadMaps();

    void CreateLevelSelection();

    SharedPtr<Window> _baseWindow;
    SharedPtr<UIElement> _levelSelection;
};