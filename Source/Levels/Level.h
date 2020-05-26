#pragma once

#include <Urho3D/Scene/SplinePath.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Network/Connection.h>
#include "../BaseLevel.h"
#include "Player/Player.h"

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
        void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
        static void RegisterObject(Context* context);

    protected:
        virtual void Init () override;

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();
        void UnsubscribeToEvents();
        void RegisterConsoleCommands();
        void StartAudio();
        void StopAllAudio();

        void HandleKeyDown(StringHash eventType, VariantMap& eventData);
        void HandleKeyUp(StringHash eventType, VariantMap& eventData);
        void HandleWindowClosed(StringHash eventType, VariantMap& eventData);
        void HandleBeforeLevelDestroy(StringHash eventType, VariantMap& eventData);
        void HandleControllerConnected(StringHash eventType, VariantMap& eventData);
        void HandleControllerDisconnected(StringHash eventType, VariantMap& eventData);

        void HandleVideoSettingsChanged(StringHash eventType, VariantMap& eventData);
        void HandlePlayerTargetChanged(StringHash eventType, VariantMap& eventData);
        void HandleClientConnected(StringHash eventType, VariantMap& eventData);
        void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);
        void HandleServerConnected(StringHash eventType, VariantMap& eventData);
        void HandleServerDisconnected(StringHash eventType, VariantMap& eventData);
        void ShowPauseMenu();
        void PauseMenuHidden();

        bool _showScoreboard;

        bool _drawDebug;

        HashMap<int, SharedPtr<Player>> _players;
        HashMap<Connection*, SharedPtr<Player>> _remotePlayers;

        SharedPtr<Terrain> _terrain;
    };
}
