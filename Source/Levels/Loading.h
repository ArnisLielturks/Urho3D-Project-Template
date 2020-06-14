#pragma once

#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Sprite.h>
#include "../BaseLevel.h"

namespace Levels {

    class Loading : public BaseLevel
    {
        URHO3D_OBJECT(Loading, BaseLevel);

    public:
        Loading(Context* context);
        virtual ~Loading();
        static void RegisterObject(Context* context);
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        void Init () override;

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        /**
         * Horizontal progress bar at the bottom of the screen
         */
        void CreateProgressBar();

        void UpdateStatusMesage();

        /**
         * Finish loading and move to next level
         */
        void HandleEndLoading(StringHash eventType, VariantMap& eventData);
        void HandleServerDisconnected(StringHash eventType, VariantMap& eventData);
        void HandleSceneLoadFailed(StringHash eventType, VariantMap& eventData);
        void HandleConnectFailed(StringHash eventType, VariantMap& eventData);
        void HandleServerConnected(StringHash eventType, VariantMap& eventData);
        void HandleRemoteClientID(StringHash eventType, VariantMap& eventData);
        void HandleLoadingStepFailed(StringHash eventType, VariantMap& eventData);

        void SearchPlayerNode();

        bool searchPlayerNode_{false};
        /**
         * Status message of the current loading step
         */
        SharedPtr<Text> status_;

        /**
         * Loading bar UI element
         */
        SharedPtr<Sprite> loadingBar_;

        String statusMessage_;
    };
}
