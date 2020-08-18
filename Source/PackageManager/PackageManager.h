#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Network/HttpRequest.h>

using namespace Urho3D;

class PackageManager : public Object
{
    URHO3D_OBJECT(PackageManager, Object);

public:
    PackageManager(Context* context);
    virtual ~PackageManager();
    static void RegisterObject(Context* context);

    void Init();

private:
    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    SharedPtr<HttpRequest> httpRequest_;
    String data_;
};
