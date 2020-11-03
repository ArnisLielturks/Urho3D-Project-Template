#pragma once
#include <Urho3D/Core/Object.h>

namespace AudioEvents {
    // play sound
    URHO3D_EVENT(E_PLAY_SOUND, PlaySound)
    {
        URHO3D_PARAM(P_INDEX, Index); // string or int - sound id
        URHO3D_PARAM(P_TYPE, Type); // string - sound type - Master / Effect / Ambient / Voice / Music
        URHO3D_PARAM(P_SOUND_FILE, SoundFile); // string - full path to sound file, not needed when Index is used
    }

    // stop sound
    URHO3D_EVENT(E_STOP_SOUND, StopSound)
    {
        URHO3D_PARAM(P_INDEX, Index); // int - sound id
        URHO3D_PARAM(P_TYPE, Type); // string - sound type - Master / Effect / Ambient / Voice / Music
    }

    // stop all sounds in progress
    URHO3D_EVENT(E_STOP_ALL_SOUNDS, StopAllSounds)
    {
    }
}