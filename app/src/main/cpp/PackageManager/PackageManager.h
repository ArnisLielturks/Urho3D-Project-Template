#ifdef PACKAGE_MANAGER

#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Resource/JSONFile.h>

#if !defined(__EMSCRIPTEN__)
#include <Urho3D/Network/HttpRequest.h>
#endif

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
#if !defined(__EMSCRIPTEN__)
    SharedPtr<HttpRequest> httpRequest_;
#endif
    String data_;
};

#endif
