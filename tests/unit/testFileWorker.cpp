// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/FileWorker.h"
#include "core/primitives/Task.h"
#include "core/primitives/AudioFile.h"
#include "core/primitives/AudioBuffer.h"
#include "testhelper.h"
#include <gtest/gtest.h>
#include <iostream>
#include <future>
#include <string>
#include <math.h>

struct testTask : public slr::Task {
    void exec(slr::FileWorker *fw) {
        for(int i=0; i<100; ++i) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        _success.set_value(true);
    }

    std::promise<bool> _success;
};

struct openTest : public slr::Task {
    void exec(slr::FileWorker *fw) {
        std::unique_ptr<slr::AudioFile> file = std::make_unique<slr::AudioFile>();

        if(file->open(path)) { 
            slr::AudioFile *f = file.get();
            fw->appendFile(std::move(file));

            _file.set_value(f);
        }
    }

    std::string path;
    std::promise<slr::AudioFile*> _file;
};

struct closeTest : public slr::Task {
    void exec(slr::FileWorker *fw) {
        _success.set_value(fw->removeFile(file));
    }

    slr::AudioFile * file;
    std::promise<bool> _success;
};

struct finalizeRecordFile : public slr::Task {
    void exec(slr::FileWorker *fw) {
        //assume that last buffer is written to file
        std::string path = file->path();
        file->finishAfterRecord();
        file->close();

        //how to check this one for be valid???
        fw->releaseTmpAudioFile(file);

        std::unique_ptr<slr::AudioFile> newFile = std::make_unique<slr::AudioFile>();
        if(newFile->open(path)) {
            slr::AudioFile * f = newFile.get();
            fw->appendFile(std::move(newFile));

            _success.set_value(f);
        } else {
            _success.set_value(nullptr);
        }
        /* 
            ofc logic must be more complex than this:
            e.g. when reopened -> emit event that buffer ready to use again in rt engine
        */
    }

    slr::AudioFile * file;
    //int targetId; // to point for track that using file
    std::promise<slr::AudioFile*> _success;
};

struct dumpBuffer : public slr::Task {
    void exec(slr::FileWorker *fw) {
        if(file->dumpRecordedData(buffer)) _done.set_value(true);
        else _done.set_value(false);
        /* 
            ofc logic must be more complex than this:
            e.g. when buffer dumped -> emit event that buffer ready to use again in rt engine
        */
    }

    slr::AudioFile * file;
    slr::AudioBuffer * buffer;
    std::promise<bool> _done;
};

TEST(FileWorker, InitAndShutdown) {
    using namespace slr;
    FileWorker * worker = new FileWorker();
    ASSERT_TRUE(worker->init());
    ASSERT_TRUE(worker->shutdown());
    delete worker;
}

TEST(FileWorker, SimpleTask) {
    using namespace slr;

    FileWorker * worker = new FileWorker();

    worker->init();

    auto task = std::make_unique<testTask>();
    std::future<bool> result = task->_success.get_future();
    
    worker->addTask(std::move(task));

    ASSERT_TRUE(result.get());

    worker->shutdown();

    delete worker;
}

TEST(FileWorker, OpenAndClose) {
    using namespace slr;

    FileWorker * worker = new FileWorker();
    worker->init();

    auto taskOpen = std::make_unique<openTest>();
    taskOpen->path = pathToTestFolder;
    taskOpen->path.append("test_1ch_float.wav");
    std::future<AudioFile*> result = taskOpen->_file.get_future();
    
    worker->addTask(std::move(taskOpen));

    AudioFile * file = result.get();
    ASSERT_NE(file, nullptr);

    //check file
    ASSERT_EQ(file->frames(), 64);
    ASSERT_EQ(file->channels(), 1);
    ASSERT_EQ(file->samplerate(), 44100);

    slr::frame_t frames = file->frames();
    for(slr::frame_t f=0; f<frames; ++f) {
        const slr::sample_t * data = (*file->getData())[0];
        ASSERT_DOUBLE_EQ(data[f], 0.1f);
    }

    const std::vector<File*> list = worker->listFiles();

    ASSERT_EQ(list.size(), static_cast<std::size_t>(1));
    ASSERT_EQ(list.at(0), file);

    auto taskClose = std::make_unique<closeTest>();
    taskClose->file = file;
    std::future<bool> res = taskClose->_success.get_future();
    worker->addTask(std::move(taskClose));


    ASSERT_TRUE(res.get());

    const std::vector<File*> list2 = worker->listFiles();

    ASSERT_EQ(list2.size(), static_cast<std::size_t>(0));

    worker->shutdown();
    delete worker;
}


