// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/utility/basicAudioManipulation.h"
#include "core/primitives/AudioBuffer.h"

#include <cstring>

namespace slr {

//for frames dst[i] += src[i];
void sumAudioBuffers(const sample_t * src, sample_t * dst, const frame_t frames) {
    for(frame_t i=0; i<frames; ++i) {
        dst[i] += src[i];
    }
}

//for frames dst[i] *= src[i];
void mulAudioBuffers(sample_t * src, sample_t * dst, const frame_t frames) {
    for(frame_t i=0; i<frames; ++i) {
        dst[i] *= src[i];
    }
}

//for frames src[i] *= value;
void mulAudioBufferToValue(sample_t * src, const frame_t frames, const sample_t value) {
    for(frame_t i=0; i<frames; ++i) {
        src[i] *= value;
    }
}

//for frames src[i] *= value;
//           src2[i] *= value;
void mulAudioBuffersToValue(sample_t * src, sample_t * src2, const frame_t frames, const sample_t value) {
    for(frame_t i=0; i<frames; ++i) {
        src[i] *= value;
        src2[i] *= value;
    }
}

//for frames dst[i] = src[i]
void copyAudioBuffer(sample_t * src, sample_t * dst, const frame_t frames) {
    for(frame_t i=0; i<frames; ++i) {
        dst[i] = src[i];
    }
}

//std::memset(src, 0.f, sizeof(sample_t) * frames);
void clearAudioBuffer(sample_t * src, const frame_t frames) {
    // for(frame_t i=0; i<frames; ++i) {
    //     src[i] = 0.0f;
    // }
    std::memset(src, 0.f, sizeof(sample_t) * frames);
}

// std::memset(src1, 0.f, sizeof(sample_t) * frames);
// std::memset(src2, 0.f, sizeof(sample_t) * frames);
void clearAudioBuffers(sample_t * src1, sample_t * src2, const frame_t frames) {
    // for(frame_t i=0; i<frames; ++i) {
    //     src1[i] = 0.0f;
    //     src2[i] = 0.0f;
    // }
    std::memset(src1, 0.f, sizeof(sample_t) * frames);
    std::memset(src2, 0.f, sizeof(sample_t) * frames);
}

//for frames dst[f] = srcL[f];
//           dst[f+1] = srcR[f];
// frames for dst size - src size * 2
void packTwoMonoToStereo(sample_t * srcL, sample_t * srcR, sample_t * dst, const frame_t frames) {
    for(frame_t f=0; f<frames; f+=2) {
        frame_t srcFrame = f/2;
        dst[f] = srcL[srcFrame];
        dst[f+1] = srcR[srcFrame];
    }
}

void packMulti(sample_t ** src, sample_t * dst, const int channels, const frame_t frames) {
    frame_t framesTotal = frames * channels;
    frame_t flocal = 0;
    for(frame_t f=0; f<framesTotal; f+=channels) {
        for(int i=0; i<channels; ++i) {
            dst[f+i] = src[i][flocal];
        }
        ++flocal;
    }
}

// for frames dstL[f] = src[f];
//            dstR[f] = src[f+1];
//frames for src size - dst size * 2
void unpackStereo(sample_t * src, sample_t * dstL, sample_t * dstR, const frame_t frames) {
    for(frame_t f=0; f<frames; f+=2) {
        frame_t dstFrame = f/2;
        dstL[dstFrame] = src[f];
        dstR[dstFrame] = src[f+1];
    }
}

void unpackMulti(sample_t * src, sample_t ** dst, const int channels, frame_t frames) {
    frame_t framesTotal = frames * channels;
    frame_t flocal = 0;
    for(frame_t f=0; f<framesTotal; f+=channels) {
        for(int i=0; i<channels; ++i) {
            dst[i][flocal] = src[f+i];
        }
        ++flocal;
    }
}

//for frames src[f] = value;
void fillAudioBuffer(sample_t * src, const frame_t frames, const sample_t value) {
    for(frame_t f=0; f<frames; ++f) {
        src[f] = value;
    }
}


/*
... сложно... если вызераем определённому каналу то что делать со вторым каналом? заполнять перкому конце нулями? или же 
    вырезанный отрезок нулями? как то не годиться... может делать просто для всех каналов одинаково? 
    не понятно что делать с пустотой в других канала - если вырезал отрезок только из одного канала, и хочешь вставить в файл
    где 2 канала, но в какой то определённый, то во втором канале образуется пустота - либо в том месте либо в конце...
    
*/
/*
    cut out region from start to end from src and place it to dst. 
    if channels == 0 than for all channels, otherwise for specified(filling everything else with zeroes)
 */
void cutBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end) {

}

/* copy region from start to end and place it into dst
    if channels == 0 than for all channels, otherwise for specified(filling everything else with zeroes)
*/
void copyBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end) {

}

void pasteBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end) {

}

void deleteBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end) {

}

void silenceBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end) {

}

void reverseBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end) {

}

void reversePhase(AudioBuffer * src,  const int channels, const frame_t start, const frame_t end) {

}


}