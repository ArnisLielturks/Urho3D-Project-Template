#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

enum SettingTabs {
    VIDEO,
    AUDIO,
    CONTROLS,
    CONTROLLERS
};

struct GraphicsSettings {
    int width;
    int height;
    int fullscreen;
    int frameLimiter;
    int monitor;
    int vsync;
    int tripleBuffer;
    int shadows;
    int shadowQuality;
    int textureQuality;
    int textureAnistropy;
    int textureFilterMode;
    int multisample;
    int activeResolution;
};

class SettingsWindow : public BaseWindow
{
URHO3D_OBJECT(SettingsWindow, BaseWindow);

public:
    /// Construct.
    SettingsWindow(Context* context);

    virtual ~SettingsWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void SaveVideoSettings();

    void HandleControlsUpdated(StringHash eventType, VariantMap& eventData);

    void ChangeTab(SettingTabs tab);

    void CreateControlsTab();
    void CreateControllersTab();
    void CreateAudioTab();
    void CreateVideoTab();

    void InitGraphicsSettings();
    HashMap<int, SharedPtr<Button>> _tabs;

    SharedPtr<UIElement> _titleBar;
    SettingTabs _activeTab;
    int _tabElementCount;
    SharedPtr<UIElement> _activeLine;

    GraphicsSettings _graphicsSettings;
    GraphicsSettings _graphicsSettingsNew;

    Vector<String> _availableResolutionNames;

    SharedPtr<Window> _baseWindow;
    SharedPtr<UIElement> _tabView;

    Button* CreateTabButton(const String& text);

    Button* CreateButton(const String& text);
    CheckBox* CreateCheckbox(const String& label);
    Text* CreateLabel(const String& text);
    Slider* CreateSlider(const String& text);
    DropDownList* CreateMenu(const String& label, Vector<String>& items);

    UIElement* CreateSingleLine();
};