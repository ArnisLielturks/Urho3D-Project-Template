#include <Urho3D/Urho3DAll.h>
#include "WindowManager.h"
#include "../MyEvents.h"

#include "Settings/SettingsWindow.h"
#include "Scoreboard/ScoreboardWindow.h"
#include "Pause/PauseWindow.h"
#include "QuitConfirmation/QuitConfirmationWindow.h"
#include "NewGameSettings/NewGameSettingsWindow.h"
#include "Achievements/AchievementsWindow.h"

/// Construct.
WindowManager::WindowManager(Context* context) :
    Object(context)
{
    SubscribeToEvents();
    RegisterAllFactories();
}

WindowManager::~WindowManager()
{
    _windowList.Clear();
}

void WindowManager::RegisterAllFactories()
{
    // Register classes
    context_->RegisterFactory<BaseWindow>();
    context_->RegisterFactory<SettingsWindow>();
    context_->RegisterFactory<ScoreboardWindow>();
    context_->RegisterFactory<AchievementsWindow>();
    context_->RegisterFactory<QuitConfirmationWindow>();
    context_->RegisterFactory<NewGameSettingsWindow>();
    context_->RegisterFactory<AchievementsWindow>();
    context_->RegisterFactory<PauseWindow>();
}

void WindowManager::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_OPEN_WINDOW, URHO3D_HANDLER(WindowManager, HandleOpenWindow));
    SubscribeToEvent(MyEvents::E_CLOSE_WINDOW, URHO3D_HANDLER(WindowManager, HandleCloseWindow));
    SubscribeToEvent(MyEvents::E_CLOSE_ALL_WINDOWS, URHO3D_HANDLER(WindowManager, HandleCloseAllWindows));
}

void WindowManager::HandleOpenWindow(StringHash eventType, VariantMap& eventData)
{
    String windowName = eventData["Name"].GetString();
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {
            if (!(*it).Refs()) {
                _windowList.Erase(it);
            } else {
                URHO3D_LOGERROR("Window '" + windowName + "' already opened!");
                BaseWindow* window = (*it)->Cast<BaseWindow>();
                //TODO bring this window to the front
                return;
            }
        }
    }

    URHO3D_LOGINFO("Opening window: " + windowName);
    SharedPtr<Object> newWindow;
    newWindow = context_->CreateObject(StringHash(windowName));
    if (newWindow) {
        BaseWindow *window = newWindow->Cast<BaseWindow>();
        _windowList.Push(newWindow);

        _openedWindows.Push(windowName);
    } else {
        URHO3D_LOGERROR("Failed to open window: " + windowName);
    }
}

void WindowManager::HandleCloseWindow(StringHash eventType, VariantMap& eventData)
{
    String windowName = eventData["Name"].GetString();
    _closeQueue.Push(windowName);
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(WindowManager, HandleUpdate));
}

void WindowManager::HandleCloseAllWindows(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Closing all windows");
    for (auto it = _openedWindows.Begin(); it != _openedWindows.End(); ++it) {
        _closeQueue.Push((*it));
    }

    _openedWindows.Clear();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(WindowManager, HandleUpdate));
}

bool WindowManager::IsWindowOpen(String windowName)
{
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {
            BaseWindow* window = (*it)->Cast<BaseWindow>();
            if ((*it).Refs()) {
                URHO3D_LOGINFO(" WINDOW " + windowName + " IS ACTIVE");
                return true;
            }
        }
    }

    URHO3D_LOGINFO(" WINDOW " + windowName + " NOT ACTIVE");
    return false;
}

void WindowManager::CloseWindow(String windowName)
{
    using namespace MyEvents::CloseWindow;
    URHO3D_LOGINFO("Closing window: " + windowName);
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {

            _windowList.Erase(it);
            VariantMap data;
            data[P_NAME] = windowName;
            SendEvent(MyEvents::E_WINDOW_CLOSED, data);
            return;
        }
    }
}

void WindowManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    for (auto it = _closeQueue.Begin(); it != _closeQueue.End(); ++it) {
        CloseWindow((*it));
    }
    _closeQueue.Clear();
    UnsubscribeFromEvent(E_UPDATE);
}