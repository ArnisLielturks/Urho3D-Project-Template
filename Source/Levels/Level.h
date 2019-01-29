#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {
    class Level : public BaseLevel
    {
        URHO3D_OBJECT(Level, BaseLevel);

    public:
        /// Construct.
        Level(Context* context);

        virtual ~Level();
        void HandlePhysicsPrestep(StringHash eventType, VariantMap& eventData);
        void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        virtual void Init () override;

        void OnLoaded();

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();
        void UnsubscribeToEvents();

        void StartAudio();
        void StopAllAudio();

        void HandleKeyDown(StringHash eventType, VariantMap& eventData);
        void HandleKeyUp(StringHash eventType, VariantMap& eventData);
        void HandleWindowClosed(StringHash eventType, VariantMap& eventData);

        void HandleControllerConnected(StringHash eventType, VariantMap& eventData);
        void HandleControllerDisconnected(StringHash eventType, VariantMap& eventData);

        void HandleVideoSettingsChanged(StringHash eventType, VariantMap& eventData);

        Node* CreateControllableObject();

        bool _showScoreboard;

        /**
        * Player variables
        */
        HashMap<int, SharedPtr<Node>> _players;

        SharedPtr<SplinePath> _path;

        HashMap<Node*, SharedPtr<Node>> _playerLabels;
    };
}