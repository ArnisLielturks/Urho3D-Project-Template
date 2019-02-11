#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

class AchievementsWindow : public BaseWindow
{
    URHO3D_OBJECT(AchievementsWindow, BaseWindow);

public:
    /// Construct.
    AchievementsWindow(Context* context);

    virtual ~AchievementsWindow();

    virtual void Init();

private:

    virtual void Create();

    void SubscribeToEvents();

    UIElement* CreateSingleLine();

    Button* CreateItem(const String& image, const String& message, bool completed, int progress, int threshold);

    void HandleAchievementUnlocked(StringHash eventType, VariantMap& eventData);

    SharedPtr<Window> _baseWindow;

    /**
     * Window title bar
     */
    SharedPtr<UIElement> _titleBar;

    SharedPtr<ListView> _listView;

    SharedPtr<UIElement> _activeLine;
};