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
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        void Init () override;

    private:

        void draw();
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        SharedPtr<Button> _startButton;
        SharedPtr<Button> _settingsButton;
		SharedPtr<Button> _creditsButton;
        SharedPtr<Button> _exitButton;

        bool _active;

        bool _showGUI;
    };
}