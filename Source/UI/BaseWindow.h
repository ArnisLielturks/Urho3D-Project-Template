#pragma once

#include <Urho3D/Urho3DAll.h>

class BaseWindow : public Object
{
    URHO3D_OBJECT(BaseWindow, Object);
public:
    BaseWindow(Context* context, IntVector2 size = IntVector2(300, 200));

    virtual ~BaseWindow();

    virtual void Init();

    SharedPtr<UIElement> CreateMenu(UIElement* parent, const String& label, StringVector options, int selected, IntVector2 position, EventHandler* handler = nullptr);
    Button* CreateButton(UIElement* parent, String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign);
    SharedPtr<UIElement> CreateCheckbox(UIElement* parent, const String& label, bool isActive, IntVector2 position, EventHandler* handler = nullptr);
	SharedPtr<UIElement> CreateSlider(UIElement* parent, const String& text, IntVector2 position, float value);
    SharedPtr<UIElement> CreateControlsElement(const String& text, IntVector2 position, String value, String actionName, EventHandler* handler);

private:
    void SubscribeToBaseEvents()
    {
    }

    virtual void Create() {};

protected:

    virtual void Dispose()
    {
        if (_base) {
            _base->Remove();
        }
    }

    SharedPtr<Window> _base;
    IntVector2 _size;
};