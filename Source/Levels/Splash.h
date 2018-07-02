#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {

    class Splash : public BaseLevel
    {
        URHO3D_OBJECT(Splash, BaseLevel);

    public:
        /// Construct.
        Splash(Context* context);

        virtual ~Splash();
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        virtual void Init();

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        void HandleEndSplash();

        Timer _timer;
    };
}