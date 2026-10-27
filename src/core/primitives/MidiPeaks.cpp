// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/MidiPeaks.h"
#include "core/primitives/MidiEvent.h"
#include "core/primitives/MidiFile.h"

namespace slr {

void MidiPeaks::buildPeaks(const MidiTrack *trk, std::size_t trackNo) {
    // std::vector<Note> notes;

    frame_t end = trk->midiEvents.back().offset;

    std::array<std::vector<size_t>, 128> activeNotes;

    for(const auto &ev : trk->midiEvents) {
        switch(ev.type) {
            case(MidiEventType::NoteOn): {
                if(ev.velocity != 0) {
                    Note n;
                    n.ppqnStart = ev.offset;
                    n.ppqnEnd = end;
                    n.velocity = ev.velocity;
                    n.note = ev.note;

                    _notes.push_back(n);
                    activeNotes[ev.note].push_back(_notes.size()-1);
                } else {
                    //count as note off
                    auto &active = activeNotes[ev.note];
                    if(!active.empty()) {
                        const size_t index = active.front();
                        active.erase(active.begin());

                        _notes[index].ppqnEnd = ev.offset;
                    }
                }
            } break;
            case(MidiEventType::NoteOff): {
                auto &active = activeNotes[ev.note];
                if(!active.empty()) {
                    const size_t index = active.front();
                    active.erase(active.begin());
                
                    _notes[index].ppqnEnd = ev.offset;
                }
            } break;
            case(MidiEventType::CC): {

            } break;
            default:
            break;
        }
    }

    _trackNo = trackNo;
}

}