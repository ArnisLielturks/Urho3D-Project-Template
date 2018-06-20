#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class Message : public Object
{
	URHO3D_OBJECT(Message, Object);

public:
	/// Construct.
	Message(Context* context);

	virtual ~Message();

	void Create();

	void Dispose();

	void HandleOkButton(StringHash eventType, VariantMap& eventData);

	void HandleShowMessage(StringHash eventType, VariantMap& eventData);

protected:
	virtual void Init();

private:

	void SubscribeToEvents();

	UIElement* _baseElement;
	SharedPtr<Button> _okButton;
	SharedPtr<Text> _title;
	SharedPtr<Text> _message;


};