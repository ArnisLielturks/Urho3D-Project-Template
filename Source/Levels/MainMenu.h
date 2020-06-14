#pragma once

#include <Urho3D/UI/Button.h>
#include "../BaseLevel.h"
#include <vector>

using namespace Urho3D;

namespace Levels {

    class MainMenu : public BaseLevel
    {
    URHO3D_OBJECT(MainMenu, BaseLevel);

    public:
        MainMenu(Context* context);
        ~MainMenu();
        static void RegisterObject(Context* context);

    protected:
        void Init () override;

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        void HandleUpdate(StringHash eventType, VariantMap& eventData);

        void InitCamera();

        SharedPtr<Button> newGameButton_;
        SharedPtr<Button> settingsButton_;
        SharedPtr<Button> achievementsButton_;
        SharedPtr<Button> creditsButton_;
        SharedPtr<Button> exitButton_;
        SharedPtr<Node> cameraRotateNode_;
        SharedPtr<UIElement> buttonsContainer_;
        List<SharedPtr<Button>> dynamicButtons_;

        Button* CreateButton(const String& text);
    };
}
