#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"
#include <vector>

namespace Levels {

    class MainMenu : public BaseLevel
    {
    URHO3D_OBJECT(MainMenu, BaseLevel);

    public:
        /// Construct.
        MainMenu(Context* context);

        ~MainMenu();

    protected:
        void Init () override;

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        SharedPtr<Button> _newGameButton;
        SharedPtr<Button> _settingsButton;
        SharedPtr<Button> _achievementsButton;
        SharedPtr<Button> _creditsButton;
        SharedPtr<Button> _exitButton;

        Button* CreateButton(const String& text, int width, IntVector2 position);
    };
}