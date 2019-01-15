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

        void HandleEndLoading(StringHash eventType, VariantMap& eventData);

        SharedPtr<Text> _status;
        Timer timer;
    };
}