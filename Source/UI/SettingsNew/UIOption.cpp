#include "UIOption.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Input/InputEvents.h>

namespace Urho3D {

    extern const char* UI_CATEGORY;

// static
    void UIOption::RegisterObject(Context* context) {
        context->RegisterFactory<UIOption>(UI_CATEGORY);

        URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);
        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Name", GetOptionName, SetOptionName, String, "", AM_DEFAULT);
    }

    UIOption::UIOption(Context* context): BorderImage(context) {
        SetFixedHeight(40);
        SetLayoutMode(LM_HORIZONTAL);
        SetLayoutBorder(IntRect(12, 0, 12, 0));
        SetColor(Color(.2f, .2f, .2f, .6f));

        label_ = new Text(context_);
        label_->SetInternal(true);
        label_->SetName("LblName");
        label_->SetMinWidth(120);
        label_->SetVerticalAlignment(VA_CENTER);
        AddChild(label_);
        control_ = new UIElement(context_);
        control_->SetInternal(true);
        control_->SetLayoutMode(LM_HORIZONTAL);
        control_->SetLayoutSpacing(6);
        AddChild(control_);

        SetFocusMode(FM_FOCUSABLE);
        label_->SetStyleAuto();
        control_->SetStyleAuto();
    }

    UIOption::~UIOption() {

    }

    void UIOption::SetOptionName(const String& name) {
        option_name_ = name;
        if (label_)
            label_->SetText(option_name_);
    }

    void UIOption::OnChanged() {
        Urho3D::VariantMap& eventData = GetEventDataMap();
        using namespace UIOptionChanged;
        eventData[P_OPTION] = this;
        SendEvent(E_UIOPTION_CHANGED, eventData);
    }

    void UIOption::OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) {
        if (key == KEY_UP || key == KEY_DOWN) {
            using namespace KeyDown;
            VariantMap& eventData = GetEventDataMap();
            eventData[P_KEY] = KEY_TAB;
            eventData[P_BUTTONS] = (unsigned)buttons;
            eventData[P_QUALIFIERS] = key == KEY_UP ? QUAL_SHIFT : 0;
            SendEvent(E_KEYDOWN, eventData);
        }
    }

// static
    void UIBoolOption::RegisterObject(Context* context) {
        context->RegisterFactory<UIBoolOption>(UI_CATEGORY);

        URHO3D_COPY_BASE_ATTRIBUTES(UIOption);

        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Value", GetOptionValue, SetOptionValue, bool, false, AM_DEFAULT);
    }

    UIBoolOption::UIBoolOption(Context* context)
            : UIOption(context), value_(false) {
        for (int i = 0; i < sizeof(buttons_) / sizeof(buttons_[0]); i++) {
            SharedPtr<Button> btn(new Button(context_));
            btn->SetName(i == 0 ? "BtnOff" : "BtnOn");
            btn->SetInternal(true);
            btn->SetFixedSize(128, 32);
            btn->SetVerticalAlignment(VA_CENTER);
            btn->SetFocusMode(FM_NOTFOCUSABLE);

            SharedPtr<BorderImage> status(new BorderImage(context_));
            status->SetName("Status");
            status->SetInternal(true);
            status->SetColor(Color::RED);
            status->SetFixedSize(16, 16);
            status->SetPosition(10, 0);
            status->SetVerticalAlignment(VA_CENTER);
            status->SetEnabled(false);
            status->SetVisible(false);
            btn->AddChild(status);

            SharedPtr<Text> label(new Text(context_));
            label->SetInternal(true);
            label->SetText(i == 0 ? "Off" : "On");
            label->SetAlignment(HA_CENTER, VA_CENTER);
            btn->AddChild(label);

            control_->AddChild(btn);

            btn->SetStyleAuto();
            label->SetStyleAuto();

            SubscribeToEvent(btn, E_RELEASED, URHO3D_HANDLER(UIBoolOption, HandleButton));
            buttons_[i] = btn;
        }
        UpdateValue();
    }

    UIBoolOption::~UIBoolOption() {

    }

    void UIBoolOption::SetOptionValue(bool value) {
        bool old_val = value_;
        value_ = value;
        if (old_val != value_) {
            UpdateValue();
            OnChanged();
        }
    }

    void UIBoolOption::HandleButton(StringHash eventType, VariantMap& eventData) {
        using namespace Released;
        auto element = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
        if (element->GetName() == "BtnOff") {
            SetOptionValue(false);
        } else if (element->GetName() == "BtnOn") {
            SetOptionValue(true);
        }
    }

    void UIBoolOption::SetOptionLabel(bool state, const String& text) {
        auto label = GetChildDynamicCast<Text>(state ? "BtnOff" : "BtnOn", true);
        if (label)
            label->SetText(text);
    }

    String UIBoolOption::GetOptionLabel(bool state) const {
        auto label = GetChildDynamicCast<Text>(state ? "BtnOff" : "BtnOn", true);
        if (label)
            return label->GetText();
        return "";
    }

    void UIBoolOption::OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) {
        if (key == KEY_LEFT) {
            SetOptionValue(false);
        } else if (key == KEY_RIGHT) {
            SetOptionValue(true);
        }
        UIOption::OnKey(key, buttons, qualifiers);
    }

    void UIBoolOption::UpdateValue() {
        if (buttons_[0]) {
            auto status = buttons_[0]->GetChild("Status", true);
            if (status) {
                status->SetEnabled(!value_);
                status->SetVisible(!value_);
            }
        }
        if (buttons_[1]) {
            auto status = buttons_[1]->GetChild("Status", true);
            if (status) {
                status->SetEnabled(value_);
                status->SetVisible(value_);
            }
        }
        MarkDirty();
    }

    static const StringVector optionsVectorNames =
            {
                    "Option Count",
                    "   Text",
            };

