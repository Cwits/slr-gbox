// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/utility/basicAudioManipulation.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <cstdlib>  //rand()
#include <ctime>    //time()
#include <cmath>    //RAND_MAX

#define BUFFERSIZE 64

using namespace slr;

void fillGarbage(sample_t * buffer, frame_t frames) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for(frame_t i=0; i<frames; ++i) {
        // rand() to [-1.0f, 1.0f]
        sample_t r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);  // [0.0, 1.0]
        buffer[i] = r * 2.0f - 1.0f; // [0.0, 1.0] -> [-1.0, 1.0]
    }
}

TEST(BasicAudioManipulation, Fill) {
    frame_t size = BUFFERSIZE;
    sample_t * dst = new sample_t[size];

    sample_t t = 0.5f;
    slr::fillAudioBuffer(dst, size, t);
    // bool res = true;

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(0.5f, dst[f]);
        // if(dst[f] != 0.5f) res = false;
    }

    // ASSERT_TRUE(res);

    delete [] dst;
}

TEST(BasicAudioManipulation, Clear) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];

    fillGarbage(src, size);

    slr::clearAudioBuffer(src, size);

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(0.0f, src[f]);
    }

    sample_t * src2 = new sample_t[size];
    fillGarbage(src, size);
    fillGarbage(src2, size);

    slr::clearAudioBuffers(src, src2, size);

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(0.0f, src[f]);
        EXPECT_DOUBLE_EQ(0.0f, src2[f]);
    }

    delete [] src;
    delete [] src2;
}

TEST(BasicAudioManipulation, Sum) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];

    sample_t t = 0.3f;
    sample_t t2 = 0.7f;
    fillAudioBuffer(src, size, t);
    fillAudioBuffer(dst, size, t2);

    slr::sumAudioBuffers(src, dst, size);

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(1.f, dst[f]);
    }

    delete [] src;
    delete [] dst;
}

TEST(BasicAudioManipulation, MulBetween) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];

    sample_t t = 0.3f;
    sample_t t2 = 0.7f;
    fillAudioBuffer(src, size, t);
    fillAudioBuffer(dst, size, t2);

    slr::mulAudioBuffers(src, dst, size);

    sample_t fres = 0.3f * 0.7f;
    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(fres, dst[f]);
    }

    delete [] src;
    delete [] dst;
}

TEST(BasicAudioManipulation, MulToValue) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];

    sample_t t = 0.3f;
    fillAudioBuffer(src, size, t);

    sample_t mul = 0.45f;
    slr::mulAudioBufferToValue(src, size, mul);

    sample_t fres = 0.3f * 0.45f;
    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(fres, src[f]);
    }

    delete [] src;
}

TEST(BasicAudioManipulation, MulTwoToValue) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];

    sample_t t = 0.3f;
    sample_t t2 = 0.7f;
    fillAudioBuffer(src, size, t);
    fillAudioBuffer(dst, size, t2);

    sample_t mul = 0.33f;
    slr::mulAudioBuffersToValue(src, dst, size, mul);

    sample_t fres = 0.3f * 0.33f;
    sample_t fres2 = 0.7f * 0.33f;
    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(fres, src[f]);
        EXPECT_DOUBLE_EQ(fres2, dst[f]);
    }

    delete [] src;
    delete [] dst;
}

TEST(BasicAudioManipulation, Copy) {
    frame_t size = BUFFERSIZE;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];

    sample_t t = 0.3f;
    fillAudioBuffer(src, size, t);
    slr::clearAudioBuffer(dst, size);

    slr::copyAudioBuffer(src, dst, size);

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(0.3f, dst[f]);
    }

    delete [] src;
    delete [] dst;
}

