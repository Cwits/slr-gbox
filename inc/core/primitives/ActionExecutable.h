// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <atomic>

namespace slr {

enum class ActionState {
	Executing,
	Waiting,
	Finished
};

struct ControlContext;
struct ActionExecutable {
	ActionExecutable() {
		setState(ActionState::Executing);
    	_step = 1;
		_toDelete = false;
	}
	virtual ~ActionExecutable() {}
	virtual void exec(ControlContext &ctx) = 0;
	virtual void checkWaitingCondition() = 0;
	
	ActionState getState() const {
		return state.load(std::memory_order_acquire);
	}
	
	bool toDelete() const { return _toDelete; }
	
	protected:
	int _step;

	void markDelete() { _toDelete = true; }
	void setState(ActionState s) {
		state.store(s, std::memory_order_release);
	}

	void abortAction() { markDelete(); setState(ActionState::Finished);}
	
	private:
	bool _toDelete;
	std::atomic<ActionState> state;
};

}