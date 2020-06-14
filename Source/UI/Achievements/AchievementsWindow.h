#pragma once
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ListView.h>
#include "../BaseWindow.h"

class AchievementsWindow : public BaseWindow
{
    URHO3D_OBJECT(AchievementsWindow, BaseWindow);

public:
    AchievementsWindow(Context* context);

    virtual ~AchievementsWindow();

    virtual void Init() override;

private:

    virtual void Create() override;

    void SubscribeToEvents();

    UIElement* CreateSingleLine();

    Button* CreateItem(const String& image, const String& message, bool completed, int progress, int threshold);

    void HandleAchievementUnlocked(StringHash eventType, VariantMap& eventData);

    SharedPtr<Window> baseWindow_;

    /**
     * Window title bar
     */
    SharedPtr<UIElement> titleBar_;

    SharedPtr<ListView> listView_;

    SharedPtr<UIElement> activeLine_;
};