TEST(BasicAudioManipulation, PackStereo) {
    frame_t size = BUFFERSIZE;
    frame_t size2 = size*2;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];
    sample_t * pack = new sample_t[size2];

    sample_t t = 0.3f;
    sample_t t2 = 0.7f;
    fillAudioBuffer(src, size, t);
    fillAudioBuffer(dst, size, t2);
    slr::clearAudioBuffer(pack, size2);

    slr::packTwoMonoToStereo(src, dst, pack, size2);

    for(frame_t f=0; f<size2; f+=2) {
        EXPECT_DOUBLE_EQ(0.3f, pack[f]);
        EXPECT_DOUBLE_EQ(0.7f, pack[f+1]);
    }

    delete [] src;
    delete [] dst;
    delete [] pack;
}

TEST(BasicAudioManipulation, PackMulti) {
    frame_t size = BUFFERSIZE;
    frame_t size2 = size*3;
    sample_t * src1 = new sample_t[size];
    sample_t * src2 = new sample_t[size];
    sample_t * src3 = new sample_t[size];
    sample_t * pack = new sample_t[size2];

    sample_t t = 0.3f;
    sample_t t2 = 0.7f;
    sample_t t3 = 0.5f;
    fillAudioBuffer(src1, size, t);
    fillAudioBuffer(src2, size, t2);
    fillAudioBuffer(src3, size, t3);

    slr::clearAudioBuffer(pack, size2);

    sample_t ** tmp = new sample_t*[3];
    tmp[0] = src1;
    tmp[1] = src2;
    tmp[2] = src3;

    slr::packMulti(tmp, pack, 3, size);

    for(frame_t f=0; f<size2; f+=3) {
        EXPECT_DOUBLE_EQ(0.3f, pack[f]);
        EXPECT_DOUBLE_EQ(0.7f, pack[f+1]);
        EXPECT_DOUBLE_EQ(0.5f, pack[f+2]);
    }

    delete [] tmp;
    delete [] src1;
    delete [] src2;
    delete [] src3;
    delete [] pack;
}

TEST(BasicAudioManipulation, UnpackStereo) {
    frame_t size = BUFFERSIZE;
    frame_t size2 = size*2;
    sample_t * src = new sample_t[size];
    sample_t * dst = new sample_t[size];
    sample_t * pack = new sample_t[size2];

    for(frame_t f=0; f<size2; f+=2) {
        pack[f] = 0.3f;
        pack[f+1] = 0.7f;
    }

    slr::clearAudioBuffer(src, size);
    slr::clearAudioBuffer(dst, size);

    slr::unpackStereo(pack, src, dst, size2);

    for(frame_t f=0; f<size; ++f) {
        EXPECT_DOUBLE_EQ(0.3f, src[f]);
        EXPECT_DOUBLE_EQ(0.7f, dst[f]);
    }

    delete [] src;
    delete [] dst;
    delete [] pack;
}

TEST(BasicAudioManipulation, UnpackMulti) {
    frame_t channels = 4;
    frame_t size = BUFFERSIZE;
    frame_t sizech = size*channels;
    sample_t * src = new sample_t[sizech];
    sample_t ** dst = new sample_t*[channels];

    for(frame_t i=0; i<channels; ++i) {
        dst[i] = new sample_t[size];
        slr::clearAudioBuffer(dst[i], size);
    }

    //fill
    float datach[channels] = {0};
    for(frame_t ch=1; ch<=channels; ++ch) {
        datach[ch-1] = ch/(float)channels;
    }

    for(frame_t f=0; f<sizech; f+=channels) {
        for(frame_t ch=0; ch<channels; ++ch) {
            src[f+ch] = datach[ch];
        }
    }

    int ch = static_cast<int>(channels);
    slr::unpackMulti(src, dst, ch, size);
    
    for(slr::frame_t f=0; f<size; ++f) {
        for(slr::frame_t ch=0; ch<channels; ++ch) {
            EXPECT_DOUBLE_EQ(datach[ch], dst[ch][f]);
        }
    }

    delete [] src;
    for(slr::frame_t i=0; i<channels; ++i) {
        delete [] dst[i];
    }
    delete [] dst;
    // delete [] pack;
}

int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}