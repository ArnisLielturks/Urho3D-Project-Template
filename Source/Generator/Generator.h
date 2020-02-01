#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Resource/Image.h>

using namespace Urho3D;

class Generator : public Object
{
    URHO3D_OBJECT(Generator, Object);

public:
    /// Construct.
    Generator(Context* context);

    virtual ~Generator();

    Image* GenerateImage(double frequency, int octaves, int seed);

    void Save();

private:
    /**
     * Subscribe to notification events
     */
    void SubscribeToEvents();

    SharedPtr<Image> _generatedImage;
};