// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace slr {

struct IUndoCommand {
    virtual ~IUndoCommand() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

class UndoManager {
public:
    void push(std::unique_ptr<IUndoCommand> cmd) {
        _redo.clear();
        _undo.push_back(std::move(cmd));
    }

    void undo() {
        if (_undo.empty()) return;
        auto cmd = std::move(_undo.back());
        _undo.pop_back();
        cmd->undo();
        _redo.push_back(std::move(cmd));
    }

    void redo() {
        if (_redo.empty()) return;
        auto cmd = std::move(_redo.back());
        _redo.pop_back();
        cmd->redo();
        _undo.push_back(std::move(cmd));
    }

private:
    std::vector<std::unique_ptr<IUndoCommand>> _undo;
    std::vector<std::unique_ptr<IUndoCommand>> _redo;
};


class AddClipCommand : public IUndoCommand {
public:
    AddClipCommand(ClipContainer& c, ClipItem* clip)
        : _container(c), _clip(clip) {}

    void redo() override {
        _container.clips.push_back(_clip);
    }

    void undo() override {
        auto& v = _container.clips;
        v.erase(std::remove(v.begin(), v.end(), _clip), v.end());
    }

private:
    ClipContainer& _container;
    ClipItem* _clip;
};

//usage
auto clip = new ClipItem{start, length, file};
auto cmd = std::make_unique<AddClipCommand>(container, clip);
cmd->redo();
undoManager.push(std::move(cmd));


template<typename T>
class SetValueCommand : public IUndoCommand {
public:
    SetValueCommand(T& ref, T newVal)
        : _ref(ref), _old(ref), _new(newVal) {}

    void redo() override { _ref = _new; }
    void undo() override { _ref = _old; }

private:
    T& _ref;
    T _old;
    T _new;
};


}