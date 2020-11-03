#pragma once
#include <Urho3D/Core/Object.h>

namespace ConsoleHandlerEvents {
    // Add new console command
    URHO3D_EVENT(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd)
    {
        URHO3D_PARAM(P_NAME, ConsoleCommandName); // string - command name
        URHO3D_PARAM(P_EVENT, ConsoleCommandEvent); // string - event to call when command entered
        URHO3D_PARAM(P_DESCRIPTION, ConsoleCommandDescription); // string - short description of the command
        URHO3D_PARAM(P_OVERWRITE, Overwrite); // bool - should we overwrite existing handler
    }

    // When global variable is changed
    URHO3D_EVENT(E_CONSOLE_GLOBAL_VARIABLE_CHANGED, ConsoleGlobalVariableChanged)
    {
        URHO3D_PARAM(P_NAME, Name);  // string - global variable name
        URHO3D_PARAM(P_VALUE, Value); // string - new value
    }
}