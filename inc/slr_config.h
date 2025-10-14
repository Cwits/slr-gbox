// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace slr {

#define RT_TRACE 0

/* Audio Buffer Manager */
#define DEFAULT_BUFFER_COUNT 64
#define DEFAULT_RECORD_BUFFER_COUNT 16
#define DEFAULT_RECORD_BUFFER_MUL (100*5) //make sure this buffer is big enough...
#define DEFAULT_BUFFER_CHANNELS 2

#define RECORD_QUEUE_LENGTH 64

}