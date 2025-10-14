// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>  // std::remove, std::fopen

#include "core/primitives/Buffer.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"
#include "core/primitives/AudioPeakFile.h"
#include "core/primitives/AudioPeaks.h"

#include "testhelper.h"
#include "utility.h"

const slr::frame_t testFileSize = 64;

slr::AudioPeaks::LODLevels levelFromInt(uint8_t lvl) {
    slr::AudioPeaks::LODLevels level;
    switch(lvl) {
        case(0): level = slr::AudioPeaks::LODLevels::LODLevel0; break;
        case(1): level = slr::AudioPeaks::LODLevels::LODLevel1; break;
        case(2): level = slr::AudioPeaks::LODLevels::LODLevel2; break;
        case(3): level = slr::AudioPeaks::LODLevels::LODLevel3; break;
        case(4): level = slr::AudioPeaks::LODLevels::LODLevel4; break;
        case(5): level = slr::AudioPeaks::LODLevels::LODLevel5; break;
        case(6): level = slr::AudioPeaks::LODLevels::LODLevel6; break;
    }
    return level;
}

TEST(AudioPeaks, GenerateTestAudioTarget) {
    const slr::frame_t BUFFERSIZE = 1024;
    std::string path = pathToTestFolder;
    path.append("test_target_no_tail.wav");

    if(file_exists(path)) {
        delete_file(path);
    }

    slr::AudioFile * f = new slr::AudioFile();
    f->createTemporary(path, 2, 44100);

    slr::sample_t ** datab = new slr::sample_t*[2];
    slr::sample_t * datach1 = new slr::sample_t[BUFFERSIZE];
    slr::sample_t * datach2 = new slr::sample_t[BUFFERSIZE];
    
    slr::clearAudioBuffer(datach1, BUFFERSIZE);
    slr::clearAudioBuffer(datach2, BUFFERSIZE);

    for(slr::frame_t f=0; f<BUFFERSIZE; ++f) {
        if(f%2 == 0) {
            datach1[f] = 0.2f;
            datach2[f] = 0.5f;
        } else {
            datach1[f] = 0.4f;
            datach2[f] = 1.f;
        }
    }

    datab[0] = datach1;
    datab[1] = datach2;

    slr::AudioBuffer * rec = new slr::AudioBuffer(datab, 2, BUFFERSIZE);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);
    f->dumpRecordedData(rec);

    f->finishAfterRecord();
    delete [] datab;
    delete rec;
    f->close();
    delete f;
    f = nullptr;
    delete [] datach1;
    delete [] datach2;


    ///generate slrpk
    path = pathToTestFolder;
    path.append("test_target_no_tail.wav");
    f = new slr::AudioFile();
    
    ASSERT_TRUE(f->open(path));

    path = path.substr(0, path.size()-4);
    path.append(".slrpk");
    slr::AudioPeakFile * apk = new slr::AudioPeakFile();

    if(slr::AudioPeakFile::exists(path)) {
        ASSERT_TRUE(apk->open(path));
    } else {
        ASSERT_TRUE(apk->createAndBuild(path, f));
        ASSERT_TRUE(apk->open(path));
    }

    delete apk;
    delete f;
}

TEST(AudioPeaks, TestLODs) {
    using namespace slr;
    std::string path = pathToTestFolder;
    path.append("test_target_no_tail.wav");
    AudioFile * f = new AudioFile();
    
    ASSERT_TRUE(f->open(path));

    path = path.substr(0, path.size()-4);
    path.append(".slrpk");
    AudioPeakFile * apk = new AudioPeakFile();

    if(AudioPeakFile::exists(path)) {
        ASSERT_TRUE(apk->open(path));
    } else {
        ASSERT_TRUE(apk->createAndBuild(path, f));
        ASSERT_TRUE(apk->open(path));
    }

    //file opened... test data:
    AudioPeaks _lod0;
    _lod0.build(f, AudioPeaks::LODLevels::LODLevel0);

    // AudioPeaks * testTarget = apk->data0();

    AudioPeaks::PeakData0 * targetData = apk->data0();
    AudioPeaks::PeakData0 * trueData = _lod0.lod0Data();

    ASSERT_EQ(targetData->bufferSize(), trueData->bufferSize());
    ASSERT_EQ(targetData->channels(), trueData->channels());

    uint8_t ** targetPtr = targetData->data();
    uint8_t ** truePtr = trueData->data();

    frame_t size = targetData->bufferSize();
    int channels = targetData->channels();

    for(frame_t f=0; f<size; ++f) {
        for(int ch=0; ch<channels; ++ch) {
            ASSERT_EQ(targetPtr[ch][f], truePtr[ch][f]);
        }
    }

    std::array<AudioPeaks, 6> _peaks;
    for(int i=0; i<6; ++i) {
        _peaks[i].build(f, levelFromInt(i+1));
    }

    for(int i=0; i<6; ++i) {
        AudioPeaks::PeakData * targetData = apk->data(levelFromInt(i+1));
        AudioPeaks::PeakData * trueData = _peaks[i].data();

        ASSERT_EQ(targetData->bufferSize(), trueData->bufferSize());
        ASSERT_EQ(targetData->channels(), trueData->channels());
        
        AudioPeaks::PeakDataBase ** targetPtr = targetData->data();
        AudioPeaks::PeakDataBase ** truePtr = trueData->data();

        frame_t size = targetData->bufferSize();
        int channels = targetData->channels();

        for(frame_t f=0; f<size; ++f) {
            for(int ch=0; ch<channels; ++ch) {
                ASSERT_EQ(targetPtr[ch][f].min, truePtr[ch][f].min);
                ASSERT_EQ(targetPtr[ch][f].max, truePtr[ch][f].max);
            }
        }
    }


    delete apk;
    delete f;
    delete_file(path);
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}