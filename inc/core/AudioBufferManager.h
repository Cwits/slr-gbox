// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include "core/primitives/AudioBuffer.h"
#include <vector>

namespace slr {

namespace AudioBufferManager {
    bool init(int blockSize, int channels);
    bool shutdown();

    AudioBuffer * acquireRegular();
    void releaseRegular(AudioBuffer * b);
    bool expandRegularPool();
    const int freeRegularCount();
    const int totalRegularCount();
    
    RT_FUNC AudioBuffer * acquireRecord();
    RT_FUNC void releaseRecord(AudioBuffer * b);
    bool expandRecordPool();
    const int totalRecordCount();

    const frame_t recordBufferSize();
}

}



/*

You're heading in a practical direction with the singleton — that's very common for global buffer managers in audio engines. But you’ve caught an important subtlety:

---

## 🔍 Your question:

> “If I make `AudioBufferManagerInst` a static inside `getInstance()`, will it be cleaned up on app exit?”

### ✅ Short answer:

Yes — it **will** be destroyed **at process exit**, but **how safely** depends on **how the app exits** and **how other objects are destroyed**.

---

## ✅ What Happens to `static` Inside Function?

In:

```cpp
static AudioBufferManagerInst _inst;
```

* It’s **guaranteed** by the C++ standard to:

  * Be created **once**, the first time `getInstance()` is called
  * Be **destroyed in reverse order** of construction at program exit

So: if your app **exits cleanly**, the singleton **will be destructed**.

---

## ⚠️ When Destruction May Not Happen

1. **Abnormal termination**

   * `exit()`, `std::terminate()`, crash, signal → **no destructor is run**

2. **Other global objects already destroyed**

   * If another global object uses `AudioBufferManager::getInstance()` during its destructor, and `_inst` is already destroyed → 💥 Undefined behavior

---

## 🧱 So Should You Worry?

| Scenario                  | Action Needed                   |
| ------------------------- | ------------------------------- |
| App exits cleanly         | ✅ Singleton destructor will run |
| App exits via signal      | ❌ No destructors are called     |
| Destruction order matters | ⚠️ Yes, may cause issues        |

---

## ✅ Good Practices (Recommended for Audio Apps)

### ✅ 1. *Don’t* depend on destructor for cleanup

Instead, provide an explicit `.shutdown()` method:

```cpp
static void shutdown() {
    getInstance().shutdown();
}
```

And call it manually during app cleanup (from `main()`, or `App::deinit()`)

---

### ✅ 2. If you want explicit control: use pointer singleton

```cpp
class AudioBufferManager {
public:
    static AudioBufferManager* instance() {
        static auto* _inst = new AudioBufferManager();
        return _inst;
    }
    static void destroy() {
        delete instance();  // manual cleanup
    }
};
```

* This gives you **explicit control** of destruction timing
* Useful if you need to clear threads, background I/O, etc.

---

## ✅ TL;DR

| Question                            | Answer                                        |
| ----------------------------------- | --------------------------------------------- |
| Will static singleton be destroyed? | ✅ Yes, if app exits normally                  |
| Do I need to worry?                 | ⚠️ Slightly — if order of destruction matters |
| Best practice?                      | ✅ Provide `shutdown()` and call it            |
| Can I ignore destructor cleanup?    | ✅ If OS will reclaim memory, yes              |

---

Would you like me to generate a robust, portable singleton wrapper with `init()`, `shutdown()`, and optional override hooks for testing or profiling?


*/