// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/AudioBuffer.h"
namespace slr {

// class AudioBuffer;

void sumAudioBuffers(const sample_t * src, sample_t * dst, const frame_t frames);
void mulAudioBuffers(sample_t * src, sample_t * dst, const frame_t frames);
void mulAudioBufferToValue(sample_t * src, const frame_t frames, const sample_t value);
void mulAudioBuffersToValue(sample_t * src, sample_t * src2, const frame_t frames, const sample_t value);
void copyAudioBuffer(sample_t * src, sample_t * dst, const frame_t frames);
void clearAudioBuffer(sample_t * src, const frame_t frames);
void clearAudioBuffers(sample_t * src1, sample_t * src2, const frame_t frames);
void packTwoMonoToStereo(sample_t * srcL, sample_t * srcR, sample_t * dst, const frame_t frames);
void packMulti(sample_t ** src, sample_t * dst, const int channels, const frame_t frames);
void unpackStereo(sample_t * src, sample_t * dstL, sample_t * dstR, const frame_t frames);
void unpackMulti(sample_t * src, sample_t ** dst, const int channels, const frame_t frames);
void fillAudioBuffer(sample_t * src, const frame_t frames, const sample_t value);

/*
//apply for all - if channels == 0 than do for all channels, otherwise for specified
//all buffers must be prepared and memory must be already allocated!!!
*/
void cutBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end);
void copyBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end);
//if start==end or end-start < copied size than dst buffer will grow, if end-start > copied size than will shrink
void pasteBuffer(AudioBuffer * src, AudioBuffer * dst, const int channels, const frame_t start, const frame_t end);
void deleteBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end);
void silenceBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end);
void reverseBuffer(AudioBuffer * src, const int channels, const frame_t start, const frame_t end);
void reversePhase(AudioBuffer * src,  const int channels, const frame_t start, const frame_t end);
}