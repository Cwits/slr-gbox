// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/AudioFile.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>  // std::remove, std::fopen

#include "core/primitives/AudioBuffer.h"
#include "core/utility/basicAudioManipulation.h"

const std::string pathToTestFiles = "/home/portablejoe/slr/tests/files/";
const slr::frame_t testFileSize = 64;

bool file_exists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (file) {
        std::fclose(file);
        return true;
    }
    return false;
}

bool delete_file(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

TEST(AudioFile, OpenClose) {
    slr::AudioFile * file = new slr::AudioFile();
    std::string path = pathToTestFiles;

    //test 1 ch
    path.append("test_1ch_float.wav");

    ASSERT_TRUE(file->open(path));
 
    ASSERT_EQ(file->frames(), testFileSize);
    ASSERT_EQ(file->channels(), 1);
    ASSERT_EQ(file->samplerate(), 44100);

    //test data
    slr::frame_t frames = file->frames();
    for(slr::frame_t f=0; f<frames; ++f) {
        const slr::sample_t * data = (*file->getData())[0];
        ASSERT_DOUBLE_EQ(data[f], 0.1f);
    }
    
    ASSERT_TRUE(file->close());

    //test 2 ch
    path = pathToTestFiles;
    path.append("test_2ch_float.wav");

    ASSERT_TRUE(file->open(path));
    ASSERT_EQ(file->frames(), testFileSize);
    ASSERT_EQ(file->channels(), 2);
    ASSERT_EQ(file->samplerate(), 44100);

    frames = file->frames();
    for(slr::frame_t f=0; f<frames; ++f) {
        const slr::sample_t * data1 = (*file->getData())[0];
        const slr::sample_t * data2 = (*file->getData())[1];
        ASSERT_DOUBLE_EQ(data1[f], 0.1f);
        ASSERT_DOUBLE_EQ(data2[f], 0.2f);
    }

    ASSERT_TRUE(file->close());

    //if next tests failed to success than, most probably, audio file started to support multichannel:)
    path = pathToTestFiles;
    path.append("test_3ch_float.wav");

    ASSERT_FALSE(file->open(path));

    path = pathToTestFiles;
    path.append("test_4ch_float.wav");

    ASSERT_FALSE(file->open(path));

    delete file;
}

//creates temporary file, write some data to it, save and than reopen and try to read
TEST(AudioFile, Temporary1ch) {
    const slr::frame_t BUFFERSIZE = 64;
    slr::AudioFile * f = new slr::AudioFile();
    std::string path = pathToTestFiles;
    path.append("tmp_test_1ch.wav");

    if(file_exists(path)) {
        delete_file(path);
    }

    f->createTemporary(path, 1, 44100);

    slr::sample_t ** datab = new slr::sample_t*[1];
    slr::sample_t * datach1 = new slr::sample_t[BUFFERSIZE];
    slr::clearAudioBuffer(datach1, BUFFERSIZE);

    for(slr::frame_t f=0; f<BUFFERSIZE; ++f) {
        slr::sample_t data = (float)f/(float)BUFFERSIZE;
        datach1[f] = data;
    }

    datab[0] = datach1;

    slr::AudioBuffer * rec = new slr::AudioBuffer(datab, 1, BUFFERSIZE);
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

    //reopen
    slr::AudioFile * file = new slr::AudioFile();
    //verify
    ASSERT_TRUE(file->open(path));
    ASSERT_EQ(file->frames(), BUFFERSIZE*4);
    ASSERT_EQ(file->channels(), 1);
    ASSERT_EQ(file->samplerate(), 44100);

    slr::frame_t frames = file->frames();
    for(slr::frame_t f=0; f<frames; ++f) {
        const slr::sample_t * data1 = (*file->getData())[0];
        // const slr::sample_t * data2 = (*f->getData())[1];
        ASSERT_DOUBLE_EQ(data1[f], datach1[f%BUFFERSIZE]);
        // ASSERT_DOUBLE_EQ(data2[f], 0.2f);
    }

    ASSERT_TRUE(file->close());

    delete [] datach1;
    delete file;
}

TEST(AudioFile, Temporary2ch) {
    const slr::frame_t BUFFERSIZE = 64;
    slr::AudioFile * f = new slr::AudioFile();
    std::string path = pathToTestFiles;
    path.append("tmp_test_2ch.wav");

    if(file_exists(path)) {
        delete_file(path);
    }

    f->createTemporary(path, 2, 44100);

    slr::sample_t ** datab = new slr::sample_t*[2];
    slr::sample_t * datach1 = new slr::sample_t[BUFFERSIZE];
    slr::sample_t * datach2 = new slr::sample_t[BUFFERSIZE];
    
    slr::clearAudioBuffer(datach1, BUFFERSIZE);
    slr::clearAudioBuffer(datach2, BUFFERSIZE);

    for(slr::frame_t f=0; f<BUFFERSIZE; ++f) {
        slr::sample_t data = (float)f/(float)BUFFERSIZE;
        datach1[f] = data;
        slr::sample_t data2 = 1.f - ((float)f/(float)BUFFERSIZE);
        datach2[f] = data2;
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

    f->finishAfterRecord();
    delete [] datab;
    delete rec;
    f->close();
    delete f;
    f = nullptr;

    //reopen
    slr::AudioFile * file = new slr::AudioFile();
    //verify
    ASSERT_TRUE(file->open(path));
    ASSERT_EQ(file->frames(), BUFFERSIZE*8);
    ASSERT_EQ(file->channels(), 2);
    ASSERT_EQ(file->samplerate(), 44100);

    slr::frame_t frames = file->frames();
    for(slr::frame_t f=0; f<frames; ++f) {
        const slr::sample_t * data1 = (*file->getData())[0];
        const slr::sample_t * data2 = (*file->getData())[1];
        ASSERT_DOUBLE_EQ(data1[f], datach1[f%BUFFERSIZE]);
        ASSERT_DOUBLE_EQ(data2[f], datach2[f%BUFFERSIZE]);
    }

    ASSERT_TRUE(file->close());

    delete [] datach1;
    delete [] datach2;
    delete file;
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}