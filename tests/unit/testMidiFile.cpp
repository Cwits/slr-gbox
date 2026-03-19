// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/MidiFile.h"
#include "testhelper.h"

#include <gtest/gtest.h>
#include <malloc.h>
#include <iostream>
#include <cstring>
#include <string>

//test file - 4 bars, all C4, E4, G4 chord start and end same time.
//1st bar - quater note, 27 velocity; 2nd - half note, 49 velocity; 3rd - 3/4 of bar, 77 velocity; 4th - full bar, 127 velocity
//and four CC 64 events hold pedal, and bunch on CC 20

TEST(MidiFile, OpenFile) {
    using namespace slr;
    
    std::string pathToFile = pathToTestFolder;
    pathToFile.append("midi_test_1.mid"); 
    
    MidiFile * file = new MidiFile();
    
    ASSERT_TRUE(file->open(pathToFile));

    ASSERT_EQ(file->trackCount(), 2);

    const MidiFile::MidiTrack & trk = file->track(1);
    ASSERT_EQ(trk.midiEvents.size(), 64);
    ASSERT_EQ(trk.metaEvents.size(), 1);
    ASSERT_EQ(trk.sysexEvents.size(), 0);

    std::vector<uint8_t> cmaj = {0, 2, 4, 5, 7, 9, 11};
    auto inScale = [cmaj](const uint8_t note) -> bool {
        for(std::size_t i=0; i<cmaj.size(); ++i) {
            if(note == cmaj.at(i)) return true;
        }
        return false;
    };

    for(std::size_t i=0; i<trk.midiEvents.size(); ++i) {
        const MidiEvent &ev = trk.midiEvents.at(i);

        frame_t expectedpos = 960 * i;
        ASSERT_EQ(ev.offset, expectedpos);
        ASSERT_EQ(ev.velocity, (i%2 == 0 ? 96 : 0));
        ASSERT_EQ(ev.type, (i%2 == 0 ? MidiEventType::NoteOn : MidiEventType::NoteOff));
        ASSERT_EQ(ev.channel, 0);
        ASSERT_TRUE(inScale(ev.note % 12));
    }
    //64 events - 32 note on, 32 note off.
    //1 track
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}