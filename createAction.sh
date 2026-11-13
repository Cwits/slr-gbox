#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 /path/to/folder ClassName"
    exit 1
fi

DIR="$1"
NAME="$2"

mkdir -p "$DIR"

cat > "$DIR/$NAME.h" << EOF
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "core/actions/ActionExecutable.h"
#include "core/actions/Actions.h"
#include "core/primitives/RtTask.h"

#include <memory>
#include <atomic>

namespace slr {

struct ActionBase;

struct ${NAME}Action : public ActionExecutable {
    ${NAME}Action(const ActionBase *base);
    ~${NAME}Action();

    void exec(ControlContext &ctx) override;
    void checkWaitingCondition(ControlContext &ctx) override;

    private:
    const Actions::${NAME} _action;

    struct FlatAction : public FlatTask {
        void execRT();

        <variables>
        std::atomic<bool> completed;
    };

    FlatAction _flat;
    RtTask _task;
};

std::unique_ptr<ActionExecutable> create${NAME}Action(const ActionBase*);

}
EOF

cat > "$DIR/$NAME.cpp" << EOF
// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later
#include "core/Actions/Folder/${NAME}.h"

#include "core/actions/ActionBase.h"
#include "core/utility/ControlContext.h"

#include "core/Project.h"

#include "snapshots/ProjectView.h"

#include "common/uiControls.h"

#include "common/logger.h"

#include <cassert>

namespace slr {

${NAME}Action::${NAME}Action(const ActionBase *base) :
    _action( *(static_cast<const Actions::${NAME}*>(base)) )
{

}

${NAME}Action::~${NAME}Action() {

}

void ${NAME}Action::exec(ControlContext &ctx) {
    assert(getState() == ActionState::Executing);

    switch(_step) {
    	case(1): {
            
            _flat.completed.store(false);
            _task = makeRtTask(&_flat);
            
            setState(ActionState::Waiting);
            ctx.EmitRtTask(&_task);
    	} break;
    	case(2): {

            setState(ActionState::Finished);
    	} break;
        default: assert(false && "Unreachable"); break;
    }
}

void ${NAME}Action::checkWaitingCondition(ControlContext &ctx) {
    assert(getState() == ActionState::Waiting);

    switch(_step) {
        case(1): {
            bool res = _flat.completed.load(std::memory_order_acquire);
            if(res) {
                _step = 2;
                setState(ActionState::Executing);
            }
        } break;
        default: assert(false && "Unreachable"); break;
    }
}

void ${NAME}Action::FlatAction::execRT() {

    completed.store(true, std::memory_order_release);
}

std::unique_ptr<ActionExecutable> create${NAME}Action(const ActionBase *base) {
    return std::make_unique<${NAME}Action>(base);
}

}
EOF

echo "Created:"
echo "  $DIR/$NAME.h"
echo "  $DIR/$NAME.cpp"
