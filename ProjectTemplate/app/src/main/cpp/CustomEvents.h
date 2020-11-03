#pragma once
#include <Urho3D/Core/Object.h>

namespace CustomEvents
{
    // Add new global variable which should be saved in the config files
    URHO3D_EVENT(E_ADD_CONFIG, AddConfig)
    {
        URHO3D_PARAM(P_NAME, Name); // string - global variable name
    }

    // Load configuration file
    URHO3D_EVENT(E_LOAD_CONFIG, LoadConfig)
    {
        URHO3D_PARAM(P_FILEPATH, Filepath); // string - filepath + filename, relative to executable
        URHO3D_PARAM(P_PREFIX, Prefix); // string - prefix, which will be added to loaded configuration variables, can be empty
    }

    // Video settings changed event
    URHO3D_EVENT(E_VIDEO_SETTINGS_CHANGED, VideoSettingsChanged)
    {
    }
}