// static
    void UIMultiOption::RegisterObject(Context* context) {
        context->RegisterFactory<UIMultiOption>(UI_CATEGORY);

        URHO3D_COPY_BASE_ATTRIBUTES(UIOption);

        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Index", GetOptionIndex, SetOptionIndex, int, -1, AM_DEFAULT);
        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Strings", GetOptionsAttr, SetOptionsAttr, VariantVector, Variant::emptyVariantVector, AM_DEFAULT)
                .SetMetadata(AttributeMetadata::P_VECTOR_STRUCT_ELEMENTS, optionsVectorNames);
    }

    UIMultiOption::UIMultiOption(Context* context)
            : UIOption(context), index_(0), options_count_(0) {
        for (int i = 0; i < sizeof(buttons_) / sizeof(buttons_[0]); i++) {
            SharedPtr<Button> btn(new Button(context_));
            btn->SetName(i == 0 ? "BtnLeft" : "BtnRight");
            btn->SetInternal(true);
            btn->SetFixedSize(32, 32);
            btn->SetVerticalAlignment(VA_CENTER);
            btn->SetFocusMode(FM_NOTFOCUSABLE);

            SharedPtr<Text> label(new Text(context_));
            label->SetText(i == 0 ? "<" : ">");
            label->SetInternal(true);
            label->SetAlignment(HA_CENTER, VA_CENTER);

            btn->AddChild(label);

            btn->SetStyleAuto();
            label->SetStyleAuto();

            buttons_[i] = btn;
        }

        auto middle = new BorderImage(context_);
        middle->SetColor(Color::TRANSPARENT_BLACK);
        middle->SetInternal(true);
        middle->SetStyleAuto();
        lbl_value_ = new Text(context_);
        lbl_value_->SetInternal(true);
        lbl_value_->SetAlignment(HA_CENTER, VA_CENTER);
        lbl_value_->SetStyleAuto();

        middle->AddChild(lbl_value_);
        middle->SetFixedSize(180, 32);
        middle->SetVerticalAlignment(VA_CENTER);

        control_->AddChild(buttons_[0]);
        control_->AddChild(middle);
        control_->AddChild(buttons_[1]);

        SubscribeToEvent(buttons_[0], E_RELEASED, URHO3D_HANDLER(UIMultiOption, HandleButton));
        SubscribeToEvent(buttons_[1], E_RELEASED, URHO3D_HANDLER(UIMultiOption, HandleButton));
    }

    UIMultiOption::~UIMultiOption() {

    }

    void UIMultiOption::UpdateValue() {
        if (!strings_.Empty() && index_ >= 0 && index_ < (int)strings_.Size()) {
            if (lbl_value_)
                lbl_value_->SetText(strings_[index_]);
            MarkDirty();
        }
    }

    void UIMultiOption::SetOptionIndex(int value) {
        int old_val = index_;
        index_ = value;
        if (index_ < 0)
            index_ = 0;
        if (index_ >= options_count_)
            index_ = options_count_ - 1;

        if (old_val != index_) {
            UpdateValue();
            OnChanged();
        }
    }

    void UIMultiOption::SetNumberOfOptions(int count) {
        options_count_ = count;
    }

    void UIMultiOption::SetStrings(const StringVector& strings) {
        strings_ = strings;
        options_count_ = strings_.Size();
        UpdateValue();
    }

    void UIMultiOption::SetOptionsAttr(const VariantVector& options) {
        unsigned index = 0;
        options_count_ = index < options.Size() ? options[index++].GetUInt() : 0;

        // Prevent negative value being assigned from the editor
        if (options_count_ > M_MAX_INT)
            options_count_ = 0;

        strings_.Resize(options_count_);

        for (auto it = strings_.Begin(); it != strings_.End() && index < options.Size(); ++it) {
            *it = options[index++].GetString();
        }
        UpdateValue();
    }

    VariantVector UIMultiOption::GetOptionsAttr() const {
        VariantVector ret;
        ret.Reserve(options_count_ + 1);
        ret.Push(options_count_);

        for (auto it = strings_.Begin(); it != strings_.End(); ++it) {
            ret.Push(*it);
        }
        return ret;
    }

    String UIMultiOption::GetValue() const {
        if (strings_.Empty())
            return "";
        if (index_ >= 0 && index_ < (int)strings_.Size())
            return strings_[index_];
        return "";
    }

    void UIMultiOption::HandleButton(StringHash eventType, VariantMap& eventData) {
        using namespace Released;

        if (options_count_ <= 0)
            return;

        auto element = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
        if (element->GetName() == "BtnLeft") {
            SetOptionIndex(index_ - 1);
        } else if (element->GetName() == "BtnRight") {
            SetOptionIndex(index_ + 1);
        }
    }

    void UIMultiOption::OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) {
        if (key == KEY_LEFT) {
            SetOptionIndex(index_ - 1);
        } else if (key == KEY_RIGHT) {
            SetOptionIndex(index_ + 1);
        }
        UIOption::OnKey(key, buttons, qualifiers);
    }


