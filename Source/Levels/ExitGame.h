#pragma once

#include "../BaseLevel.h"

namespace Levels {
    class ExitGame : public BaseLevel
    {
        URHO3D_OBJECT(ExitGame, BaseLevel);

    public:
        ExitGame(Context* context);
        virtual ~ExitGame();
        static void RegisterObject(Context* context);
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        void Init () override;

    private:
        void SubscribeToEvents();

        void CreateUI();

        Timer _timer;
    };
}
