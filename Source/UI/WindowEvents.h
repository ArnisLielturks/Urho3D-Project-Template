#pragma once
#include <Urho3D/Core/Object.h>

namespace WindowEvents {
    // Open UI Window
    URHO3D_EVENT(E_OPEN_WINDOW, OpenWindow)
    {
        URHO3D_PARAM(P_NAME, Name); // string - window object name
        URHO3D_PARAM(P_CLOSE_PREVIOUS, ClosePrevious); // bool - Close window first if it was already opened
    }
    // Close UI Window
    URHO3D_EVENT(E_CLOSE_WINDOW, CloseWindow)
    {
        URHO3D_PARAM(P_NAME, Name); // string - window object name
    }
    // When specific UI window is closed
    URHO3D_EVENT(E_WINDOW_CLOSED, WindowClosed)
    {
        URHO3D_PARAM(P_NAME, Name); // string - close window object name
    }
    // Close all active UI Windows
    URHO3D_EVENT(E_CLOSE_ALL_WINDOWS, CloseAllWindows)
    {
    }
}