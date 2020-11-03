#pragma once

#include <Urho3D/Urho3DAll.h>

class ProjectTemplate : public Application
{
    URHO3D_OBJECT(ProjectTemplate, Application);

public:
    explicit ProjectTemplate(Context* context);
    void Start() override;

private:
    SharedPtr<Scene> scene_;
};
