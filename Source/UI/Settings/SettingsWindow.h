#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include "UIOption.h"
#include "../BaseWindow.h"

class SettingsWindow : public BaseWindow
{
    URHO3D_OBJECT(SettingsWindow, BaseWindow);

public:
    /// Construct.
    SettingsWindow(Context* context);

    virtual ~SettingsWindow();

    virtual void Init() override;

protected:

    virtual void Create() override;

private:

    void SubscribeToEvents();

    void CreateVideoTab();
    void CreateGraphicsTab();
    void CreateAudioTab();
    void CreateControllersTab();
    void CreateControlsTab();
    void RefreshControlsTab();
    void CreateGameTab();

    /// Create and initialize a Window control.
    void InitWindow();
    /// Create and add various common controls for demonstration purposes.
    void InitControls();
    /// Handle UIOption based control being changed
    void HandleOptionChanged(StringHash eventType, VariantMap& eventData);
    /// Handle "Apply" button in Video settings tab
    void HandleApply(StringHash eventType, VariantMap& eventData);
    /// Handle tab changed
    void HandleTabChanged(StringHash eventType, VariantMap& eventData);
    /// Fill refresh rate options for specified monitor
    void FillRates(int monitor);
    /// Fill resolutions for specified monitor and rate. If rate is -1, fill all resolutions
    void FillResolutions(int monitor, int rate = -1);
    /// Refresh video tab options
    void RefreshVideoOptions();
    /// Apply video tab options
    void ApplyVideoOptions();
    /// Refresh graphics tab options
    void RefreshGraphicsOptions();
    /// Apply graphics tab options
    void ApplyGraphicsOptions();

    /// The Window.
    SharedPtr<Window> window_;

    /// Video controls.
    WeakPtr<UITabPanel> tabs_;
    WeakPtr<UIMultiOption> opt_monitor_;
    WeakPtr<UIMultiOption> opt_fullscreen_;
    WeakPtr<UIMultiOption> opt_rate_;
    WeakPtr<UIMultiOption> opt_resolution_;
    WeakPtr<UIBoolOption> opt_vsync_;
    WeakPtr<Button> btn_apply_;
    WeakPtr<UISliderOption> gamma_;

    /// Misc video controls.
    WeakPtr<UIBoolOption> opt_resizable_;
    WeakPtr<UIMultiOption> opt_fpslimit_;

    /// Graphics controls.
    WeakPtr<UIMultiOption> opt_texture_quality_;
    WeakPtr<UIMultiOption> opt_material_quality_;
    WeakPtr<UIMultiOption> opt_shadows_;
    WeakPtr<UIMultiOption> opt_shadow_quality_;
    WeakPtr<UIMultiOption> opt_occlusion_;
    WeakPtr<UIMultiOption> opt_instancing_;
    WeakPtr<UIMultiOption> opt_specular_;
    WeakPtr<UIMultiOption> opt_hdr_;
    WeakPtr<UIMultiOption> opt_ssao_;

    WeakPtr<UIMultiOption> language_selection_;
    WeakPtr<UIBoolOption> enable_mods_;
    WeakPtr<UIBoolOption> developer_console_;
    WeakPtr<Button> clear_achievements_;

    WeakPtr<UIBoolOption> invert_mouse_x;
    WeakPtr<UIBoolOption> invert_mouse_y;
    WeakPtr<UISliderOption> mouse_sensitivity;
    WeakPtr<UIBoolOption> invert_joystic_x;
    WeakPtr<UIBoolOption> invert_joystick_y;
    WeakPtr<UISliderOption> joystick_sensitivity_x;
    WeakPtr<UISliderOption> joystick_sensitivity_y;
    WeakPtr<UISliderOption> deadzone_;
    WeakPtr<UIBoolOption> multiple_controllers_;
    WeakPtr<UIBoolOption> joystick_as_first_;

    HashMap<String, WeakPtr<UISliderOption>> audio_settings_;

    /// Mark options currently refreshing so "change" handler won't do any work.
    bool refreshing_{};
    /// True when video settings are changed and apply is needed.
    bool needs_apply_{};
    /// Initial window resolution.
    IntVector2 windowed_resolution_{};
    IntVector2 windowed_position_{};

    HashMap<int, WeakPtr<Text>> control_mappings_;
};