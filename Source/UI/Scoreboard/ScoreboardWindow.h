#pragma once
#include <Urho3D/UI/Window.h>
#include "../BaseWindow.h"

class ScoreboardWindow : public BaseWindow
{
    URHO3D_OBJECT(ScoreboardWindow, BaseWindow);

public:
    ScoreboardWindow(Context* context);

    virtual ~ScoreboardWindow();

    virtual void Init() override;

protected:

    virtual void Create() override;

private:

    void SubscribeToEvents();

    void HandleScoresUpdated(StringHash eventType, VariantMap& eventData);

    void CreatePlayerScores();

    SharedPtr<Window> baseWindow_;
};
