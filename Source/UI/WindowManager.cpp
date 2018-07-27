#include <Urho3D/Urho3DAll.h>
#include "WindowManager.h"
#include "../MyEvents.h"

#include "Settings/SettingsWindow.h"
#include "Scoreboard/ScoreboardWindow.h"
#include "Pause/PauseWindow.h"
#include "WeaponChoice/WeaponChoiceWindow.h"
#include "Console/ConsoleWindow.h"

/// Construct.
WindowManager::WindowManager(Context* context) :
    Object(context)
{
    SubscribeToEvents();
    RegisterAllFactories();

    _persistentWindows["ConsoleWindow"] = true;

    for (auto it = _persistentWindows.Begin(); it != _persistentWindows.End(); ++it) {
        StringHash type = StringHash((*it).first_);
        SharedPtr<Object> newWindow;
        newWindow = context_->CreateObject(type);
        _windowList.Push(newWindow);

        BaseWindow* window = _windowList.At(_windowList.Size() - 1)->Cast<BaseWindow>();
        window->SetActive(false);
    }
}

WindowManager::~WindowManager()
{
    _windowList.Clear();
}

void WindowManager::RegisterAllFactories()
{
    // Register classes
    context_->RegisterFactory<SettingsWindow>();
    context_->RegisterFactory<ScoreboardWindow>();
    context_->RegisterFactory<PauseWindow>();
    context_->RegisterFactory<WeaponChoiceWindow>();
    context_->RegisterFactory<ConsoleWindow>();
}

void WindowManager::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_OPEN_WINDOW, URHO3D_HANDLER(WindowManager, HandleOpenWindow));
    SubscribeToEvent(MyEvents::E_CLOSE_WINDOW, URHO3D_HANDLER(WindowManager, HandleCloseWindow));
    SubscribeToEvent(MyEvents::E_CLOSE_ALL_WINDOWS, URHO3D_HANDLER(WindowManager, HandleCloseAllWindows));
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
                BaseWindow* window = (*it)->Cast<BaseWindow>();
                window->SetActive(true);
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

            if (_persistentWindows.Contains(windowName)) {
                URHO3D_LOGINFO("Cannot destroy persistent window " + windowName);
                BaseWindow* window = (*it)->Cast<BaseWindow>();
                window->SetActive(false);
                return;
            }

            _windowList.Remove(*it);
            SendEvent(MyEvents::E_WINDOW_CLOSED, eventData);
            return;
        }
    }
}

void WindowManager::HandleCloseAllWindows(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Closing all windows");
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        StringHash type = (*it)->GetType();
        bool persistent = false;
        for (auto it2 = _persistentWindows.Begin(); it2 != _persistentWindows.End(); ++it2) {
            if (StringHash((*it2).first_) == type) {
                persistent = true;
            }
        }
        if (!persistent) {
            _windowList.Erase(it);
        }
    }
}

bool WindowManager::IsWindowOpen(String windowName)
{
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {
            BaseWindow* window = (*it)->Cast<BaseWindow>();
            if ((*it).Refs() && window->IsActive()) {
                URHO3D_LOGINFO(" WINDOW " + windowName + " IS ACTIVE");
                return true;
            }
        }
    }

    URHO3D_LOGINFO(" WINDOW " + windowName + " NOT ACTIVE");
    return false;
}