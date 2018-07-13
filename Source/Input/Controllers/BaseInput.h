#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../../Config/ConfigFile.h"

class BaseInput : public Object
{
    URHO3D_OBJECT(BaseInput, Object);

public:
    /// Construct.
    BaseInput(Context* context):
		Object(context),
		_activeAction(0) {
	}

    virtual ~BaseInput() {};

	/**
	 * Get name of the key which uses specific action
	 */
	virtual String GetActionKeyName(int action) { return String::EMPTY; };

	/**
	 * Bind key to specific action
	 */
	void SetKeyToAction(int key, int action) {
		_mappedKeyToControl[key] = action;
		_mappedControlToKey[action] = key;
	}

	bool IsActionUsed(int action) {
		return _mappedControlToKey.Contains(action);
	}

	/**
	 * Get all action mapping against controls
	 */
	HashMap<int, int> GetConfigMap()
	{
		return _mappedControlToKey;
	}

	/**
	 * Start the key to action mapping process
	 */
	void StartMappingAction(int action)
	{
		_activeAction = action;
	}

	/**
	 * Stop the key to action mapping process
	 */
	void StopMappingAction()
	{
		_activeAction = 0;
		_timer.Reset();
	}

	/**
	 * Release all mappings for specific action
	 */
	void ReleaseAction(int action)
	{
		for (auto it = _mappedControlToKey.Begin(); it != _mappedControlToKey.End(); ++it) {
			int keyCode = (*it).second_;
			int actionCode = (*it).first_;
			if (action == actionCode) {
				_mappedControlToKey.Erase(actionCode);
				_mappedKeyToControl.Erase(keyCode);
			}
		}
		for (auto it = _mappedKeyToControl.Begin(); it != _mappedKeyToControl.End(); ++it) {
			int keyCode = (*it).first_;
			int actionCode = (*it).second_;
			if (action == actionCode) {
				_mappedKeyToControl.Erase(keyCode);
				_mappedControlToKey.Erase(actionCode);
			}
		}
	}

	/**
	 * Release all mappings for specific key
	 */
	void ReleaseKey(int key)
	{
		for (auto it = _mappedKeyToControl.Begin(); it != _mappedKeyToControl.End(); ++it) {
			int keyCode = (*it).first_;
			int actionCode = (*it).second_;
			if (key == keyCode) {
				_mappedKeyToControl.Erase(keyCode);
				_mappedControlToKey.Erase(actionCode);
			}
		}
		for (auto it = _mappedControlToKey.Begin(); it != _mappedControlToKey.End(); ++it) {
			int keyCode = (*it).second_;
			int actionCode = (*it).first_;
			if (key == keyCode) {
				_mappedControlToKey.Erase(actionCode);
				_mappedKeyToControl.Erase(keyCode);
			}
		}
	}

protected:
	// Control against key map
	HashMap<int, int> _mappedControlToKey;
	// key against control map
	HashMap<int, int> _mappedKeyToControl;

	int _activeAction;

	Timer _timer;
};