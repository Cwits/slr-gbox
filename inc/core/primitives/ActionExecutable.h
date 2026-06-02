// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>

namespace slr {

struct ControlContext;

enum class ActionState {
	Abort,
	Executing,
	Waiting,
	Finished
};

enum class ActionDirection {
	Forward,
	Backward
};


struct Undoable {
	virtual ~Undoable() = default;
	virtual void undo(ControlContext &ctx) = 0;
	virtual void redo(ControlContext &ctx) = 0;
};

struct ActionExecutable {
	ActionExecutable() {
		_direction = ActionDirection::Forward;
    	_step = 1;
		setState(ActionState::Executing);
	}
	virtual ~ActionExecutable() {}
	virtual void exec(ControlContext &ctx) = 0;
	virtual void checkWaitingCondition(ControlContext &ctx) = 0;
	
	ActionState getState() const {
		return state.load(std::memory_order_acquire);
	}
	
	ActionDirection direction() const { return _direction; }

	protected:
	int _step;
	ActionDirection _direction;

	void setState(ActionState s) {
		state.store(s, std::memory_order_release);
	}

	void abortAction() { setState(ActionState::Abort);}
	
	private:
	std::atomic<ActionState> state;
};

}