#ifndef __ENGINE_UI_OPTION_H__
#define __ENGINE_UI_OPTION_H__
#pragma once

#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/Core/Object.h>

namespace Urho3D {

/// UI option was changed
    URHO3D_EVENT(E_UIOPTION_CHANGED, UIOptionChanged)
    {
        URHO3D_PARAM(P_OPTION, Option); // UIOption pointer
    }

    URHO3D_EVENT(E_UITAB_CHANGED, UITabChanged)
    {
        URHO3D_PARAM(P_UITABPANEL, TabPanel); // UITabPanel pointer
        URHO3D_PARAM(P_INDEX, TabIndex); // int
    }

    URHO3D_EVENT(E_UISLIDER_CHANGED, UISliderChanged)
    {
        URHO3D_PARAM(P_UISLIDER, Slider); // UISlider pointer
    }

    class UIOption : public BorderImage {
    URHO3D_OBJECT(UIOption, BorderImage)
    public:
        /// Construct.
        explicit UIOption(Context* context);
        /// Destruct.
        ~UIOption() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetOptionName(const String& name);
        String GetOptionName() const { return option_name_; }
    protected:
        String option_name_;
        SharedPtr<Text> label_;
        SharedPtr<UIElement> control_;

        void OnChanged();
        void OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) override;
    };

    class UIBoolOption : public UIOption {
    URHO3D_OBJECT(UIBoolOption, UIOption)
    public:
        UIBoolOption(Context* context);
        ~UIBoolOption() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetOptionValue(bool value);
        bool GetOptionValue() const { return value_; }

        void SetOptionLabel(bool state, const String& text);
        String GetOptionLabel(bool state) const;
    private:
        bool value_;
        SharedPtr<Button> buttons_[2];

        void HandleButton(StringHash eventType, VariantMap& eventData);
        void OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) override;

        void UpdateValue();
    };

    class UIMultiOption : public UIOption {
    URHO3D_OBJECT(UIMultiOption, UIOption)
    public:
        UIMultiOption(Context* context);
        ~UIMultiOption() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetOptionIndex(int value);
        int GetOptionIndex() const { return index_; }

        void SetNumberOfOptions(int count);
        int GetNumberOfOptions() const { return options_count_; }

        void SetOptionsAttr(const VariantVector& strings);
        VariantVector GetOptionsAttr() const;

        void SetStrings(const StringVector& strings);
        StringVector GetStrings() const { return strings_; }

        String GetValue() const;
    private:
        int options_count_;
        Vector<String> strings_;
        int index_;

        // Arrow buttons
        SharedPtr<Button> buttons_[2];
        // Value text
        SharedPtr<Text> lbl_value_;

        void UpdateValue();
        void HandleButton(StringHash eventType, VariantMap& eventData);
        void OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) override;
    };

    class UISliderOption : public UIOption {
    URHO3D_OBJECT(UISliderOption, UIOption)
    public:
        UISliderOption(Context* context);
        ~UISliderOption() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetRange(float range);
        float GetRange() const { return range_; }

        void SetValue(float value);
        float GetValue() const { return value_; }
    private:
        float range_;
        float value_;
        SharedPtr<Slider> slider_;
        SharedPtr<Text> lbl_value_;

        void OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) override;
        void HandleSlider(StringHash eventType, VariantMap& eventData);
    };

    class UITabPanel : public BorderImage {
    URHO3D_OBJECT(UITabPanel, BorderImage)
    public:
        UITabPanel(Context* context);
        ~UITabPanel() override;
        /// Register object factory.
        static void RegisterObject(Context* context);

        void SetTabsAttribute(const VariantVector& strings);
        VariantVector GetTabsAttribute() const;

        void SetCurrentTab(int tab);
        int GetCurrentTab() const { return current_tab_; }

        unsigned GetTabCount() const { return tabs_.Size(); }

        UIElement* AddTab(const String& name);
        void RemoveTab(const String& name);
        void RemoveTab(int index);
        void RemoveAllTabs();
        UIElement* GetTab(int index) const;
    private:
        unsigned tab_count_;
        StringVector tab_names_;

        int current_tab_;
        struct Tab {
            String name;
            SharedPtr<Button> button;
            SharedPtr<Text> text;
            SharedPtr<UIElement> page;
        };
        Vector<Tab> tabs_;

        SharedPtr<UIElement> header_;
        SharedPtr<UIElement> body_;

        void HandleTabButton(StringHash eventType, VariantMap& eventData);
        void HandleKeyPress(StringHash eventType, VariantMap& eventData);
    };

} // namespace Urho3D

#endif