#include <Urho3D/Urho3DAll.h>
#include "WindowManager.h"
#include "../MyEvents.h"

#include "Settings/SettingsWindow.h"
#include "Scoreboard/ScoreboardWindow.h"
#include "Pause/PauseWindow.h"
#include "WeaponChoice/WeaponChoiceWindow.h"
#include "Console/ConsoleWindow.h"
#include "QuitConfirmation/QuitConfirmationWindow.h"
#include "NewGameSettings/NewGameSettingsWindow.h"

/// Construct.
WindowManager::WindowManager(Context* context) :
    Object(context),
    _consoleVisible(false)
{
    SubscribeToEvents();
    RegisterAllFactories();

    _persistentWindows["ConsoleWindow"] = true;

    for (auto it = _persistentWindows.Begin(); it != _persistentWindows.End(); ++it) {
        StringHash type = StringHash((*it).first_);
        SharedPtr<Object> newWindow;
        newWindow = context_->CreateObject(type);

        BaseWindow* window = newWindow->Cast<BaseWindow>();
        window->SetActive(false);

        _windowList.Push(newWindow);
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
    context_->RegisterFactory<QuitConfirmationWindow>();
    context_->RegisterFactory<NewGameSettingsWindow>();
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
                _windowList.Erase(it);
            } else {
                URHO3D_LOGERROR("Window '" + windowName + "' already opened!");
                BaseWindow* window = (*it)->Cast<BaseWindow>();
                window->SetActive(true);
                if (windowName == "ConsoleWindow") {
                    _consoleVisible = true;
                }
                return;
            }
        }
    }

    URHO3D_LOGINFO("Opening window: " + windowName);
    SharedPtr<Object> newWindow;
    newWindow = context_->CreateObject(StringHash(windowName));
    BaseWindow* window = newWindow->Cast<BaseWindow>();
    window->SetActive(true);
    _windowList.Push(newWindow);

    _openedWindows.Push(windowName);
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
        /*using namespace MyEvents::WindowClosed;
        VariantMap data;
        data[P_NAME] = (*it);
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);*/
        /*if (!(*it)) {
            _windowList.Erase(it);
            continue;
        }
        StringHash type = (*it)->GetType();
        bool persistent = false;
        for (auto it2 = _persistentWindows.Begin(); it2 != _persistentWindows.End(); ++it2) {
            if (StringHash((*it2).first_) == type) {
                persistent = true;
            }
        }
        if (!persistent) {
            _windowList.Erase(it);
        }*/
    }

    _openedWindows.Clear();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(WindowManager, HandleUpdate));
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

bool WindowManager::IsConsoleVisible()
{
    return _consoleVisible;
}

void WindowManager::CloseWindow(String windowName)
{
    using namespace MyEvents::CloseWindow;
    URHO3D_LOGINFO("Closing window: " + windowName);
    for (auto it = _windowList.Begin(); it != _windowList.End(); ++it) {
        if ((*it)->GetType() == StringHash(windowName)) {

            if (_persistentWindows.Contains(windowName)) {
                URHO3D_LOGINFO("Cannot destroy persistent window " + windowName);
                BaseWindow* window = (*it)->Cast<BaseWindow>();
                window->SetActive(false);

                if (windowName == "ConsoleWindow") {
                    _consoleVisible = false;
                    VariantMap data;
                    data[P_NAME] = windowName;
                    SendEvent(MyEvents::E_WINDOW_CLOSED, data);
                }
                return;
            }

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