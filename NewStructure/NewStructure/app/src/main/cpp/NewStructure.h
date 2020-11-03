#pragma once

#include <Urho3D/Urho3DAll.h>

class NewStructure : public Application
{
    URHO3D_OBJECT(NewStructure, Application);

public:
    explicit NewStructure(Context* context);
    void Start() override;

private:
    SharedPtr<Scene> scene_;
};
