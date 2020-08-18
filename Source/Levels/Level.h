#pragma once

#include <Urho3D/Scene/SplinePath.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include "../BaseLevel.h"
#include "Player/Player.h"

namespace Levels {
    class Level : public BaseLevel
    {
        URHO3D_OBJECT(Level, BaseLevel);

    public:
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

#ifdef VOXEL_SUPPORT
        void CreateVoxelWorld();
#endif

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
        void HandleMappedControlPressed(StringHash eventType, VariantMap& eventData);

        bool RaycastFromCamera(Camera* camera, float maxDistance, Vector3& hitPos, Vector3& hitNormal, Drawable*& hitDrawable);

        void ShowPauseMenu();
        void PauseMenuHidden();

        SharedPtr<Player> CreatePlayer(int controllerId, bool controllable, const String& name = String::EMPTY, int nodeID = -1);

        bool showScoreboard_;

        bool drawDebug_;

        HashMap<int, SharedPtr<Player>> players_;
        HashMap<Connection*, SharedPtr<Player>> remotePlayers_;

        SharedPtr<Terrain> terrain_;
    };
}
