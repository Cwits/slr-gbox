#pragma once

#include <alsa/asoundlib.h>

namespace PushLib {

class PushCore;
struct MidiInterface {
    MidiInterface(PushCore *core);
    ~MidiInterface();

    bool connect(snd_rawmidi_t *in, snd_rawmidi_t *out);
    void disconnect();
};


}