#include <Urho3D/Urho3DAll.h>
#include "BaseWindow.h"

/// Construct.
BaseWindow::BaseWindow(Context* context, IntVector2 size):
    Object(context)
{
    SubscribeToBaseEvents();
    _size = size;
    Init();
}

BaseWindow::~BaseWindow()
{
    Dispose();
}

void BaseWindow::Init()
{
    UI* ui = GetSubsystem<UI>();
    _base = ui->GetRoot()->CreateChild<Window>();
    _base->SetStyleAuto();
    _base->SetSize(_size);
    _base->SetAlignment(HA_CENTER, VA_CENTER);
}

SharedPtr<UIElement> BaseWindow::CreateMenu(UIElement* parent, const String& label, StringVector options, int selected, IntVector2 position, EventHandler* handler)
{
    SharedPtr<UIElement> container(parent->CreateChild<UIElement>());
    container->SetPosition(position);
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 8);
    _base->AddChild(container);

    SharedPtr<Text> text(new Text(context_));
    container->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetMinWidth(200);
    //text->AddTag(TEXT_TAG);

    SharedPtr<DropDownList> list(new DropDownList(context_));
    container->AddChild(list);
    list->SetMinWidth(150);
    list->SetStyleAuto();
	list->SetName(label);
	SubscribeToEvent(list, E_ITEMSELECTED, handler);

    for (auto it = options.Begin(); it != options.End(); ++it)
    {
        SharedPtr<Text> item(new Text(context_));
        list->AddItem(item);
        item->SetText((*it));
		item->SetName((*it));
        item->SetStyleAuto();
        item->SetMinWidth(150);
        //item->AddTag(TEXT_TAG);
    }

    list->SetSelection(selected);

    //text->SetMaxWidth(text->GetRowWidth(0));

    //SubscribeToEvent(list, E_ITEMSELECTED, handler);
    return container;
}

SharedPtr<UIElement> BaseWindow::CreateCheckbox(UIElement* parent, const String& label, bool isActive, IntVector2 position, EventHandler* handler)
{
    SharedPtr<UIElement> container(parent->CreateChild<UIElement>());
    container->SetPosition(position);
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 8);
    _base->AddChild(container);

    SharedPtr<Text> text(new Text(context_));
    container->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetMinWidth(200);

    SharedPtr<CheckBox> box(new CheckBox(context_));
    container->AddChild(box);
	box->SetName(label);
    box->SetStyleAuto();
    box->SetChecked(isActive);
	SubscribeToEvent(box, E_TOGGLED, handler);

    // text->AddTag(TEXT_TAG);

    //SubscribeToEvent(box, E_TOGGLED, handler);
    return container;
}

SharedPtr<UIElement> BaseWindow::CreateSlider(UIElement* parent, const String& text, IntVector2 position, float value)
{
	SharedPtr<UIElement> container(parent->CreateChild<UIElement>());
	container->SetPosition(position);
	container->SetAlignment(HA_LEFT, VA_TOP);
	container->SetLayout(LM_HORIZONTAL, 8);

	auto* cache = GetSubsystem<ResourceCache>();
	auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	// Create text and slider below it
	auto* sliderText = container->CreateChild<Text>();
	//sliderText->SetPosition(IntV);
	sliderText->SetStyleAuto();
	sliderText->SetFont(font, 12);
	sliderText->SetText(text);
	sliderText->SetMinWidth(200);

	auto* slider = container->CreateChild<Slider>();
	slider->SetStyleAuto();
	//slider->SetPosition(position);
	//slider->SetSize(size);
	slider->SetMinWidth(150);
	// Use 0-1 range for controlling sound/music master volume
	slider->SetRange(1.0f);
	slider->SetValue(value);

	return container;
}

SharedPtr<UIElement> BaseWindow::CreateControlsElement(const String& text, IntVector2 position, String value, String actionName, EventHandler* handler)
{
	SharedPtr<UIElement> container(new UIElement(context_));//(_base->CreateChild<UIElement>());
    container->SetStyleAuto();
	container->SetPosition(position);
    container->SetMinHeight(30);
	container->SetAlignment(HA_LEFT, VA_TOP);
	container->SetLayout(LM_HORIZONTAL, 8, IntRect(4, 4, 4, 4));

	auto* cache = GetSubsystem<ResourceCache>();
	auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	// Create text and slider below it
	auto* label = container->CreateChild<Text>();
	//sliderText->SetPosition(IntV);
	label->SetStyleAuto();
	label->SetFont(font, 12);
	label->SetText(text);
	label->SetMinWidth(250);
    label->SetAlignment(HA_LEFT, VA_CENTER);
    label->SetPosition(IntVector2(10, 0));

    Button* button = container->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetMinWidth(40);
    button->SetVar("ActionName", actionName);
    button->SetPosition(IntVector2(-10, 0));
    button->SetFocusMode(FM_RESETFOCUS);

    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetText(value);
    buttonText->SetName(actionName);
    buttonText->SetStyleAuto();
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    SubscribeToEvent(button, E_RELEASED, handler);

	// auto* lineEdit = container->CreateChild<LineEdit>();
	// lineEdit->SetStyleAuto();
	// lineEdit->SetMinWidth(130);
    // lineEdit->SetText(value);
    // lineEdit->SetVar("ActionName", actionName);

	return container;
}

Button* BaseWindow::CreateButton(UIElement* parent, String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign)
{
    Button* button = parent->CreateChild<Button>();
    button->SetSize(size);
    button->SetPosition(position);
    button->SetStyleAuto();
    button->SetAlignment(hAlign, vAlign);
    button->SetFocusMode(FM_RESETFOCUS);

    Text* text = button->CreateChild<Text>();
    text->SetText(name);
    text->SetStyleAuto();
    text->SetAlignment(HA_CENTER, VA_CENTER);

    return button;
}
