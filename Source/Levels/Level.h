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
        void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        virtual void Init();

        virtual void OnLoaded();

    private:
        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        SharedPtr<Node> _controlledNode;
        unsigned int _id;

        bool shouldReturn;
        String returnMessage;

        WeakPtr<Node> _characterCameraNode;
    };
}