TEST(FileWorker, OpenAndCloseMulti) {
    using namespace slr;

    FileWorker * worker = new FileWorker();
    worker->init();

    //1st
    auto taskOpen1 = std::make_unique<openTest>();
    taskOpen1->path = pathToTestFolder;
    taskOpen1->path.append("test_1ch_float.wav");
    std::future<AudioFile*> result1 = taskOpen1->_file.get_future();
    worker->addTask(std::move(taskOpen1));

    //2nd
    auto taskOpen2 = std::make_unique<openTest>();
    taskOpen2->path = pathToTestFolder;
    taskOpen2->path.append("test_2ch_float.wav");
    std::future<AudioFile*> result2 = taskOpen2->_file.get_future();
    worker->addTask(std::move(taskOpen2));

    //3rd
    auto taskOpen3 = std::make_unique<openTest>();
    taskOpen3->path = pathToTestFolder;
    taskOpen3->path.append("test_1ch_float.wav");
    std::future<AudioFile*> result3 = taskOpen3->_file.get_future();
    worker->addTask(std::move(taskOpen3));

    //4th
    auto taskOpen4 = std::make_unique<openTest>();
    taskOpen4->path = pathToTestFolder;
    taskOpen4->path.append("test_2ch_float.wav");
    std::future<AudioFile*> result4 = taskOpen4->_file.get_future();
    worker->addTask(std::move(taskOpen4));

    AudioFile * res1 = result1.get();
    AudioFile * res2 = result2.get();
    AudioFile * res3 = result3.get();
    AudioFile * res4 = result4.get();

    ASSERT_NE(res1, nullptr);
    ASSERT_NE(res2, nullptr);
    ASSERT_NE(res3, nullptr);
    ASSERT_NE(res4, nullptr);

    const std::vector<File*> list = worker->listFiles();

    ASSERT_EQ(list.size(), static_cast<std::size_t>(4));
    ASSERT_EQ(list.at(0), res1);
    ASSERT_EQ(list.at(1), res2);
    ASSERT_EQ(list.at(2), res3);
    ASSERT_EQ(list.at(3), res4);

    auto taskClose = std::make_unique<closeTest>();
    taskClose->file = res2;
    std::future<bool> res = taskClose->_success.get_future();
    worker->addTask(std::move(taskClose));


    ASSERT_TRUE(res.get());

    const std::vector<File*> list2 = worker->listFiles();

    ASSERT_EQ(list2.size(), static_cast<std::size_t>(3));
    
    ASSERT_EQ(list2.at(0), res1);
    ASSERT_EQ(list2.at(1), res3);
    ASSERT_EQ(list2.at(2), res4);

    worker->shutdown();
    delete worker;
}

