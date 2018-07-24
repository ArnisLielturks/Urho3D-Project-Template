#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {
    class ExitGame : public BaseLevel
    {
        URHO3D_OBJECT(ExitGame, BaseLevel);

    public:
        /// Construct.
        ExitGame(Context* context);

        virtual ~ExitGame();
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        void Init () override;

    private:
        void SubscribeToEvents();

        void CreateUI();

        Timer _timer;
    };
}