//static
    void UISliderOption::RegisterObject(Context* context) {
        context->RegisterFactory<UISliderOption>(UI_CATEGORY);

        URHO3D_COPY_BASE_ATTRIBUTES(UIOption);

        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Value", GetValue, SetValue, float, 0.f, AM_DEFAULT);
        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Option Range", GetRange, SetRange, float, 1.f, AM_DEFAULT);
    }

    UISliderOption::UISliderOption(Context* context): UIOption(context), range_(1.f), value_(0.f) {
        slider_ = new Slider(context_);
        slider_->SetInternal(true);
        slider_->SetValue(0);
        control_->AddChild(slider_);

        lbl_value_ = new Text(context_);
        lbl_value_->SetInternal(true);
        lbl_value_->SetStyleAuto();
        lbl_value_->SetAlignment(HA_CENTER, VA_CENTER);
        SharedPtr<BorderImage> value_container(new BorderImage(context_));
        value_container->SetInternal(true);
        value_container->AddChild(lbl_value_);
        control_->AddChild(value_container);

        SetRange(range_);
        SetValue(value_);

        slider_->SetFixedHeight(32);
        slider_->SetStyleAuto();
        slider_->SetVerticalAlignment(VA_CENTER);

        value_container->SetVerticalAlignment(VA_CENTER);
        value_container->SetColor(Color::TRANSPARENT_BLACK);
        value_container->SetStyleAuto();
        value_container->SetFixedSize(32, 32);

        SubscribeToEvent(slider_, E_SLIDERCHANGED, URHO3D_HANDLER(UISliderOption, HandleSlider));
    }

    UISliderOption::~UISliderOption() {

    }

    void UISliderOption::SetRange(float range) {
        range_ = range;
        if (slider_)
            slider_->SetRange(range_);
    }

    void UISliderOption::SetValue(float value) {
        value_ = value;
        if (value_ > range_)
            value_ = range_;
        else if (value_ < 0)
            value_ = 0;

        if (slider_)
            slider_->SetValue(value_);

        if (lbl_value_)
            lbl_value_->SetText(ToString("%3.1f", value_));
    }

    void UISliderOption::OnKey(Key key, MouseButtonFlags buttons, QualifierFlags qualifiers) {
        auto step = range_ / 10;

        if (key == KEY_LEFT) {
            SetValue(GetValue() - step);
        } else if (key == KEY_RIGHT) {
            SetValue(GetValue() + step);
        }
        UIOption::OnKey(key, buttons, qualifiers);
    }

    void UISliderOption::HandleSlider(StringHash eventType, VariantMap& eventData) {
        using namespace SliderChanged;
        auto value = eventData[P_VALUE].GetFloat();
        SetValue(value);

        VariantMap& data = GetEventDataMap();
        data[UISliderChanged::P_UISLIDER] = this;
        SendEvent(E_UISLIDER_CHANGED, data);
    }


    static const StringVector tabVectorNames =
            {
                    "Tab Count",
                    "   Name",
            };

