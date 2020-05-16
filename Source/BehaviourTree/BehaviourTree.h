#pragma once

#include <Urho3D/Container/Str.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Controls.h>

#include "BehaviourTreeDefs.h"

using namespace Urho3D;

struct BTService {
    String name;
};

struct BTDecorator {
    String name;
};

struct BTNode {
    BTNodeType nodeType;
    String name;
    Vector<BTService> services;
    Vector<BTNode> childNodes;
    Vector<BTDecorator> decorators;
};

class BehaviourTree : public LogicComponent
{
    URHO3D_OBJECT(BehaviourTree, LogicComponent);

public:
    explicit BehaviourTree(Context* context);

    virtual ~BehaviourTree();

    static void RegisterFactory(Context* context);

    void Init(const String& config);

    const Controls& GetControls();

    void FixedUpdate(float timeStep) override;

private:
    void SubscribeToEvents();

    void LoadConfig(const String& config);

    Controls _controls;

    BTNode _rootNode;
};
