#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "BehaviourTree.h"
#include "../MyEvents.h"
#include "../Global.h"

using namespace Urho3D;

BehaviourTree::BehaviourTree(Context* context):
    LogicComponent(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

BehaviourTree::~BehaviourTree()
{
}

void BehaviourTree::RegisterFactory(Context* context)
{
    context->RegisterFactory<BehaviourTree>();
}

void BehaviourTree::Init(const String& config)
{
    LoadConfig(config);
    SubscribeToEvents();
}

void BehaviourTree::LoadConfig(const String& config)
{
    URHO3D_LOGINFOF("Loading behaviour tree: %s", config.CString());
    auto json = GetSubsystem<ResourceCache>()->GetResource<JSONFile>(config);
    JSONValue& content = json->GetRoot();
    if (content.IsObject()) {
        for (auto it = content.Begin(); it != content.End(); ++it) {
            URHO3D_LOGWARNINGF("JSON: %s", (*it).first_.CString());
            auto field  = (*it).first_;
            auto value = (*it).second_;
            if (field == "type") {
                if (value.IsString()) {
                    if (value.GetString() == "Sequence") {
                        _rootNode.nodeType = SEQUENCE;
                    } else if (value.GetString() == "Selector") {
                        _rootNode.nodeType = SELECTOR;
                    }
                }
            }
        }
    }
}

const Controls& BehaviourTree::GetControls()
{
    return _controls;
}

void BehaviourTree::FixedUpdate(float timeStep)
{
    _controls.yaw_ += timeStep * 100;
}

void BehaviourTree::SubscribeToEvents()
{
    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD,
            MyEvents::ConsoleCommandAdd::P_NAME, "behaviour_debug",
            MyEvents::ConsoleCommandAdd::P_EVENT, "#behaviour_debug",
            MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Change behaviour tree state");
    SubscribeToEvent("#behaviour_debug", [&](StringHash eventType, VariantMap& eventData) {
        _controls.Set(CTRL_FORWARD, true);
    });
}
