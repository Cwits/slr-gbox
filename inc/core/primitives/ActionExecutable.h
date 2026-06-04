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

struct Undoable;

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

	friend class Undoable;
};

/*
	To make action undoable - 
		1. inherit public Undoable
		2. in executin stage and waiting use _direction, whete
			_direction == Forward means normal and redo behaviour(what action should do)
			_direction == Backward means what it should revert

	Check Project::CreateNewUnit and Project::DeleteUnit actions for example
*/
struct Undoable {
	virtual ~Undoable() = default;
	void undo(ControlContext &ctx, ActionExecutable *self) {
		self->_step = 1;
		self->_direction = ActionDirection::Backward;
		self->setState(ActionState::Executing);
	}
	void redo(ControlContext &ctx, ActionExecutable *self) {
		self->_step = 1;
		self->_direction = ActionDirection::Forward;
		self->setState(ActionState::Executing);
	}
};

}