// static
    void UITabPanel::RegisterObject(Context* context) {
        context->RegisterFactory<UITabPanel>(UI_CATEGORY);

        URHO3D_COPY_BASE_ATTRIBUTES(BorderImage);

        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Current Tab", GetCurrentTab, SetCurrentTab, int, 0, AM_DEFAULT);
        URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Tabs", GetTabsAttribute, SetTabsAttribute, VariantVector, Variant::emptyVariantVector, AM_DEFAULT)
                .SetMetadata(AttributeMetadata::P_VECTOR_STRUCT_ELEMENTS, tabVectorNames);
    }

    UITabPanel::UITabPanel(Context* context): BorderImage(context), tab_count_(0), current_tab_(0) {
        SetColor(Color::TRANSPARENT_BLACK);
        SetLayoutMode(LM_VERTICAL);

        header_ = new BorderImage(context_);
        header_->SetName("TP_Header");
        header_->SetInternal(true);
        header_->SetLayoutMode(LM_HORIZONTAL);
        header_->SetLayoutSpacing(6);
        header_->SetFixedHeight(30);
        header_->SetHorizontalAlignment(HA_LEFT);
        header_->SetColor(Color::TRANSPARENT_BLACK);
        // the whole header is focusable so we can use arrow keys to navigate
        header_->SetFocusMode(FM_FOCUSABLE);

        body_ = new BorderImage(context_);
        body_->SetName("TP_Body");
        body_->SetInternal(true);
        body_->SetHorizontalAlignment(HA_LEFT);
        body_->SetLayoutMode(LM_VERTICAL);

        AddChild(header_);
        AddChild(body_);

        header_->SetStyleAuto();
        body_->SetStyle("Window");
        //header_->SetStyle("UITabHeader");
        //body_->SetStyle("UITabBody");

        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(UITabPanel, HandleKeyPress));
    }

    UITabPanel::~UITabPanel() {

    }

    void UITabPanel::SetTabsAttribute(const VariantVector& strings) {
        unsigned index = 0;
        tab_count_ = index < strings.Size() ? strings[index++].GetUInt() : 0;

        // Prevent negative value being assigned from the editor
        if (tab_count_ > M_MAX_INT)
            tab_count_ = 0;

        tab_names_.Resize(tab_count_);

        for (auto it = tab_names_.Begin(); it != tab_names_.End() && index < strings.Size(); ++it) {
            *it = strings[index++].GetString();
        }

        // Update tabs
        int tab_diff = tab_count_ - tabs_.Size();

        if (tab_diff > 0) {
            for (int i = 0; i < tab_diff; ++i) {
                if (0 == AddTab(tab_names_[tabs_.Size()]))
                    break;
            }
        } else if (tab_diff < 0) {
            for (int i = tab_diff; i < 0; i++) {
                RemoveTab(tabs_.Size() - 1);
            }
        }

        for (int i = 0; i < (int)tabs_.Size(); i++) {
            tabs_[i].name = tab_names_[i];
            tabs_[i].text->SetText(tab_names_[i]);
            tabs_[i].page->SetName(tab_names_[i]);
        }

        SetCurrentTab(current_tab_);
    }

    VariantVector UITabPanel::GetTabsAttribute() const {
        VariantVector ret;
        ret.Reserve(tab_count_ + 1);
        ret.Push(tab_count_);

        for (auto it = tab_names_.Begin(); it != tab_names_.End(); ++it) {
            ret.Push(*it);
        }
        return ret;
    }

    UIElement* UITabPanel::AddTab(const String& name) {
        int tab_index = tabs_.Size();
        bool enabled = current_tab_ == tab_index;

        Tab tab;
        tab.name = name;
        tab.button = new Button(context_);
        // buttons are not focusable, upper element is
        tab.button->SetFocusMode(FM_NOTFOCUSABLE);
        tab.button->SetInternal(true);
        tab.button->SetStyle("TabButton");
        tab.button->SetFixedSize(100, 30);
        tab.button->SetVar("tab_index", Variant(tab_index));
        SubscribeToEvent(tab.button, E_PRESSED, URHO3D_HANDLER(UITabPanel, HandleTabButton));
        header_->AddChild(tab.button);

        tab.text = new Text(context_);
        tab.text->SetInternal(true);
        tab.text->SetStyleAuto();
        tab.text->SetText(name);
        tab.text->SetAlignment(HA_CENTER, VA_CENTER);
        tab.button->AddChild(tab.text);

        tab.page = new UIElement(context_);
        tab.page->SetInternal(true);
        tab.page->SetVisible(enabled);
        tab.page->SetColor(Color::RED);

        body_->AddChild(tab.page);

        tabs_.Push(tab);

        if (tabs_.Size() == 1) {
            tab.button->SetStyle("TabButtonSelected");
        }

        //tab.button->SetStyle("UITabButton");

        return tab.page.Get();
    }

    void UITabPanel::RemoveTab(const String& name) {
        auto index = -1;
        // find the tab index
        for (auto it = tabs_.Begin(); it != tabs_.End(); ++it, ++index) {
            if (it->name == name)
                break;
        }

        if (index != 1)
            RemoveTab(index);
    }

    void UITabPanel::RemoveTab(int index) {
        if (index < 0 || index >= (int)tabs_.Size())
            return;

        auto tab = tabs_[index];
        tab.text->Remove();
        tab.button->Remove();
        tab.page->Remove();

        tabs_.Erase(index, 1);
        tab_count_ = tabs_.Size();
    }

    void UITabPanel::RemoveAllTabs() {
        for (auto it = tabs_.Begin(); it != tabs_.End(); ++it) {
            it->text->Remove();
            it->button->Remove();
            it->page->Remove();
        }
        tabs_.Clear();
    }

    UIElement* UITabPanel::GetTab(int index) const {
        if (index < 0 || index >= (int)tab_count_ || index >= (int)tabs_.Size())
            return 0;

        return tabs_[index].page;
    }

    void UITabPanel::SetCurrentTab(int tab) {
        if (tab >= (int)tabs_.Size())
            tab = tabs_.Size();

        if (tab < 0)
            tab = 0;

        bool tab_changed = current_tab_ = tab;
        current_tab_ = tab;

        // hide all tabs first
        for (int i = 0; i < (int)tabs_.Size(); ++i) {
            tabs_[i].page->SetVisible(false);
            tabs_[i].button->SetStyle("TabButton");
        }

        // show only the current tab
        for (int i = 0; i < (int)tabs_.Size(); ++i) {
            tabs_[i].page->SetVisible(tab == i);
            if (tab == i) {
                tabs_[i].button->SetStyle("TabButtonSelected");
            }
        }

        if (tab_changed) {
            using namespace UITabChanged;
            VariantMap& eventData = GetEventDataMap();
            eventData[P_UITABPANEL] = this;
            eventData[P_INDEX] = current_tab_;
            SendEvent(E_UITAB_CHANGED, eventData);
        }
    }

    void UITabPanel::HandleTabButton(StringHash eventType, VariantMap& eventData) {
        using namespace Pressed;
        auto element = static_cast<UIElement*>(eventData[P_ELEMENT].GetVoidPtr());
        Variant tab_index = element->GetVar("tab_index");

        if (tab_index.IsEmpty())
            return;

        SetCurrentTab(tab_index.GetInt());
    }

    void UITabPanel::HandleKeyPress(StringHash eventType, VariantMap& eventData) {
        if (!header_->HasFocus())
            return;

        using namespace KeyDown;
        auto key = eventData[P_KEY].GetInt();

        if (key == KEY_LEFT) {
            SetCurrentTab(current_tab_ - 1);
        } else if (key == KEY_RIGHT) {
            SetCurrentTab(current_tab_ + 1);
        } else if (key == KEY_UP || key == KEY_DOWN) {
            using namespace KeyDown;
            VariantMap& eventData = GetEventDataMap();
            eventData[P_KEY] = KEY_TAB;
            eventData[P_BUTTONS] = 0;
            eventData[P_QUALIFIERS] = key == KEY_UP ? QUAL_SHIFT : 0;
            SendEvent(E_KEYDOWN, eventData);
        }
    }

} // namespace Urho3D