#pragma once
#include <Urho3D/Core/Object.h>

namespace NakamaEvents {
    URHO3D_EVENT(E_LOGGED_IN, LoggedIn) {
        URHO3D_PARAM(P_TOKEN, Token);
        URHO3D_PARAM(P_USER_ID, UserID);
        URHO3D_PARAM(P_USERNAME, Username);
        URHO3D_PARAM(P_NEW_USER, NewUser);
    }

    URHO3D_EVENT(E_LOGIN_FAILED, LoginFailed) {
    }
}
