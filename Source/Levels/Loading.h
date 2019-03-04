#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {

    class Loading : public BaseLevel
    {
        URHO3D_OBJECT(Loading, BaseLevel);

    public:
        /// Construct.
        Loading(Context* context);

        virtual ~Loading();
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

        /**
         * Finish loading and move to next level
         */
        void HandleEndLoading(StringHash eventType, VariantMap& eventData);

        /**
         * Status message of the current loading step
         */
        SharedPtr<Text> _status;

        /**
         * Loading bar UI element
         */
        SharedPtr<Sprite> _loadingBar;
    };
}