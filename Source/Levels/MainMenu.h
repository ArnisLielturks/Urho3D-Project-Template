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
        virtual void Init();

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        void HandleStartGame(StringHash eventType, VariantMap& eventData);
        void HandleShowSettings(StringHash eventType, VariantMap& eventData);
		void HandleShowCredits(StringHash eventType, VariantMap& eventData);
        void HandleExit(StringHash eventType, VariantMap& eventData);
        void HandleKeyDown(StringHash eventType, VariantMap& eventData);

        SharedPtr<Button> _startButton;
        SharedPtr<Button> _settingsButton;
		SharedPtr<Button> _creditsButton;
        SharedPtr<Button> _exitButton;
    };
}