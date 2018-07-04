#include <Urho3D/Urho3DAll.h>
#include "WindowManager.h"
#include "../MyEvents.h"

#include "Settings/SettingsWindow.h"
#include "Scoreboard/ScoreboardWindow.h"

/// Construct.
WindowManager::WindowManager(Context* context) :
    Object(context)
{
    SubscribeToEvents();
    RegisterAllFactories();
}

WindowManager::~WindowManager()
{
}

void WindowManager::RegisterAllFactories()
{
    // Register classes
    context_->RegisterFactory<SettingsWindow>();
    context_->RegisterFactory<ScoreboardWindow>();
}

void WindowManager::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_OPEN_WINDOW, URHO3D_HANDLER(WindowManager, HandleOpenWindow));
    SubscribeToEvent(MyEvents::E_CLOSE_WINDOW, URHO3D_HANDLER(WindowManager, HandleCloseWindow));
}

void WindowManager::Dispose()
{
}

void WindowManager::HandleOpenWindow(StringHash eventType, VariantMap& eventData)
{
    String windowName = eventData["Name"].GetString();
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {
            if (!(*it).Refs()) {
                _windowList.Remove(*it);
            } else {
                URHO3D_LOGERROR("Window '" + windowName + "' already opened!");
                return;
            }
        }
    }

    URHO3D_LOGINFO("Opening window: " + windowName);
    SharedPtr<Object> newWindow;
    newWindow = context_->CreateObject(StringHash(windowName));
    _windowList.Push(newWindow);
}

void WindowManager::HandleCloseWindow(StringHash eventType, VariantMap& eventData)
{
    String windowName = eventData["Name"].GetString();
    URHO3D_LOGINFO("Closing window: " + windowName);
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {
            _windowList.Remove(*it);
            return;
        }
    }
}