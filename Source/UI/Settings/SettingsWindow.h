#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

enum SettingTabs {
    VIDEO,
    AUDIO,
    CONTROLS,
    CONTROLLERS,
    GAME,
    MISC
};

/**
 * Video settings are applied only after "Apply" button
 * is pressed, so this structure is meant for storign temporary
 * video settings values
 */
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

    /**
     * Subscribe for events which could affect settings window
     */
    void SubscribeToEvents();

    /**
     * Apply & Store new video settings
     */
    void SaveVideoSettings();

    /**
     * Handle control mapping events
     */
    void HandleControlsUpdated(StringHash eventType, VariantMap& eventData);

    /**
     * Switch between settings window tabs
     * @param tab
     */
    void ChangeTab(SettingTabs tab);

    /**
     * Create controls view with all the options
     */
    void CreateControlsTab();

    /**
     * Create mouse/keyboard/joystick controls view
     */
    void CreateControllersTab();

    /**
     * Create audio settings view
     */
    void CreateAudioTab();

    /**
     * Create video settings view
     */
    void CreateVideoTab();

    /**
     * Create game settings tab
     */
    void CreateGameTab();

    /**
     * Create miscellaneous settings view
     */
    void CreateMiscTab();

    /**
     * Set up graphics settings temporary data and available options
     */
    void InitGraphicsSettings();

    /**
     * Tab buttons
     */
    HashMap<int, SharedPtr<Button>> _tabs;

    /**
     * Window title bar
     */
    SharedPtr<UIElement> _titleBar;

    /**
     * Active tab index
     */
    SettingTabs _activeTab;

    /**
     * Tab view element count to make sure that the settings don't overlap
     */
    int _tabElementCount;

    /**
     * Currently created active line on which the buttons, checkboxes, dropdowns will be added
     */
    SharedPtr<UIElement> _activeLine;

    /**
     * Current graphics settings
     */
    GraphicsSettings _graphicsSettings;

    /**
     * New graphics settings
     */
    GraphicsSettings _graphicsSettingsNew;

    /**
     * Available resolutions
     */
    Vector<String> _availableResolutionNames;

    /**
     * Settings window main view
     */
    SharedPtr<Window> _baseWindow;

    /**
     * Tab view in which all the settings will be placed
     */
    SharedPtr<UIElement> _tabView;

    /**
     * Create TAB button at the top on window
     */
    Button* CreateTabButton(const String& text);

    /**
     * Add button to the currently active line
     */
    Button* CreateButton(const String& text);

    /**
     * Add checkbox to the currently active line
     */
    CheckBox* CreateCheckbox(const String& label);

    /**
     * Add label to the currently active line
     */
    Text* CreateLabel(const String& text);

    /**
     * Add slider to the currently active line
     */
    Slider* CreateSlider(const String& text);

    /**
     * Add dropdown to the currently active line
     */
    DropDownList* CreateMenu(const String& label, Vector<String>& items);

    /**
     * Create new line in the tab view where all the settings elemets will be placed
     */
    UIElement* CreateSingleLine();
};