TEST(FileWorker, TemporaryAudioFileWork) {
    using namespace slr;
    FileWorker * worker = new FileWorker();
    worker->init();

    //acquire and release
    AudioFile * record = worker->acquireTmpAudioFile();
    std::string randomName = generateRandomName(8);
    randomName.append(".wav");
    std::string path = pathToTestFolder;
    path.append(randomName);
    record->createTemporary(path, 2, 44100);

    ASSERT_EQ(record->channels(), 2);
    ASSERT_EQ(record->samplerate(), 44100);
    ASSERT_STREQ(record->name().c_str(), randomName.c_str());
    
    record->close();
    worker->releaseTmpAudioFile(record);
    std::remove(path.c_str());
    // record = nullptr;
    randomName.clear();
    path.clear();

    //acquire, "fake use", close, substitute(closeTestTemporary)
    randomName = generateRandomName(8);
    randomName.append(".wav");
    path = pathToTestFolder;
    path.append(randomName);

    AudioFile * record2 = worker->acquireTmpAudioFile();
    
    ASSERT_EQ(record2, record);
    
    sample_t * testb1 = new sample_t[1024];
    sample_t * testb2 = new sample_t[1024];

    for(frame_t i=0; i<1024; ++i) {
        sample_t res = sin(((float)i/(float)1024)*2*M_PI);
        testb1[i] = res;
        testb2[i] = -res;
    }

    record2->createTemporary(path, 2, 44100);

    AudioBuffer * test = new AudioBuffer(1024, 2);
    sample_t * ptr1 = (*test)[0];
    sample_t * ptr2 = (*test)[1];
    for(frame_t f=0; f<test->bufferSize(); ++f) {
        ptr1[f] = testb1[f];
        ptr2[f] = testb2[f];
    }

    for(int i=0; i<10; ++i) {
        std::future<bool> waitfuture;
        auto dumpTask = std::make_unique<dumpBuffer>();
        dumpTask->buffer = test;
        dumpTask->file = record2;
        waitfuture = dumpTask->_done.get_future();
        worker->addTask(std::move(dumpTask));
        ASSERT_TRUE(waitfuture.get());
    }

    delete test;
    //in file now should be 10*1024 frames for 2 channels
    //now close file and reopen it...

    auto finalizeRec = std::make_unique<finalizeRecordFile>();
    finalizeRec->file = record2;
    std::future<AudioFile*> opened = finalizeRec->_success.get_future();
    worker->addTask(std::move(finalizeRec));

    AudioFile * readyToUseFile = opened.get();

    ASSERT_NE(readyToUseFile, nullptr);
    ASSERT_EQ(readyToUseFile->frames(), 10*1024);
    ASSERT_EQ(readyToUseFile->channels(), 2);
    ASSERT_STREQ(readyToUseFile->path().c_str(), path.c_str());
    ASSERT_STREQ(readyToUseFile->name().c_str(), randomName.c_str());

    //compare buffers
    const AudioBuffer * fileData = readyToUseFile->getData();
    const sample_t * filePtr = (*fileData)[0];
    const sample_t * filePtr2 = (*fileData)[1];

    frame_t samples = readyToUseFile->frames();
    for(frame_t f=0; f<samples; ++f) {
        ASSERT_DOUBLE_EQ(filePtr[f], testb1[f%1024]);
        ASSERT_DOUBLE_EQ(filePtr2[f], testb2[f%1024]);
    }

    //??? that's all???

    delete [] testb1;
    delete [] testb2;

    //last checks
    AudioFile * testFile = worker->acquireTmpAudioFile();
    ASSERT_EQ(testFile, record);//still should be same one
    ASSERT_FALSE(testFile->opened());
    ASSERT_FALSE(testFile->temporary());
    ASSERT_EQ(testFile->frames(), 0);
    ASSERT_EQ(testFile->channels(), 0);
    ASSERT_EQ(testFile->samplerate(), 0);
    ASSERT_EQ(testFile->sections(), 0);
    ASSERT_EQ(testFile->format(), 0);
    ASSERT_EQ(testFile->seekable(), 0);
    ASSERT_EQ(testFile->getData(), nullptr);
    ASSERT_TRUE(testFile->path().empty());
    ASSERT_TRUE(testFile->name().empty());

    worker->shutdown();
    delete worker;
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    srand(time(NULL));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}