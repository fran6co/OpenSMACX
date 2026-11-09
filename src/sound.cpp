/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */

// The sound TU (0x004C5C50-0x004C9411): the sound module loader, the Sound
// family (Sound, Wave, Midi, Ambience, Video and Voice lifetimes), the name
// table and the small word/time/string helpers the image places between them.
// The boundary is inferred from the image; the original file name is unknown.
#include "stdafx.h"
#include "sound.h"
#include "basepop.h"
#include "wave.h"
#include "wave_device.h"
#include "sounddevice.h"
#include "voice.h"
#include "general.h"
#include "fx.h"
#include "dialog.h"
#include "net_class.h"
#include <cstring>
#include "midi.h"
#include "ambience.h"
#include "video.h"
#include <new>
#include <mmsystem.h>
#include "name_table.h"
#include "word_state.h"
#include "time.h"
#include "win.h"
#include "multiwin.h"
#include "stringstruct.h"
#include "spritebox.h"
#include "technology.h"
#include "random.h"
#include "popup.h"
#include "strings.h"
#include "heap.h"
#include "faction.h"
#include "alpha.h"
#include "game.h"
#include "reportwin.h"
#include "text.h"
#include "console.h"
#include "buffer.h"
#include "graphicwin.h"
#include "worldwin.h"
#include "font.h"
#include "sprite.h"
#include "veh.h"
#include "lock.h"
#include "log.h"
#include "pulldown.h"
#include "diplopop.h"
#include "mapwin.h"
#include "map.h"
#include "maininterface.h"
#include "netdaemon.h"
// The sound module loader and Sound class share their real storage and
// compiler context here. Original object-file boundaries remain inferred.

// The process wave, MIDI and wave-in devices open this TU: their CRT
// initializers are its first functions and their .bss cells interleave with
// the loader's storage below (0x0090D950..0x0090DB7C).
// ORIGINAL: 0x004C5C50 sub_4c5c50 0x004C5C50-0x004C5C66 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682608 selects startup 0x4c5c50..0x4c5c66 and registers cleanup 0x4c5c70..0x4c5c7a. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?WaveDeviceGlobal@@3VWave_Device@@A is selected by physical COFF constructor/destructor relocations; current emitted initializer is identified by the adjacent symbol fact. Original receiver 0x90d978 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original init operation to the genuine C++ lifetime of ?WaveDeviceGlobal@@3VWave_Device@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E16
// ORIGINAL: 0x004C5C70 sub_4c5c70 0x004C5C70-0x004C5C7A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x682608 selects startup 0x4c5c50..0x4c5c66 and registers cleanup 0x4c5c70..0x4c5c7a. Complete root decode proves the exclusive RET/JMP endpoints.", "The source-owned object ?WaveDeviceGlobal@@3VWave_Device@@A is selected by physical COFF constructor/destructor relocations; current emitted finalizer is identified by the adjacent symbol fact. Original receiver 0x90d978 belongs to this real global; preserve its class lifetime and registration.", "Buffer relative declaration order is PCX, interface array, middle, vehicle, special-text owner, Sprite, shared Buffer. Generated helpers are compiler output, not hand-authored address callbacks. Selected same-owner order does not establish full CRT order, dependency provider, linked preservation or runtime/game acceptance. Guarded cleanup is a separate unfinished obligation."], "reason": "Bind original fini operation to the genuine C++ lifetime of ?WaveDeviceGlobal@@3VWave_Device@@A; retain original behavior and remaining matching obligations."}
// symbol    _$E14
// ORIGINAL: 0x004C5C80 ??__Eg_MIDI_DEVICE@@YAXXZ 0x004C5C80-0x004C5C96 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "MIDI construction and atexit registration at original receiver 0x0090D950; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E21
// ORIGINAL: 0x004C5CA0 ??__Fg_MIDI_DEVICE@@YAXXZ 0x004C5CA0-0x004C5CAA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "MIDI static destruction tail call at original receiver 0x0090D950; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E19
// ORIGINAL: 0x004C5CB0 ??__Eg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CB0-0x004C5CC6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "Wave-In construction and atexit registration at original receiver 0x0090DB50; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E26
// ORIGINAL: 0x004C5CD0 ??__Fg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CD0-0x004C5CDA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "Wave-In static destruction tail call at original receiver 0x0090DB50; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E24
// Pinned local CRT slots 0x682608/60C/610 select Wave, Midi, Wave-In startup
// helpers 0x4C5C50/80/B0 and their cleanup helpers 0x4C5C70/A0/D0. The real
// object declarations below follow that observed local sequence. This does not
// establish wider CRT order or full object extents; bindings remain start-only.
// ====================
// The process wave device, 0x0090D978 in the image - REAL STORAGE carrying
// the public name wave.h's consumers read. It was `Wave_Device g_WAVE_DEVICE`
// beside the `Wave_Device *const WaveDeviceGlobal = (Wave_Device *)0x0090D978`
// binding in wave.h naming terranx.exe data that is unmapped in a standalone
// build; the object and the binding were the same address, so the object now
// carries the public name and every `WaveDeviceGlobal->` call site reaches it
// directly, the folded `mov ecx, imm32` receivers relocated.
// The 0x1A4-byte device ends where SoundChainTail begins at 0x0090DB1C.
// RECOVERY_OBJECT: v2 0x0090D978 0x0090DB1C ?WaveDeviceGlobal@@3VWave_Device@@A 0x00491380
Wave_Device WaveDeviceGlobal;  // 0x0090D978


// RECOVERY_OBJECT: v1 0x0090D950 ?g_MIDI_DEVICE@@3VMidi_Device@@A
Midi_Device g_MIDI_DEVICE;
// RECOVERY_OBJECT: v1 0x0090DB50 ?g_WAVE_IN_DEVICE@@3VWave_In_Device@@A
Wave_In_Device g_WAVE_IN_DEVICE;
// The release-hook guard, 0x0090DB7C in the image: init_sound zeroes it when
// the device fails to come up and sets it once the hook is live, and the
// wave teardown paths run the release slot only while it reads nonzero. Was
// `int *const WaveDeviceReleaseGuard = (int *)0x0090DB7C` (wave.h) and a
// `g_0090db7c` static in sound.cpp - the same terranx.exe dword twice.
// RECOVERY_OBJECT: v1 0x0090DB7C ?WaveDeviceReleaseGuard@@3HA
int WaveDeviceReleaseGuard;  // 0x0090DB7C

// The Midi, Ambience and Wave hooks call through the product-owned
// SoundDllProcs table (sound.h), not wave.h's fixed-address accessors: the
// image calls `dword ptr [0x90DB24]`/`[0x90DB28]` with a HIGHLOW fixup at each
// site, and only a symbol-based spelling emits the DIR32 relocation the
// checker can see.


// These are behavior views over observed fields, not names or complete
// allocation claims for either original class. No connection between their
// construction/use/lifetime is inferred from the nearby addresses.
class RangePrefixInitializationView {
public:
    RangePrefixInitializationView();
    unsigned int unknown_dword_0_;
    unsigned int zeroed_dword_4_;
    unsigned int unknown_dword_8_;
    unsigned int unknown_dword_C_;
};



// Keep the real base-to-derived lifecycle definitions visible together.
// Original constructors publish the SoundFadeState, Sound and derived tables
// around real memset calls; derived destruction contains the Sound stage inline.
// These are ordinary class lifetimes, not manual table or unwind substitutes.
// Standalone identities and shared unwind obligations remain mapped below.

// Independent zero-initialized pointer objects used by the common chain.
// Original attach writes head at 0x004C6387 and tail at 0x004C638D;
// Sound::detach and Wave_Device::select use the same storage identities.
// RECOVERY_OBJECT: v2 0x0090DB20 0x0090DB24 ?SoundChainHead@@3PAVSound@@A 0x004C6370
Sound *SoundChainHead;
// RECOVERY_OBJECT: v2 0x0090DB1C 0x0090DB20 ?SoundChainTail@@3PAVSound@@A 0x004C6370
Sound *SoundChainTail;


namespace {
// Historical LEVER/TRIED notes below document manual dispatch spellings that
// predate this typed external call view; their compiler observations remain
// useful history, not descriptions of the current call expressions.
// Sound wraps its device at 0x3C and asks it these questions through the
// device's own vtable; the original tail-jumps, so the device's answer is the
// caller's, and with no device wrapped the answer is zero.
__forceinline Sound *sound_device(Sound *device) {
    return device;
}
}  // namespace


namespace {
}  // namespace

// Root slot 0 has the same original body as root slot 1. Sound overrides
// only this slot to notify its wrapped device; the root only remembers it.
// RECOVERY_SHARED: v1 0x004482F0 0x0044830C ?set_fade@SoundFadeState@@UAEHK@Z 0x004C6080
int SoundFadeState::set_fade(unsigned long interval) {
    if (interval == 0) {
        return 10;
    }
    fade_38_ = interval;
    return 0;
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?unload@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::unload() {
  return 0;
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_game_hwnd@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::get_game_hwnd() {
  return 0;
}

// RECOVERY_SHARED: v1 0x4c93e0 0x4c93e8 ?set_attrib@VideoSoundState@@UAEHK@Z 0x4c8620
int VideoSoundState::set_attrib(unsigned long) {
  return 0x0B;
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_attrib@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::get_attrib() {
  return 0;
}

namespace {
// SoundX mode-1 constructor10028750 clears its child at+3C; factory helper
// 10028B70 only replaces the filename. Its getter10035A30->10028F40 therefore
// has no nested callback on the newly-created paths in dyna_load/init.
// The original captures the provider vptr before get_attrib and rereads the
// receiver afterward; the typed calls preserve behavior for this pinned
// read-only getter, not arbitrary replacement/reentrant provider DLLs.
inline WavePlaybackDeviceInterface *wave_playback_device(
    Sound *device) {
    return static_cast<WavePlaybackDeviceInterface *>(device);
}

inline uint32_t wave_streaming_mode(unsigned long flags) {
    return flags & 4;
}
}


// RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?get_game_hwnd@Midi@@UAEHXZ 0x004C75B0
int Midi::get_game_hwnd() { return 0; }

// RECOVERY_SHARED: v1 0x00406B30 0x00406B33 ?get_attrib@Midi@@UAEHXZ 0x004C75B0
int Midi::get_attrib() { return 0; }

class TimeRangeReceiverView {
public:
    // Endpoint operations and sample share only the observed fields;
    // original class and lifetime ownership are not established.
    void set_lower_endpoint(int value);
    void set_upper_endpoint(int value);
    int sample();
private:
    unsigned char unobserved_prefix_0_[4];
    unsigned char enabled_flags_4_;
    unsigned char unobserved_bytes_5_7_[3];
    int lower_bound_8_;
    int upper_bound_C_;
};


/*
// ORIGINAL: 0x004C5CE0 ?init_sound@@YAHPAXK@Z 0x004C5CE0-0x004C5D8E BYTE_EXACT
// size      174 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C4F40 0x004C57A0 0x004C5A10 0x004C5E50
//
// Promoted 2026-08-15 from src/unrecovered/004c5ce0.cpp to retire its
// pending_bodies forwarder. Brings up the wave device, then the midi and
// wave-in devices per the backend bitmask. The driver-unload and FreeLibrary
// calls are read out of their fixed slots, as the original does.
Status: Complete
*/
int __cdecl init_sound(void *window, unsigned long backends) {
    int loadResult = load_sound_dll();
    if (loadResult != 0) {
        return loadResult;
    }
    if (SoundDllModule != 0) {
        SoundXOrdinal3InitTimer()(0, 0);
    }
    int result = WaveDeviceGlobal.init(window, backends);
    if (result != 0) {
        // THE GUARD IS READ BEFORE THE STORE. The image loads [0x90db78]
        // at 0x004C5D21 and only then writes 0 to [0x90db7c]; the two are
        // different addresses, so the order is free and the image picked one.
        const HMODULE module = SoundDllModule;
        WaveDeviceReleaseGuard = 0;
        if (module != 0) {
            FreeLibrary(module);
            SoundDllModule = 0;
        }
        memset(SoundDllProcs, 0, sizeof(SoundDllProcsTable));
        return result;
    }
    if ((backends & 2) != 0) {
        // QUALIFIED CALLS: the image calls both device inits directly
        // (`call 0x4c57a0` / `call 0x4c5a10`, receiver in ecx), not through
        // the vtables - an unqualified call virtual-dispatches and the
        // shapes diverge.
        g_MIDI_DEVICE.Midi_Device::init(window, 2);
    }
    if ((backends & 8) != 0) {
        g_WAVE_IN_DEVICE.Wave_In_Device::init(window, backends);
    }
    WaveDeviceReleaseGuard = 1;
    return 0;
}

// ORIGINAL: 0x004C5D90 ?release_sound@@YAHXZ 0x004C5D90-0x004C5E42 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:edbf268ff962712fadc389b8283e4c795b6d27a2:src/unrecovered/004c5d90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "sound.cpp; sound.h adds only a friend declaration for existing protected Sound::device_/fname_ and the free-function prototype. Calls the current SoundDllProcs entries, Sound::detach, existing device release methods and unload_sound_dll. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
int __cdecl release_sound() {
    if (WaveDeviceReleaseGuard == 0) {
        return 1;
    }

    SoundXOrdinal4ReleaseSound()();

    while (SoundChainHead != 0) {
        Sound *sound = SoundChainHead;
        void *device = sound->device_;
        if (device != 0) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device);
            }
            sound->device_ = 0;
        }
        if (sound->fname_ != 0) {
            ::operator delete(sound->fname_);
            sound->fname_ = 0;
        }
        sound->detach();
    }

    g_MIDI_DEVICE.release();
    WaveDeviceGlobal.release();
    g_WAVE_IN_DEVICE.release();
    unload_sound_dll();
    return 0;
}

// SDK dependency for releasing the dynamically loaded sound module.
// RECOVERY_IMPORT: v1 __imp__FreeLibrary@4 KERNEL32.dll name:FreeLibrary

// ORIGINAL: 0x004C5E50 ?load_sound_dll@@YAHXZ 0x004C5E50-0x004C5F68 FILE BYTE_EXACT
// A plain indexed for loop over SoundDllProcs; the failure path is the
// inlined unload_sound_dll followed by a second guard clear, and the version
// probe is the inlined get_sound_version.
// size      280 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C9080 0x0062D390
// indirect  0x004C5E65 0x004C5E9C 0x004C5ED1 0x004C5EFD

// Earlier recovery: rehomed from src/unrecovered/004c5e50.cpp into a separate TU (not sound.cpp,
// which owns the call site) so init_sound can call the real body instead of a
// forward into the original image. Its one unrecovered callee, the message
// helper below, is homed in the same file so the pair loads as one story.

// ---- callees, declared and never defined here ----
// sound_header_version runs from general.cpp: the sound-header version the
// game was built with, a real `mov eax,1 / ret` recovered under its own name.
extern "C" int __cdecl sound_header_version();
// sound_version_warning is homed below, under its own marker in this file.
extern "C" void __cdecl sound_version_warning(const char *text, const char *caption);

// The module handle and the version slot are sound.h's SoundDllModule and
// SoundDllProcs[10]. The three kernel32 entrypoints are direct API calls:
// the image calls them through its own import table, and this tree does the
// same through its own. The strings are the image's .rdata/.data text,
// recovered verbatim.

// The module handle and the eleven export slots, the loader's own
// storage: SoundDllModule keeps the LoadLibraryA reference once
// load_sound_dll fills it, and stays null when the DLL is absent.
// The pinned loader stores LoadLibraryA's result at 0x004C5E6D and the
// timer wrapper reads it at 0x004C5FB3. This identifies the handle's start,
// not a recovered allocation boundary or a separate initialization proof.
// RECOVERY_OBJECT: v1 0x0090DB78 ?SoundDllModule@@3PAUHINSTANCE__@@A
HMODULE SoundDllModule;
// Original loader 0x004C5E85 starts at 0x0090DB24, advances by four bytes
// through ordinal 11, and stops at 0x0090DB50 (0x004C5EA8). Its failure
// path clears the same eleven words. This is pre-loader storage, not a
// claim that the DLL exports or their ABI have been verified.
// RECOVERY_OBJECT: v2 0x0090DB24 0x0090DB50 ?SoundDllProcs@@3PAPAXA 0x004C5E50
SoundDllProcsTable SoundDllProcs;

int __cdecl load_sound_dll() {
    if (SoundDllModule == 0) {
        SoundDllModule = LoadLibraryA(".\\soundx.dll");
        if (SoundDllModule == 0) {
            return 1;
        }
        for (int index = 0; index < 11; ++index) {
            SoundDllProcs[index] = reinterpret_cast<void *>(GetProcAddress(
                SoundDllModule, MAKEINTRESOURCEA(index + 1)));
            if (SoundDllProcs[index] == 0) {
                unload_sound_dll();
                WaveDeviceReleaseGuard = 0;
                return 1;
            }
        }

        unsigned int version = get_sound_version();
        if ((version & 0xff00) != 0x100) {
            sound_version_warning("The sound header files used in the game do not match "
                       "the ones used in sound.dll. Check all sound.h and "
                       "sound device.h versions!",
                       "Sound Version Warning");
        }
        if (sound_header_version() != 1) {
            sound_version_warning("The sound header files used by jackal do not match "
                       "the ones used by the game. Check all sound.h and "
                       "sound device.h versions!",
                       "Sound Version Warning");
        }
        if (sound_header_version() != ((version >> 8) & 0xff)) {
            sound_version_warning("The sound header files used by jackal do not match "
                       "the ones used in sound.dll. ",
                       "Sound Version Warning");
        }
    }
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C5F70 ?unload_sound_dll@@YAXXZ 0x004C5F70-0x004C5FA5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["docs/recovery/derived-prototypes.csv:941; src/recovered/004c5f70.cpp; src/sound.cpp:SoundDllModule/SoundDllProcs ownership"], "reason": "Typed owner body unloads the already owned SoundDllModule, clears the release guard, and clears the eleven-slot export table; original derived prototype and matching recovered seed establish void __cdecl signature and span."}
void __cdecl unload_sound_dll() {
    HMODULE module = SoundDllModule;
    WaveDeviceReleaseGuard = 0;
    if (module != 0) {
        FreeLibrary(module);
        SoundDllModule = 0;
    }
    memset(SoundDllProcs, 0, sizeof(SoundDllProcsTable));
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C5FB0 ?init_sound_timer@@YAHKK@Z 0x004C5FB0-0x004C5FD6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["docs/recovery/derived-prototypes.csv:942; src/recovered/004c5fb0.cpp; src/sound.cpp:SoundDllProcs[2] call sites"], "reason": "Typed owner forwards the two uint32 timer arguments to sound export-table index 2 and preserves the absent-module return 4. Derived prototype and seed establish int cdecl (unsigned int, unsigned int)."}
int __cdecl init_sound_timer(unsigned long timer, unsigned long resolution) {
    if (SoundDllModule == 0) {
        return 4;
    }
    return SoundXOrdinal3InitTimer()(timer, resolution);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C5FE0 ?get_sound_version@@YAHXZ 0x004C5FE0-0x004C5FF2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["docs/recovery/derived-prototypes.csv:943; src/recovered/004c5fe0.cpp; src/sound.cpp:SoundDllProcs[10] version probe"], "reason": "Typed owner checks the existing loader module and dispatches the already-owned ordinal 11/version slot, returning zero when unavailable. Original prototype and seed establish int cdecl ()."}
int __cdecl get_sound_version() {
    if (SoundDllModule == 0) {
        return 0;
    }
    return static_cast<int>(SoundXOrdinal11Version()());
}

// Ordinal 1 is the device-creation callback already typed by
// SoundXOrdinal1CreateSound and its Wave/Midi call sites. The image gates this
// wrapper on the same release guard as other sound-device callbacks.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C6000 sub_4c6000 0x004C6000-0x004C602A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed src/recovered/004c6000.cpp; docs/recovery/ghidra-functions.csv:1214; src/sound.h:SoundXOrdinal1CreateSound; src/wave.cpp/src/midi.cpp slot-0 calls"], "reason": "Image wrapper gates ordinal 1 on the existing wave-device release guard and forwards its three stack arguments to the typed slot-0 device-create callback; stack cleanup matches cdecl. The same signature is exercised by Midi/Wave call sites."}
// symbol    _sub_4c6000
extern "C" int __cdecl sub_4c6000(Sound **device_slot,
                                 const char *filename,
                                  int mode) {
    if (WaveDeviceReleaseGuard == 0) {
        return 1;
    }
    return SoundXOrdinal1CreateSound()(device_slot, filename, mode);
}

// The seed preserves the callback's EAX return. Wave/Midi release callers
// historically viewed this slot as void(void*); they still ignore its result.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C6030 sub_4c6030 0x004C6030-0x004C6052 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed src/recovered/004c6030.cpp and src/wave.h:func_wave_device_release; original image disassembly 0x004C6030-0x004C6052; src/sound.h:SoundXOrdinal2DeleteSound; src/midi.cpp:Midi::~Midi"], "reason": "The original wrapper tests the wave release guard, pushes one 32-bit argument, calls slot 1, explicitly cleans four stack bytes, and returns callback EAX. The shared contract is int cdecl(void*); Wave/Midi callers ignore EAX without changing the shared cell or calling convention."}
// symbol    _sub_4c6030
extern "C" int __cdecl sub_4c6030(void *device) {
    if (WaveDeviceReleaseGuard == 0) {
        return 1;
    }
    return SoundXOrdinal2DeleteSound()(device);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C6060 sub_4c6060 0x004C6060-0x004C6071 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004c6060.cpp; docs/recovery/agent-structure-observations.csv:42; src/sound.cpp:SoundDllProcs ownership"], "reason": "Typed source zeros the eleven loader-owned export words, matching the seed block clear. Original seed has a zero-argument return; conservative neutral cdecl void spelling preserves observed no-argument use."}
// symbol    _sub_4c6060
extern "C" void __cdecl sub_4c6060() {
    memset(SoundDllProcs, 0, sizeof(SoundDllProcsTable));
}
// The loader above inlines its table clears; every later memset in this TU
// is a real CRT call in the image.
#pragma function(memset)
// stand-in: a nothrow redeclaration. The image's derived audio constructors
// (Wave, Ambience, 0x004C75B0) call memset without opening a base-complete
// unwind state, so the original treated this call as non-throwing.
extern "C" void * __cdecl memset(void *, int, size_t) throw();

// Construct the Sound-owned fields after the fade prefix. The original
// flags clear covers one complete dword and does not initialize delay_.
// The first table publication in 0x004C6080 and final publication in
// 0x004C6120 identify the base table. Its complete original extent remains
// unclaimed; two observed slots are not a full linked-table proof.
// RECOVERY_GENERATED: v1 vtable 0x0066E444 ? ??_7SoundFadeState@@6B@ 0x004C6080
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// Historical SHARED_TAIL observation below predates the independent source identity for 0x004C8450; retain it as prior trial context, not a current span classification.
// TRIED: no tier is reachable for ANY body - SHARED_TAIL, the helper at 0x004C8450 (mov [ecx],0x66e444; ret) is COMDAT-folded with another catalogued function, same wall as VoiceRx::VoiceRx() at 0x004C8960. Straight-line field reconstruction below, no attempt at the SEH try/catch unwind frame (bare try/catch with rethrow produced no fs:[0] frame at all under this compiler's flags; the scaffold gives Sound no base class to hang the automatic base/derived vtable-swap unwind on, same as the already-tried ~UV2Player shape).
// ORIGINAL: 0x004C6080 ??0Sound@@QAE@XZ 0x004C6080-0x004C611A;0x00659ED0-0x00659EE2 BYTE_EXACT
Sound::Sound() {
    chain_prev_ = 0;
    chain_next_ = 0;
    device_ = 0;
    fname_ = 0;
    memset(&flags_40_, 0, sizeof(flags_40_));
    flags_40_ &= ~1u;
    Sound::set_fade(1000);
    type_ = 0;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C61E0 0x004C6260 0x004C627C 0x004C61EE
/*
Purpose: Record the sound's type. Types 1..7 - except 3, which the original's
         jump table routes to the invalid arm - store the type at 0x50 and OR
         a per-type class bit into the flag dword at 0x40: 1 -> 4, 2 -> 8,
         4 -> 0x10, 5 -> 0x28, 6 -> 0x100, 7 -> 0x80. Anything else stores
         type 0 and leaves the flags alone.
// ORIGINAL: 0x004C61E0 ?set_type@Sound@@QAEXI@Z 0x004C61E0-0x004C6260 BYTE_EXACT
// symbol    ?set_type@Sound@@UAEXI@Z
// size      128 bytes
// prototype void (__thiscall ?set_type@Sound@@QAEXI@Z)(Sound* this, SOUNDTYPE)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER, two parts. (1) each `case` inlines its own `type_ = a1;
//        flags_40_ |= N; return;` rather than sharing a `class_bit` local
//        and a common tail - the image duplicates the four-instruction
//        block per arm with no merged epilogue. (2) the CATALOGUED mapping
//        1->4 / 4->0x10 is backwards: the jump table's own byte order says
//        1->0x10 and 4->4 - `constant #9/#21 or` swapped is what measure
//        named it, and swapping the two immediates is what closed it.
Return Value: n/a
Status: Complete
*/
// The historical LEVER above mistook instruction order for selector order.
// The original seven-entry switch table at 4C6260..4C627C lies outside the
// recorded instruction span: its type-1 entry reaches 4C6215 (OR 4), and
// its type-4 entry reaches 4C61F5 (OR 0x10). Keep that dispatch as well as
// the original arm order; matching the instructions alone misses this error.
// stand-in: auto_inline(off) keeps set_type out of line in this TU. The image
// calls it from the Wave, Midi, Ambience and Voice constructors (0x004C6787,
// 0x004C8A14); with the definition visible, /Ob2 inlines the switch instead.
#pragma auto_inline(off)
void Sound::set_type(unsigned int type) {
    // EACH ARM INLINE, not a shared `type_ = type; flags_40_ |= class_bit;`
    // tail: the image duplicates the store-and-OR in every case block and
    // returns from each directly, rather than merging them.
    switch (type) {
    case 4:
        flags_40_ |= 0x10;
        type_ = type;
        return;
    case 2:
        flags_40_ |= 8;
        type_ = type;
        return;
    case 1:
        flags_40_ |= 4;
        type_ = type;
        return;
    case 5:
        flags_40_ |= 0x28;
        type_ = type;
        return;
    case 6:
        flags_40_ |= 0x100;
        type_ = type;
        return;
    case 7:
        flags_40_ |= 0x80;
        type_ = type;
        return;
    default:
        type_ = 0;
        return;
    }
}
#pragma auto_inline(on)

/*
Purpose: Load the sound from a filename. The name resolves through the
         recovered filefind_get (0xA when it cannot); a dead creation-hook
         guard answers 1; a missing device is built from the resolved path
         through the guarded hook, while an existing one is first asked
         through its vtable slot 0x60 whether it is busy (0xF if so). The
         device loads the resolved path through its slot 0x10. On a first
         success the loaded bit of the 0x40 flag dword is set BEFORE the
         sound's own vtable slot 0x7C runs, and a nonzero loop dword at 0x30
         starts the device looping through its slot 0x48; on failure the
         loaded bit is cleared. Either way the resolved path is copied onto
         the game heap - new copy first, old one freed after - and remembered
         at 0x4C.
// ORIGINAL: 0x004C6280 ?load@Sound@@QAEHPBD@Z 0x004C6280-0x004C6361 BYTE_EXACT
// symbol    ?load@Sound@@UAEHPBD@Z
// LEVER: the two vtable dispatches (slots 0x60 and 0x10) spelled with `vtable_method<Fn>(object, offset)` instead of `original_slot<Fn>(vtable + offset)` - single indirect call, matching the image's `call dword ptr [reg+N]` where the slot-read idiom cost an extra `mov`. Took this from 44/102 to 100/102.
// TRIED: the one remaining byte is the slot-0x60 dispatch alone - image loads the vtable into EDX (`mov edx,[ecx]; call [edx+0x60]`), this tree into EAX. Naming the method pointer in its own local first made it much WORSE (47/102) rather than fixing the register; reverted. Not chased further.
// size      225 bytes
// prototype int (__thiscall ?load@Sound@@QAEHPBD@Z)(Sound* this, int8*)
// callers   5   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006005D0 0x006453E0 0x00645460 0x0064557F 0x0064558A
// indirect  0x004C62C9 0x004C62DF 0x004C62F7 0x004C6311 0x004C6321
Return Value: the device's load answer, 0xA for an unresolvable name, 1 for
              a dead creation hook, 0xF for a busy device, or the creation
              error
Status: Complete
*/
int Sound::load(const char *fname) {
    char *const resolved = filefind_get(fname);
    if (!resolved) {
        return 0xA;
    }
    if (!WaveDeviceReleaseGuard) {
        return 1;
    }
    if (!device_) {
        const int created = (SoundXOrdinal1CreateSound())(&device_, resolved, 1);
        if (created) {
            return created;
        }
    } else {
        if (sound_device(device_)->unk_slot24()) {
            return 0xF;
        }
    }
    int result;
    {
        result = sound_device(device_)->load(resolved);
    }
    if (result == 0) {
        if (!(flags_40_ & 1)) {
            flags_40_ |= 1;
            attach();
            if (loop_flag_30_) {
                sound_device(device_)->set_loop_state(1);
            }
        }
    } else {
        flags_40_ &= ~1u;
    }
    char *const copy =
        static_cast<char *>(operator new(strlen(resolved) + 1));
    strcpy(copy, resolved);
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = copy;
    return result;
}


/*
Purpose: The compiler-generated scalar deleting destructor: destroy the
         sound and, when bit 0 of the mode asks, free the storage to the
         game heap.
// ORIGINAL: 0x004C92D0 ??_GSound@@UAEPAXI@Z 0x004C92D0-0x004C92F1 FILE BYTE_EXACT
// symbol    ??_GSound@@UAEPAXI@Z
// size      33 bytes
// prototype void* (__thiscall ??_GSound@@UAEPAXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6120 0x0064557F
Return Value: the object pointer
Status: Complete
*/
// The existing virtual destructor owns this compiler-generated entry.
// Historical seed evidence: its qualified Sound destructor call reproduced
// the original direct call. The unqualified form dispatched through slot 8,
// re-reading the mode from the stack. The real generated deleting destructor
// now owns that complete-destructor call; no handwritten dispatch is needed.


/*
Purpose: Join the sound chain at the tail. A sound with either neighbour
         already set is left alone. An empty chain makes it both ends;
         otherwise the old tail becomes its prev - re-read after the tail
         slot advances, as the original orders it - and learns its next.
         Either way the chained bit sets.
// ORIGINAL: 0x004C6370 ?attach@Sound@@QAEHXZ 0x004C6370-0x004C63C1 BYTE_EXACT
// symbol    ?attach@Sound@@UAEHXZ
// TRIED 2026-08-30, after the slot reconciliation (the image's vftable slot
//   29 is get_time 0x4C66A0 and slot 31 is attach - the old header had them
//   swapped with detach at 31, and detach 0x4C63D0 is not in the vftable at
//   all): now MEASURABLE, this body shows the image's persistent zero
//   register - `xor edx,edx` materialised once, then `cmp eax,edx` and
//   `cmp [ecx+0x44],edx` - where every source spelling emits `test`. Same
//   wall class as sub_63f9b0 (vox2 batch): 5/28, the virtualization made it
//   measurable, the zero-register plan awaits the flag-axis/emitter-level
//   investigation.
// size      81 bytes
// prototype int (__thiscall ?attach@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Sound::attach() {
    if (chain_next_ || chain_prev_) {
        return 0;
    }
    // BUG IN THE ORIGINAL: the empty-chain arm re-zeroes both links the guard
    // above has just proved null. The stores are unobservable in the running
    // program - but the image performs them, and they are what put the zero in
    // the register the image's `cmp eax,edx` / `cmp [ecx+0x44],edx` reuse, so
    // they are written here and left alone deliberately.
    // TRIED 2026-08-30 with the stores omitted: the image's persistent zero
    // register never materialises and the compares fall back to `test`.
    if (!SoundChainTail) {
        SoundChainHead = this;
        SoundChainTail = this;
        chain_next_ = nullptr;
        chain_prev_ = nullptr;
        flags_40_ |= 2;
        return 0;
    }
    chain_prev_ = SoundChainTail;
    SoundChainTail = this;
    chain_prev_->chain_next_ = this;
    chain_next_ = nullptr;
    flags_40_ |= 2;
    return 0;
}


/*
Purpose: Leave the sound chain: nothing at all for an unchained sound;
         otherwise the standard unlink with the head and tail slots
         maintained at the ends, both links cleared, and the chained bit
         dropped.
// ORIGINAL: 0x004C63D0 ?detach@Sound@@QAEHXZ 0x004C63D0-0x004C642C BYTE_EXACT
// symbol    ?detach@Sound@@UAEHXZ
// size      92 bytes
// prototype int (__thiscall ?detach@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Sound::detach() {
    if (!(flags_40_ & 2)) {
        return 0;
    }
    Sound *const prev = chain_prev_;
    if (prev) {
        prev->chain_next_ = chain_next_;
    } else {
        SoundChainHead = chain_next_;
    }
    Sound *const next = chain_next_;
    if (next) {
        next->chain_prev_ = chain_prev_;
    } else {
        SoundChainTail = chain_prev_;
    }
    chain_next_ = nullptr;
    flags_40_ &= ~2u;
    chain_prev_ = nullptr;
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 11.
// ORIGINAL: 0x004C6430 ?UNK1@Sound@@QAEHH@Z 0x004C6430-0x004C6438 BYTE_EXACT
// symbol    ?UNK1@Sound@@UAEHH@Z
// size      8 bytes
// prototype int (__thiscall ?UNK1@Sound@@QAEHH@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Sound::UNK1(int) {
    return 11;
}


/*
Purpose: Release the loaded sound. The wrapped device, if any, is asked to
         unload through its own vtable slot 0x14 and its answer is the
         result; the sound's own vtable slot 0x80 then runs UNCONDITIONALLY
         (Wave's override suppresses it by flag; the base does not), the
         device is forgotten, and the loaded bit at 0x40 is cleared.
// ORIGINAL: 0x004C6440 ?unload@Sound@@QAEHXZ 0x004C6440-0x004C6472 BYTE_EXACT
// symbol    ?unload@Sound@@UAEHXZ
// size      50 bytes
// prototype int (__thiscall ?unload@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C644F 0x004C6458
Return Value: whatever the device's unload returned, or 0 with none wrapped
Status: Complete
*/
int Sound::unload() {
    // BOTH dispatches call the slot WHERE IT LIVES - `call [eax+0x14]` on the
    // device, `call [reg+0x80]` on the sound - not the slot-read-then-call the
    // original_slot / original_method idioms emit.
    // The loaded bit is cleared with a plain read-modify-write, as detach's
    // byte-exact `flags_40_ &= ~2u` is: the byte-form `and al, 0xfe` comes
    // from that spelling, not from masking into a local first.
    // TRIED for the 0x80 dispatch, which the image reads into EDX and every
    // spelling here reads into EAX: a real virtual `detach();` call, and the
    // __fastcall `vtable_slot` free-function form. Two instructions of 21
    // differ on the register name alone - fade_in(unsigned) puts two
    // own-vtable dispatches in eax then edx, but here a device dispatch and a
    // branch merge sit between them and the allocator restarts at eax.
    int result = 0;
    Sound *device = device_;
    if (device) {
        // The no-argument device dispatch goes through the __fastcall slot
        // form, as play()/stop()/release() do - the receiver is already in
        // ecx, so there is no receiver pointer to name at the call site.
        result = sound_device(device)->unload();
    }
    detach();
    device_ = nullptr;
    flags_40_ &= ~1u;
    return result;
}


// `forward_sound_device` stood here until 2026-09-03. It carried every
// argument-forwarding dispatch through `original_slot` - a slot-read-then-call
// - and every body that reached it is byte-exact only now that each one spells
// the dispatch itself with `vtable_method`, which leaves the call operand on
// the slot.

/*
Purpose: Ask the wrapped device to play, through vtable slot 0x1C.
// ORIGINAL: 0x004C6480 ?play@Sound@@QAEHXZ 0x004C6480-0x004C6492 BYTE_EXACT
// symbol    ?play@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?play@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play() {
    Sound *device = device_;
    int result = 0x14;
    if (device) {
        result = sound_device(device)->play();
    }
    return result;
}

/*
Purpose: Ask the wrapped device to play the given sound, through vtable slot
         0x18.
// ORIGINAL: 0x004C64A0 ?play@Sound@@QAEHI@Z 0x004C64A0-0x004C64BC BYTE_EXACT
// symbol    ?play@Sound@@UAEHI@Z
// size      28 bytes
// prototype int (__thiscall ?play@Sound@@QAEHI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C64B5
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::play(unsigned int effect) {
    // LEVER: the same `device / result = 0x14 / if` shape the no-argument
    // play(), stop() and release() use, with `vtable_method` for the argument
    // dispatch - the image passes the effect thiscall-on-the-stack. The old
    // forward_sound_device helper read the slot into a register first and
    // compiled 3/12 here; the byte-exact no-argument family goes through
    // `vtable_slot` because its slot needs no argument.
    Sound *device = device_;
    int result = 0x14;
    if (device) {
        result = sound_device(device)->play(effect);
    }
    return result;
}

/*
Purpose: Ask the wrapped device to stop, through vtable slot 0x20.
// ORIGINAL: 0x004C64D0 ?stop@Sound@@QAEHXZ 0x004C64D0-0x004C64E2 BYTE_EXACT
// symbol    ?stop@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?stop@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::stop() {
    Sound *device = device_;
    int result = 0x14;
    if (device) {
        result = sound_device(device)->stop();
    }
    return result;
}

/*
Purpose: Ask the wrapped device to release, through vtable slot 0x38.
// ORIGINAL: 0x004C64F0 ?release@Sound@@QAEHXZ 0x004C64F0-0x004C6502 BYTE_EXACT
// symbol    ?release@Sound@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?release@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x14 when none is wrapped
Status: Complete
*/
int Sound::release() {
    Sound *device = device_;
    int result = 0x14;
    if (device) {
        result = sound_device(device)->release();
    }
    return result;
}


/*
Purpose: Set the sound's volume: the low seven bits are stored at 0x04 and
         the wrapped device, if any, hears them through its vtable slot 0x40.
         Unlike Wave's override there is no group rescaling here.
// ORIGINAL: 0x004C6510 ?set_volume@Sound@@QAEXH@Z 0x004C6510-0x004C6532 BYTE_EXACT
// symbol    ?set_volume@Sound@@UAEXH@Z
// size      34 bytes
// prototype void (__thiscall ?set_volume@Sound@@QAEXH@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C652B
Return Value: n/a
Status: Complete
*/
void Sound::set_volume(int volume) {
    // MASKED IN PLACE, the same shape Wave::set_volume documents: the argument
    // is masked first and the field masks AGAIN - so the image hands the
    // device the once-masked value (eax) and stores the twice-masked one
    // (edx). Masking once into a local lets VC6 merge the two and stores the
    // same register it pushes.
    volume &= 0x7F;
    volume_ = volume & 0x7F;
    if (device_) {
        sound_device(device_)->set_volume(volume);
    }
}

/*
Purpose: Record the loop state at 0x30 and hand it to the wrapped device,
         through vtable slot 0x48. The field is written whether or not a device
         is wrapped.
// ORIGINAL: 0x004C6540 ?set_loop_state@Sound@@QAEXJ@Z 0x004C6540-0x004C655A BYTE_EXACT
// symbol    ?set_loop_state@Sound@@UAEXJ@Z
// size      26 bytes
// prototype void (__thiscall ?set_loop_state@Sound@@QAEXJ@Z)(Sound* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6553
Return Value: n/a
Status: Complete
*/
void Sound::set_loop_state(long loop_state) {
    int value = loop_state;
    loop_flag_30_ = value;
    Sound *device = device_;
    if (device) {
        sound_device(device)->set_loop_state(value);
    }
}

/*
Purpose: Record the delay at 0x34 and hand it to the wrapped device, through
         vtable slot 0x4C. The field is written whether or not a device is
         wrapped.
// ORIGINAL: 0x004C6560 ?set_delay@Sound@@QAEXI@Z 0x004C6560-0x004C657A BYTE_EXACT
// symbol    ?set_delay@Sound@@UAEXI@Z
// size      26 bytes
// prototype void (__thiscall ?set_delay@Sound@@QAEXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6573
Return Value: n/a
Status: Complete
*/
void Sound::set_delay(unsigned int delay) {
    int value = static_cast<int>(delay);
    delay_ = value;
    Sound *device = device_;
    if (device) {
        sound_device(device)->set_delay(static_cast<unsigned int>(value));
    }
}


/*
Purpose: Set the fade time. Zero is refused with 0xA; otherwise the value is
         remembered at 0x38 and the wrapped device hears it through its
         vtable slot 0.
// ORIGINAL: 0x004C6580 ?set_fade@Sound@@QAEHK@Z 0x004C6580-0x004C65A8 BYTE_EXACT
// symbol    ?set_fade@Sound@@UAEHK@Z
// size      40 bytes
// prototype int (__thiscall ?set_fade@Sound@@QAEHK@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C65A0
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade(unsigned long fade) {
    if (!fade) {
        return 0xA;
    }
    fade_38_ = fade;
    if (device_) {
        sound_device(device_)->set_fade(fade);
    }
    return 0;
}


/*
Purpose: Set the fade-in time. Zero is refused with 0xA; otherwise the value
         shares the 0x38 field with set_fade and the wrapped device hears it
         through its vtable slot 0x54.
// ORIGINAL: 0x004C65B0 ?set_fade_in@Sound@@QAEHI@Z 0x004C65B0-0x004C65D9 BYTE_EXACT
// symbol    ?set_fade_in@Sound@@UAEHI@Z
// size      41 bytes
// prototype int (__thiscall ?set_fade_in@Sound@@QAEHI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C65D0
Return Value: 0, or 0xA for a zero time
Status: Complete
*/
int Sound::set_fade_in(unsigned int fade_in) {
    if (!fade_in) {
        return 0xA;
    }
    fade_38_ = fade_in;
    if (device_) {
        sound_device(device_)->set_fade_in(fade_in);
    }
    return 0;
}

/*
Purpose: Ask the wrapped device to fade, through vtable slot 0x28. Refuses
         unless the gate field at 0x38 is set and a device is wrapped.
// ORIGINAL: 0x004C65E0 ?fade@Sound@@QAEHXZ 0x004C65E0-0x004C65F9 BYTE_EXACT
// symbol    ?fade@Sound@@UAEHXZ
// size      25 bytes
// prototype int (__thiscall ?fade@Sound@@QAEHXZ)(Sound* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade() {
    if (fade_38_ == 0) {
        return 0x13;
    }
    Sound *device = device_;
    if (!device) {
        return 0x13;
    }
    return sound_device(device)->fade();
}

// Sound destruction publishes this table at original 0x004C6140. Its
// inherited overload positions are independently consumed by derived tables;
// unfinished placeholder slots still prevent complete-table verification.
// RECOVERY_GENERATED: v1 vtable 0x0066E3C0 0x0066E444 ??_7Sound@@6B@ 0x004C6120

/*
Purpose: Fade the sound out. The work is done by two of the object's own
         virtual methods: slot 0 is asked to fade with the given argument, and
         only when it declines (returns zero) is slot 0x28 run as the fallback.
// ORIGINAL: 0x004C6600 ?fade@Sound@@QAEXK@Z 0x004C6600-0x004C6620 BYTE_EXACT
// symbol    ?fade@Sound@@UAEXK@Z
// size      32 bytes
// prototype void (__thiscall ?fade@Sound@@QAEXK@Z)(Sound* this, unsigned int)
// callers   11   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C660E 0x004C6618
Return Value: n/a
Status: Complete
*/
void Sound::fade(unsigned long a1) {
    // The source-owned virtual APIs occupy original slots 0 and 10.
    // Historical seam spelling used vtable_method to emit the pinned calls;
    // the source calls retain virtual dispatch, with matching still owed.
    if (set_fade(a1) == 0) {
        fade();
    }
}

/*
Purpose: Ask the wrapped device to fade in, through vtable slot 0x30. Carries
         the same pair of guards fade does.
// ORIGINAL: 0x004C6620 ?fade_in@Sound@@QAEHXZ 0x004C6620-0x004C6639 BYTE_EXACT
// symbol    ?fade_in@Sound@@UAEHXZ
// size      25 bytes
// prototype int (__thiscall ?fade_in@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0x13 when either guard fails
Status: Complete
*/
int Sound::fade_in() {
    if (fade_38_ == 0) {
        return 0x13;
    }
    Sound *device = device_;
    if (!device) {
        return 0x13;
    }
    return sound_device(device)->fade_in();
}

/*
Purpose: Hand a three-argument ramp to the wrapped device, through vtable slot
         0x34. Does nothing when no device is wrapped.
// ORIGINAL: 0x004C6640 ?ramp@Sound@@QAEXHHI@Z 0x004C6640-0x004C665F BYTE_EXACT
// symbol    ?ramp@Sound@@UAEXHHI@Z
// size      31 bytes
// prototype void (__thiscall ?ramp@Sound@@QAEXHHI@Z)(Sound* this, int, int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6658
Return Value: n/a
Status: Complete
*/
void Sound::ramp(int a1, int a2, unsigned int a3) {
    // The wrapped device, named: it is this class's own member, not an
    // offset to be walked.
    if (!device_) {
        return;
    }
    sound_device(device_)->ramp(a1, a2, a3);
}


/*
Purpose: Fade the sound in: its own vtable slot 0x54 takes the time, and only
         a zero answer lets its own slot 0x28 follow up.
// ORIGINAL: 0x004C6660 ?fade_in@Sound@@QAEXI@Z 0x004C6660-0x004C6681 BYTE_EXACT
// symbol    ?fade_in@Sound@@UAEXI@Z
// size      33 bytes
// prototype void (__thiscall ?fade_in@Sound@@QAEXI@Z)(Sound* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C666E 0x004C6679
Return Value: n/a
Status: Complete
*/
void Sound::fade_in(unsigned int a1) {
    // Both dispatches go to the slot WHERE IT LIVES - `call [eax+0x54]` and
    // `call [edx+0x28]` - the same shape the byte-exact fade(unsigned long)
    // uses, not the slot-read-then-call the original_slot idiom emits.
    if (set_fade_in(a1) == 0) {
        // Slot 0x28 is this class's own fade() - a REAL virtual call, which
        // reads the vtable into edx exactly as the hand dispatch does.
        fade();
    }
}

/*
Purpose: Ask the wrapped device whether it is playing, through vtable slot 0x5C.
// ORIGINAL: 0x004C64C0 ?is_playing@Sound@@QAEHXZ 0x004C64C0-0x004C64CF FILE BYTE_EXACT
// compile_context src/sound.cpp
// body src/sound.h
// symbol    ?is_playing@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?is_playing@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/

/*
Purpose: Ask the wrapped device whether it is looping, through vtable slot 0x58.
// ORIGINAL: 0x004C6690 ?is_looping@Sound@@QAEHXZ 0x004C6690-0x004C669F BYTE_EXACT
// symbol    ?is_looping@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?is_looping@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::is_looping() {
    if (device_) {
        return sound_device(device_)->is_looping();
    }
    return 0;
}

/*
Purpose: Ask the wrapped device for its play position, through vtable slot 0x74.
// ORIGINAL: 0x004C66A0 ?get_time@Sound@@QAEHXZ 0x004C66A0-0x004C66AF BYTE_EXACT
// symbol    ?get_time@Sound@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?get_time@Sound@@QAEHXZ)(Sound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// VIRTUAL since 2026-08-30: slot 29 of Sound's table (0x0066E3C0) is this
// function in the image, between the pure pair and get_type. Virtualness
// spells U, not Q, in the mangled name - hence the symbol fact. The body is
// unchanged; the claim re-measured BYTE_EXACT under it.
Return Value: the device's answer, or 0 when none is wrapped
Status: Complete
*/
int Sound::get_time() {
    if (device_) {
        return sound_device(device_)->get_time();
    }
    return 0;
}


/*
Purpose: Set the pan, clamped to the range the engine accepts (-0x40 to
         0x3F), stored at 0x08 and handed to the wrapped device through its
         vtable slot 0x44.
// ORIGINAL: 0x004C66B0 ?set_pan@Sound@@QAEXH@Z 0x004C66B0-0x004C66E0 BYTE_EXACT
// symbol    ?set_pan@Sound@@UAEXH@Z
// size      48 bytes
// prototype void (__thiscall ?set_pan@Sound@@QAEXH@Z)(Sound* this, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C66D9
Return Value: n/a
Status: Complete
*/
void Sound::set_pan(int a1) {
    // Written out rather than with std::min/std::max: <windows.h> defines min
    // and max as function-like macros, and VC6's STL spells the templates
    // _cpp_min/_cpp_max, so neither name is usable here.
    const int pan = a1 < -0x40 ? -0x40 : (a1 > 0x3F ? 0x3F : a1);
    pan_8_ = pan;
    if (device_) {
        sound_device(device_)->set_pan(pan);
    }
}

/*
Purpose: Destroy the sound. This is exactly the body ~Wave inlines as its
         base stage: publish the Sound vtable, return the filename copy to
         the game heap (cleared only when there was one), put the wrapped
         device through the guarded release hook and forget it, unlink from
         the sound chain, and publish the ultimate base's vtable on the way
         out through its real SoundFadeState base. The compiler owns the
         exception cleanup; remaining external hooks are not lifetime proof.
// ORIGINAL: 0x004C6120 ??1Sound@@QAE@XZ 0x004C6120-0x004C61D4;0x00659EE2-0x00659EF4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:Sound destructor at 0x004C6120 has main body 0x004C6120..0x004C61D3, base-stage helper 0x004C8450..0x004C8456, and EH continuation 0x00659EE2..0x00659EF3", "the original Sound destructor marker retains BYTE_EXACT; removing the helper span from this mapping does not withdraw or grant that claim", "the helper now has its own original source identity at 0x004C8450 as sub_4c8450"], "reason": "Preserve the historical complete Sound destructor byte claim as a restoration obligation: its former compound span included the independent SoundFadeState unwind entry, now mapped separately."}
// symbol    ??1Sound@@UAE@XZ
// body      src/sound.h
// size      205 bytes
// prototype void (__thiscall ??1Sound@@QAE@XZ)(Sound* this)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C616E
Return Value: n/a
Status: Complete
*/


/*
Purpose: Build the inherited Sound/Fade state once, then initialize Wave's
         own flags, pitch, reverb, timing and group state. Sound::set_type(1)
         is a direct base-method call in the original. The compiler owns
         construction tables and exception cleanup; the historical spelling
         experiments below predate recovery of the fade base.
// ORIGINAL: 0x004C66E0 ??0Wave@@QAE@XZ 0x004C66E0-0x004C67B6;0x00659EF4-0x00659F06 BYTE_EXACT
// TRIED: 3/73 - the image carries an SEH unwind frame here (`push -1 /
//            push 0x659efc / mov eax,fs:[0] / ...`), the same symptom as
//            FlatButton::FlatButton() (flatbutton.cpp) and
//            GraphicWin::~GraphicWin() (graphicwin.cpp): a real
//            constructor calling something VC6 assumes can throw
//            (`Sound::set_type`) alongside a non-trivial `~Wave()`
//            (0x004C67C0, this file). Not attempted: this body already
//            deliberately elides two intermediate vtable-install stages
//            the image performs before the SEH frame, on the documented
//            theory that a CRT memset between them makes both
//            unobservable - reworking the frame risks that already-
//            reasoned simplification for an uncertain payoff, given
//            FlatButton's own SEH-frame recovery needed no vtable-stage
//            changes and this body's does. Left as is.
// size      239 bytes
// prototype void (__thiscall ??0Wave@@QAE@XZ)(Wave* this)
// callers   16   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C6762
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
*/
// LEVER: the image's call list is memset x3 + Sound::set_type - the region
// loop and the two single-dword field clears (flags_40_, flags_54_) are all
// real `call memset` in the image, not stores; writing them as memset() calls
// (matching set_fname's own `memset(&flags_54_, 0, 4)` lever below) reaches
// that count. Store order also follows the image: an early temporary vtable
// write (0x66E444), then the region memset, then a SECOND temporary vtable
// write (0x66E3C0) ahead of the pointer-field clears and the flags_40_
// memset, then the FINAL vtable write (0x66E44C) ahead of the flags_54_
// memset.
Wave::Wave() {
    // Sound and SoundFadeState construct their own fields exactly once.
    // The original's `memset(this + 0x54, 0, 4)` at 0x004C6774 - same lever
    // as set_fname's own `memset(&flags_54_, 0, 4)` below.
    memset(&flags_54_, 0, sizeof(flags_54_));
    flags_40_ |= 4;
    Sound::set_type(1);
    pitch_ = 0;
    reverb_mix_ = 1.0f;
    group_slot_ = 0x10;
    ms_length_ = 0;
    start_time_ = 0;
}

// The destructor's dependencies. pull_from_group is the Wave_Device method at
// 0x004C5280 with its singleton receiver at 0x0090D978; the buffer free goes
// to the game CRT's operator delete so the block returns to the heap that
// allocated it; the release hook is an indirect call on the slot at 0x0090DB28
// guarded by the dword at 0x0090DB7C; the chain end slots are the dwords the
// unlink falls back to when a neighbour is null.

/*
Purpose: Destroy the wave. The original is a three-stage teardown of an
         inlined hierarchy, republishing a vtable pointer at each stage: its
         own (0x0066E44C) at entry, the base's (0x0066E3C0) midway, and the
         ultimate base's (0x0066E444) on the way out. While the wave still
         holds a device group slot it is pulled from its group; the filename
         copy goes back to the game CRT heap; a chained wave unlinks itself
         from the global wave chain; the wrapped device is put through the
         release hook when the hook is live, then forgotten. The inlined base
         destructor owns the later filename/device/chain cleanup. The real
         Sound base performs that stage once, with Sound virtual dispatch
         already active. The compiler owns the exception cleanup and final
         SoundFadeState destruction; do not duplicate those stages here.
// ORIGINAL: 0x004C67C0 ??1Wave@@QAE@XZ 0x004C67C0-0x004C68EC;0x00659F06-0x00659F20 BYTE_EXACT
// symbol    ??1Wave@@UAE@XZ
// LEVER: plain member access, not the `Wave volatile *const self` view. The
//        view predates the real `Sound` base: once the SEH frame is in the
//        body for real, VC6 keeps every store anyway, and the volatile only
//        blocked the hoist of the `flags_40_` read above the two null
//        neighbour stores. 49/101 -> 58/101, instruction count 101/101.
// Historical WALL (superseded by the co-located real lifetimes below): the ONLY four instructions left are the
//        base-destructor stage - the image republishes the mid vtable
//        (0x0066E3C0) at instruction 52, moves the SEH state to 1 at 54, and
//        ends `mov [esi], 0x66e444`; this tree emits `mov ecx,esi /
//        mov [ebp-4],-1 / call` - the OUT-OF-LINE base destructor call the
//        `#pragma auto_inline(off)` below exists to keep. Inlining the base
//        stage would break ~Effect's five-byte tail jump (see that marker and
//        the ~Sound stale-TU note there). Not re-derived here.
// TRIED: 0.888 similar, 2/101 agreeing - already a real destructor,
//            and everything BUT the SEH prologue/epilogue is close; the
//            gap is the missing unwind frame, matching Wave::Wave()'s own
//            ceiling above (same file). The comment above claims the
//            frame is "unreachable, so omitted", which is the SAME
//            reasoning that write_l's own font1_ rebind note (buffer.cpp)
//            turned out to be WRONG about elsewhere in this pass -
//            flagged here rather than trusted, but not re-investigated:
//            unlike FlatButton's destructor (flatbutton.cpp, where the
//            fix was straightforward - just stop routing through a
//            `destroy()` method), this is ALREADY a real destructor, has
//            no base class to call into, and is wrapped in heavy
//            `volatile` reads/writes throughout; reproducing the frame
//            here would need isolating which of those is responsible,
//            which this budget does not cover.
// size      333 bytes
// prototype
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5280 0x0064557F
// indirect  0x004C6886
Return Value: n/a
Status: Complete
*/
Wave::~Wave() {
    // Original 0x004C6856 changes to the Sound table before the release
    // callback at 0x004C6886. Only the Wave stage belongs in this body;
    // the automatic base destructor owns that transition and cleanup.
    if (group_slot_ < 0x10) {
        WaveDeviceGlobal.pull_from_group(this);
    }
    char *const block = fname_;
    if (block) {
        operator delete(block);
    }
    fname_ = nullptr;
    if (flags_40_ & 2) {
        // Unlink from the wave chain. The second neighbour is re-read after
        // the first write because a neighbour pointer may alias this very
        // object - the original re-reads it the same way.
        Sound *const prev = chain_prev_;
        if (prev) {
            prev->chain_next_ = chain_next_;
        } else {
            SoundChainHead = chain_next_;
        }
        Sound *const next = chain_next_;
        if (next) {
            next->chain_prev_ = chain_prev_;
        } else {
            SoundChainTail = chain_prev_;
        }
        chain_next_ = nullptr;
        chain_prev_ = nullptr;
        flags_40_ &= ~2u;
    }
}


/*
Purpose: Start the wave playing. A wrapped device answers through its own
         vtable slot 0x94, as the receiver, with the play argument passed on;
         its result is returned verbatim. With no device the answer is a
         fixed 0x14.
// ORIGINAL: 0x004C68F0 ?play@Wave@@QAEHH@Z 0x004C68F0-0x004C6913 BYTE_EXACT
// symbol    ?play@Wave@@UAEHH@Z
// size      35 bytes
// prototype int (__thiscall ?play@Wave@@QAEHH@Z)(Wave* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6900
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::play(int effect) {
    if (device_) {
        return wave_playback_device(device_)->play(effect);
    }
    return 0x14;
}


/*
Purpose: Start the wave. While it holds a device group slot, a disabled group
         answers 0x14 immediately (the original trusts only the answer's low
         byte); otherwise, when bit 4 of the 0x54 flag dword marks a clocked
         wave, the wave's own vtable slot 0x40 replays the stored volume. A
         wrapped device then starts through its slot 0x1C and its answer is
         the result; with no device, a clocked wave runs the original
         no-argument load and starts the device that load may have wrapped
         (answering 0 when it wrapped none), while an unclocked one answers
         0x14. Finally a clocked wave stamps the start time from the game's
         timeGetTime import, runs its own vtable slot 0x80, and forgets the
         device.
// ORIGINAL: 0x004C6920 ?play@Wave@@QAEHXZ 0x004C6920-0x004C69AD BYTE_EXACT
// symbol    ?play@Wave@@UAEHXZ
// TRIED: 52/54 MNEMONIC_ONLY; sole divergence is the vtable-pointer
//   register (edx vs eax) at the FIRST `vtable_slot<device_start_fn>(device_,
//   0x1C)(device_)` call site only - the second, identical call already
//   matches. Tried: a named `device_start_fn fn = vtable_slot<...>(...)`
//   temp at the first site only (worse, 26/54); restructuring the
//   device_/flags_54_ branch as `if (!device_) {...} else {...}` with a
//   ternary on the reload path (worse, 2/54). Plain register-allocation
//   divergence between two textually-identical call expressions; no source
//   reshaping tried closed it.
// size      141 bytes
// prototype int (__thiscall ?play@Wave@@QAEHXZ)(Wave* this)
// callers   20   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C5460 0x004C6CE0
// indirect  0x004C6953 0x004C695F 0x004C697C 0x004C698E 0x004C699B
Return Value: the device's start answer, 0x14 when disabled or unstartable,
              0 when a clocked load wrapped no device
Status: Complete
*/
int Wave::play() {

    int result = 0;
    if (group_slot_ < 0x10) {
        if (static_cast<uint8_t>(
                WaveDeviceGlobal.is_group_disabled(group_slot_))) {
            return 0x14;
        }
        if (flags_54_ & 0x10) {
            set_volume(volume_);
        }
    }
    if (device_) {
        result = wave_playback_device(device_)->play();
    } else if (flags_54_ & 0x10) {
        Wave::load();
        if (device_) {
            result = wave_playback_device(device_)->play();
        }
    } else {
        result = 0x14;
    }
    if (flags_54_ & 0x10) {
        start_time_ = timeGetTime();
        detach();
        device_ = nullptr;
    }
    return result;
}


/*
Purpose: Initialise the wave from a filename and a mode mask. Streaming waves
         (bit 2) refuse the bit-4 and bit-7 modes outright. The name resolves
         through the recovered filefind_get; an unresolvable name changes
         nothing. The resolved path replaces the remembered filename on the
         game heap and the 0x54 flag dword is cleared. A streaming wave marks
         flag bit 1 and, with no device yet and a live creation hook, builds
         one from the resolved path - the wave's slot 0x70 composes an
         attribute mask and the typed provider setter applies it, like
         dyna_load - then hands the RAW mode mask to whatever device exists
         through its typed setter. The mode bits then fold onto the flag dword
         as in set_attrib
         (bit 4 suppressed for streaming waves, bit 8 unsuppressed here), and
         bit 1 of the mode runs the wave's own vtable slot 0x48 with 1.
// ORIGINAL: 0x004C69B0 ?init@Wave@@QAEXPADK@Z 0x004C69B0-0x004C6AD5 BYTE_EXACT
// symbol    ?init@Wave@@UAEXPADK@Z
// TRIED: best flag set /c /O2 /Oi- /Gy /GR- /Oy- /GX reaches 13/122
//   (0.975 similar); `listing_diff` at those flags shows the structure and
//   control flow already match - 23 differing runs, nearly all the SAME
//   global register swap: the image keeps `this` in edi and the resolved
//   filename / later `&flags_54_` pointer in esi (reused once the filename
//   is done with), while this tree keeps `this` in esi and the other value
//   in edi throughout. Tried reassigning the resolved path back onto the
//   `a1` parameter slot itself (`a1 = filefind_get(a1);`, matching the
//   image's `mov [ebp+8], esi` reuse of the a1 stack slot) instead of a
//   separate `resolved` local - byte-identical output, no effect on the
//   allocator's choice. Also present: the image issues the `a2 & 0x10` and
//   `a2 & 0x80` streaming guards as two SEPARATE `test al, N` probes re-
//   testing `streaming` each time, where this tree's compiled output merges
//   them into one `test al, 0x90`, saving 4 instructions (118 vs image's
//   122) - a legal optimization of the identical-`return;` arms that no
//   restructuring tried suppressed.
// size      293 bytes
// prototype void (__thiscall ?init@Wave@@QAEXPADK@Z)(Wave* this, int8*, unsigned int)
// callers   5   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006005D0 0x006453E0 0x00645460 0x0064557F 0x0064558A 0x006465F0
// indirect  0x004C6A4F 0x004C6A64 0x004C6A70 0x004C6A77 0x004C6A88 0x004C6AAD
Return Value: n/a
Status: Complete
*/
void Wave::init(char *group_id, unsigned long flags) {
    const uint32_t streaming = flags & 4;
    if (wave_streaming_mode(flags) && (flags & 0x10)) {
        return;
    }
    if (wave_streaming_mode(flags) && (flags & 0x80)) {
        return;
    }
    char *const resolved = filefind_get(group_id);
    if (!resolved) {
        return;
    }
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = static_cast<char *>(operator new(strlen(resolved) + 1));
    strcpy(fname_, resolved);
    // The original's `memset(this + 0x54, 0, 4)` call is at 0x004C6A22
    // (its argument setup starts at 0x004C6A1A). This used to be
    // a byte loop, the only way to spell a four-byte clear while the header
    // declared a byte and three pad bytes at 0x54.
    // `memset(&flags_54_, 0, 4)`, not a store. The image clears it that
    // way - `lea esi, [edi+0x54]; push 4; push 0; push esi; call` - and
    // keeps `&flags_54_` in esi afterwards, so every later flag update is
    // one `or dword ptr [esi], imm`. A plain store costs three
    // instructions at each of those six sites.
    memset(&flags_54_, 0, 4);
    if (streaming) {
        flags_54_ |= 2;
        if (!device_ && WaveDeviceReleaseGuard) {
            const int created = (SoundXOrdinal1CreateSound())(&device_, resolved, 1);
            if (!created) {
                device_->set_attrib(
                    static_cast<unsigned long>(get_attrib()));
                attach();
            }
        }
        if (device_) {
            wave_playback_device(device_)->set_attrib(flags);
        }
    }
    if (flags & 1) {
        flags_54_ |= 1;
    }
    if ((flags & 0x10) && !streaming) {
        flags_54_ |= 4;
    }
    if (flags & 2) {
        set_loop_state(1);
    }
    if (flags & 0x40) {
        flags_54_ |= 8;
    }
    if (flags & 0x80) {
        flags_54_ |= 0x10;
    }
    if (flags & 0x100) {
        flags_54_ |= 0x20;
    }
}


// The wrapped-device forwarder family: each method lets the device at 0x3C
// answer through its own vtable slot, as the receiver, and returns a fixed
// answer when no device is wrapped. Only the slot, the argument list, and the
// no-device default vary.

/*
Purpose: Ask the wrapped device, through its vtable slot 0xC8, whether the
         buffer is in hardware.
// ORIGINAL: 0x004C6AE0 ?is_hwbuffer@Wave@@QAEHXZ 0x004C6AE0-0x004C6AF2 BYTE_EXACT
// symbol    ?is_hwbuffer@Wave@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?is_hwbuffer@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::is_hwbuffer() {
    if (device_) {
        return wave_playback_device(device_)->is_hwbuffer();
    }
    return 0;
}


/*
Purpose: Report the wave's length in milliseconds, a plain read of the field
         at 0x60.
// ORIGINAL: 0x004C6B00 ?get_ms_length@Wave@@QAEHXZ 0x004C6B00-0x004C6B04 BYTE_EXACT
// symbol    ?get_ms_length@Wave@@UAEHXZ
// size      4 bytes
// prototype int (__thiscall ?get_ms_length@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the stored length
Status: Complete
*/
int Wave::get_ms_length() {
    return ms_length_;
}


/*
Purpose: Report whether the wave is still sounding. A wrapped device answers
         for itself through its own vtable slot 0x5C; with no device the wave
         is timed against the clock instead.
// ORIGINAL: 0x004C6B10 ?is_playing@Wave@@QAEHXZ 0x004C6B10-0x004C6B53 BYTE_EXACT
// symbol    ?is_playing@Wave@@UAEHXZ
// size      67 bytes
// prototype int (__thiscall ?is_playing@Wave@@QAEHXZ)(Wave* this)
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6B2B 0x004C6B4A
// TRIED: inverting the guard so the CLOCK path is the fall-through and
//   the device dispatch is the merged tail - which is the shape the image's
//   forward `jne` suggests - scores WORSE, 2 of 30 against the current 4 of
//   30, and compiles 28 instructions against the image's 30. Spelling the
//   flag test as `(flags_54_ >> 4) & 1` and returning a literal 1 instead of
//   the ternary was part of the same attempt and did not rescue it. The
//   branch-polarity lever is real elsewhere in this tree; it does not apply
//   here.
// TRIED 2026-09-03: nesting the whole clock path inside
//   `if (device_ == nullptr)` with the dispatch as the fall-through return -
//   the shape the image's `jne 0x4c6b48` to an end-of-body dispatch suggests -
//   compiles 26 instructions against the image's 30 and still 4/30, with and
//   without the `(flags_54_ >> 4) & 1` spelling for the clocked test. The
//   committed flat spelling keeps the most instructions agreeing; leave it.
Return Value: nonzero while playing, 0 once finished or when not started
Status: Complete
*/
int Wave::is_playing() {
    Sound *const device = device_;
    // `mov eax, dword ptr [esi+0x54]` then `shr eax, 4` / `test al, 1` at
    // 0x004C6B1A: a dword read of the whole flag field, testing bit 4.
    unsigned char release_flags;
    if (device == 0 && ((release_flags = flags_54_ >> 4) & 1)) {
        // `cmp ecx, eax` / `jbe`: the length is compared unsigned, so a negative
        // stored length reads as a very long one rather than as already finished.
        if (start_time_ != 0 &&
            static_cast<uint32_t>(ms_length_) > timeGetTime() - start_time_) {
            return 1;
        }
        return 0;
    }
    // This is an external virtual call through the Sound prefix's slot 23.
    return Sound::is_playing();
}


/*
Purpose: Remember the wave's filename. The previous copy, if any, goes back
         to the game CRT heap first; the new string is measured, allocated on
         that same heap, recorded at 0x4C, and copied in - in that order, so
         the field already names the block while it is still being filled.
// ORIGINAL: 0x004C6B60 ?set_fname@Wave@@QAEHPBD@Z 0x004C6B60-0x004C6BAB BYTE_EXACT
// size      75 bytes
// prototype int (__thiscall ?set_fname@Wave@@QAEHPBD@Z)(Wave* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// LEVER: branch polarity - `if (fname) { work; return 0; } return 0xA;`,
//        NOT the guard clause `if (!fname) return 0xA;`. The guard clause
//        compiles `test edi,edi; jne <work>` with the 0xA return INLINE at the
//        top (11/34); the positive form compiles the image's `je` to a shared
//        0xA tail at the END of the body. 11/34 -> BYTE_EXACT 34/34.
Return Value: 0, or 0xA when the name is null
Status: Complete
*/
int Wave::set_fname(const char *fname) {
    if (fname) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = static_cast<char *>(operator new(strlen(fname) + 1));
        strcpy(fname_, fname);
        return 0;
    }
    return 0xA;
}


/*
Purpose: Load the wave from in-memory data. Refuses (0xC) when a device is
         already wrapped; otherwise the guarded creation hook builds one from
         the data pointer. The wave's own slot 0x70 composes the attribute
         mask, which is sent to the created provider through its typed setter;
         attach follows that setter.
// ORIGINAL: 0x004C6BB0 ?dyna_load@Wave@@QAEHPAD@Z 0x004C6BB0-0x004C6C1A
// size      106 bytes
// prototype int (__thiscall ?dyna_load@Wave@@QAEHPAD@Z)(Wave* this, int8*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6BDC 0x004C6BF5 0x004C6BFB 0x004C6C02
// LEVER: the work inside the positive `if (device_ == nullptr)` branch, the
//        dead hook as an early `return 1` INSIDE it, and a guard clause on
//        `created`. Three effects, all measured: the 0xC exit becomes a tail
//        at the END of the body instead of an inline return at the top; the
//        `return 1` stays inline after the guard test; and `created` - live
//        past its own block instead of returned early - spills into the dead
//        parameter slot at [ebp+8] and is re-read for the final return, which
//        is exactly the image's `mov [ebp+8],eax` / `mov eax,[ebp+8]` pair.
//        9/49 -> 42/49.
// TRIED: the same body with the hook spelled `if (WaveDeviceReleaseGuard) {
//        ...create... } return 1;` - the 1-exit moved to a tail at the end and
//        the agreement dropped to 17/49.
// TRIED: the two remaining runs are the register swap on the 0x70/0x7C
//        dispatches (image `edx` then `eax`, this tree `eax` then `edx`) - the
//        same allocator plateau reload and load(char *) carry.
Return Value: 0 on success, 0xC with a device already wrapped, 1 with a dead
              creation hook, or the creation error
Status: Complete
*/
int Wave::dyna_load(char *fname) {
    // THE WORK IN THE POSITIVE BRANCH, and the guard clause on `created` - the
    // two shapes reload and set_fname record. The image's 0xC exit is a shared
    // tail at the END of the body, not an inline return at the top, and its
    // `created` is spilled to the dead parameter slot at [ebp+8] and re-read
    // for the final return - which only happens when the value stays live past
    // the block instead of being returned early on the nonzero path.
    if (device_ == nullptr) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (SoundXOrdinal1CreateSound())(&device_, fname, 1);
        if (created == 0) {
            device_->set_attrib(
                static_cast<unsigned long>(get_attrib()));
            attach();
        }
        return created;
    }
    return 0xC;
}


/*
Purpose: Load the wave from a caller-supplied filename. The guarded creation
         hook builds a device from that name when none is wrapped; the folded
         attribute mask (bits 0 and 1 plus the 0x40/0x80 mappings, without
         the companions the no-argument load adds) goes through the device's
         slot 0x6C; the base Sound::load reads the file; and on success the
         device answers the length through slot 0xC4 and then hears the
         stored volume, pitch, and the dword at 0x08 through its slots
         0x40, 0x98, and 0x44.
// ORIGINAL: 0x004C6C20 ?load@Wave@@QAEHPBD@Z 0x004C6C20-0x004C6CD8 BYTE_EXACT
// symbol    ?load@Wave@@UAEHPBD@Z
// LEVER: `int attribs = 0;` declared BEFORE the device-create block (not
//        after) - matches the image's `xor ebx, ebx` at function entry,
//        which is reused as the attribs accumulator only after the
//        create/guard logic. Moved 22/79 to 57/79 MISMATCH.
// TRIED: hoisting the 0xC4 length read into a named local and deferring
//            the `ms_length_ =` store past the volume dispatch, to match the
//            image's `mov edx,[esi+4]; mov [esi+0x60],eax` order - no effect
//            on codegen (still 57/79), so reverted; that ordering looks like
//            pure instruction scheduling, not source order.
// size      184 bytes
// prototype int (__thiscall ?load@Wave@@QAEHPBD@Z)(Wave* this, int8*)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C6C50 0x004C6C6C 0x004C6C8C 0x004C6CA2 0x004C6CB3 0x004C6CBE 0x004C6CCC
Return Value: 0 on success, 1 with a dead creation hook, or the
              creation/load error
Status: Complete
*/
int Wave::load(const char *fname) {
    int attribs = 0;
    if (!device_) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        const int created = (SoundXOrdinal1CreateSound())(&device_, fname, 1);
        if (created) {
            return created;
        }
    }
    if (flags_54_ & 1) {
        attribs |= 1;
    }
    if (is_looping()) {
        attribs |= 2;
    }
    const uint32_t flags = flags_54_;
    if (flags & 8) {
        attribs |= 0x40;
    }
    if (flags & 0x10) {
        attribs |= 0x80;
    }
    wave_playback_device(device_)->set_attrib(static_cast<unsigned long>(attribs));
    // The original calls Sound::load (0x004C6280) directly, not this
    // Wave overload. Qualification preserves the non-recursive base call.
    const int loaded = Sound::load(fname);
    if (loaded) {
        return loaded;
    }
    ms_length_ = wave_playback_device(device_)->get_ms_length();
    wave_playback_device(device_)->set_volume(volume_);
    wave_playback_device(device_)->set_pitch(pitch_);
    wave_playback_device(device_)->set_pan(static_cast<int>(pan_8_));
    return 0;
}


/*
Purpose: Load the wave from its remembered filename. With no wrapped device
         yet, the creation hook - the slot beside the release hook, behind
         the same guard - builds one directly into the 0x3C field; a dead
         hook answers 1 and a failed creation its own error. The 0x54 flag
         dword and the wave's own vtable slot 0x58 fold into an attribute mask
         the device hears through its slot 0x6C, the base Sound::load reads
         the file, and on success the device reports the length in
         milliseconds through its slot 0xC4 into the field at 0x60.
// ORIGINAL: 0x004C6CE0 ?load@Wave@@QAEHXZ 0x004C6CE0-0x004C6DAC BYTE_EXACT
// symbol    ?load@Wave@@UAEHXZ
// LEVER: `int attribs = 0;` declared as the FIRST statement, before even the
//        `fname` null check - matches the image's `xor edi, edi` right after
//        the prologue's `push edi`, which forces edi to be pushed/popped
//        uniformly across every exit path. 7/88 -> 9/88 MISMATCH.
// TRIED: declaring `attribs` after the fname check but before the
//            device check (partway) - identical to the original placement,
//            no change (7/88); the image's shared tail between "no fname"
//            and "load failed" needs edi live from function ENTRY, not just
//            before the device branch.
// LEVER: the no-filename path spelled `if (fname != nullptr) { ...whole
//        body... } return 8;` - the same positive-branch shape reload and
//        set_fname record. The image jumps to a shared 8 tail at the END;
//        the guard clause inlined it at the top. 9/88 -> 44/88.
// TRIED: swapping the two declaration lines (fname before attribs) - 44/88
//        either way. The remaining 40+ instructions are ONE register-role
//        swap this tree cannot seem to flip: the image holds the attribute
//        accumulator in `edi` and `&device_` in `ebx` (`lea ebx,[esi+0x3c]`),
//        this tree holds them the other way round; reload() is the mirror
//        image and already agrees. Pure allocator choice, no source shape
//        tried moves it.
// size      204 bytes
// prototype int (__thiscall ?load@Wave@@QAEHXZ)(Wave* this)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C6D1D 0x004C6D39 0x004C6D74 0x004C6D8C
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/load error
Status: Complete
*/
int Wave::load() {
    int attribs = 0;
    int result = 0;
    const char *const fname = fname_;
    if (fname != nullptr) {
        if (!device_) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = (SoundXOrdinal1CreateSound())(&device_, fname, 1);
            if (created) {
                return created;
            }
        }
        if (flags_54_ & 1) {
            attribs |= 1;
        }
        if (is_looping()) {
            attribs |= 2;
        }
        const uint32_t flags = flags_54_;
        if (flags & 4) {
            attribs |= 0x11;
        }
        if (flags & 8) {
            attribs |= 0x40;
            if (!(flags & 0x20)) {
                attribs |= 1;
            }
        }
        if (flags & 0x10) {
            attribs |= 0x80;
        }
        if (flags & 0x20) {
            attribs |= 0x100;
        }
        wave_playback_device(device_)->set_attrib(static_cast<unsigned long>(attribs));
        // The original calls Sound::load (0x004C6280) directly. Virtual
        // dispatch here would recurse into Wave's filename overload.
        result = Sound::load(fname);
        if (result == 0) {
            ms_length_ = wave_playback_device(device_)->get_ms_length();
        }
    } else {
        result = 8;
    }
    return result;
}

/*
Purpose: Load a wave. The object's own vtable slot 0x88 does the loading; unless
         bit 2 of the second argument says otherwise, its own slot 0x8C then
         runs as the follow-up and supplies the result.
// ORIGINAL: 0x004C6DB0 ?load@Wave@@QAEHPADK@Z 0x004C6DB0-0x004C6DE6 BYTE_EXACT
// symbol    ?load@Wave@@UAEHPADK@Z
// size      54 bytes
// prototype int (__thiscall ?load@Wave@@QAEHPADK@Z)(Wave* this, int8*, unsigned int)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6DC3 0x004C6DDA
// TRIED: the sole remaining divergence at best flags (/O2 /Gy /GR- /Oy-
//   /GX, 24/26, 1.000 similar, MNEMONIC_ONLY) is `mov edx,[esi]`/
//   `call [edx+0x8c]` in the image against this tree's `eax` for the same
//   pointer - the same register-choice gap `Wave::unload` has on its own
//   trailing `vtable_slot` dispatch, where reordering statements and binding
//   `this` into a named local did not move it. Not re-tried here on that
//   evidence.
Return Value: slot 0x8C's result, or 0 when bit 2 of a2 skipped it
Status: Complete
*/
int Wave::load(char *a1, unsigned long a2) {
    init(a1, a2);
    if (a2 & 4) {
        return 0;
    }
    return load();
}


/*
Purpose: Reload the wave from its remembered filename. The device is created
         through the guarded hook when missing, hears the folded attribute
         mask (only the low two bits here) through its slot 0x6C, and
         reloads through its slot 0x84. On a first successful load the
         loaded bit of the 0x40 flag dword is set BEFORE the wave's own
         vtable slot 0x7C runs, and a nonzero dword at 0x30 additionally
         starts the device looping through its slot 0x48 with argument 1.
// ORIGINAL: 0x004C6DF0 ?reload@Wave@@QAEHXZ 0x004C6DF0-0x004C6E95 BYTE_EXACT
// symbol    ?reload@Wave@@UAEHXZ
// size      165 bytes
// prototype int (__thiscall ?reload@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6E23 0x004C6E3F 0x004C6E4E 0x004C6E55 0x004C6E71 0x004C6E81
// LEVER: attribs first (image `xor ebx,ebx` at entry), ONE result cell for the
//        whole body instead of a `return` per exit. The image keeps it in ebx
//        from `mov ebx, 8` on the no-filename path through the `mov eax, ebx`
//        all five exits share; per-exit `return`s give VC6 a provably-constant
//        final return and split the tail in two. 1/72 -> 70/72 MNEMONIC_ONLY.
// TRIED: the 0x6C attribute dispatch spelled `__fastcall(void *, int)` rather
//        than the pointer-to-member form - worse, 67/72. The sole divergence
//        left is the same register-choice gap unload and load(char *) carry:
//        image `mov edx,[ecx]`/`call [edx+0x6c]`, this tree `eax`.
Return Value: 0 on success, 8 with no filename, 1 with a dead creation hook,
              or the creation/reload error
Status: Complete
*/
int Wave::reload() {
    // FIRST, before the fname check - the image's `xor ebx,ebx` at entry, the
    // same lever `load(const char *)` records below: the accumulator has to be
    // live from function entry for the shared tails to line up.
    int attribs = 0;
    // ONE result cell for the whole body. The image keeps it in ebx from the
    // `mov ebx, 8` on the no-filename path through the `mov eax, ebx` every
    // exit shares - spelling each exit its own `return` breaks that tail into
    // two epilogues.
    int result = 0;
    const char *const fname = fname_;
    if (fname != nullptr) {
        if (!device_) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = (SoundXOrdinal1CreateSound())(&device_, fname, 1);
            if (created) {
                return created;
            }
        }
        if (flags_54_ & 1) {
            attribs |= 1;
        }
        if (is_looping()) {
            attribs |= 2;
        }
        wave_playback_device(device_)->set_attrib(static_cast<unsigned long>(attribs));
        result = wave_playback_device(device_)->reload();
        // The image's every exit - the reload error, the already-loaded bit,
        // the loop-flag jump and this fall-through - merges into ONE tail.
        // `if (result) return result;` lets VC6 prove the final return is a
        // constant zero and the shared tail disappears.
        if (result == 0) {
            if (!(flags_40_ & 1)) {
                flags_40_ |= 1;
                attach();
                if (loop_flag_30_) {
                    wave_playback_device(device_)->set_loop_state(1);
                }
            }
        }
    } else {
        result = 8;
    }
    return result;
}


/*
Purpose: Release the loaded wave. The wrapped device, if there is one, is asked
         to unload through its own vtable slot 0x14 and its result becomes the
         return value; the device is then forgotten. Unless bit 1 of the flag
         dword at 0x54 suppresses it, the object's own vtable slot 0x80 is run,
         and the loaded bit (bit 0) of the flag dword at 0x40 is cleared.
// ORIGINAL: 0x004C6EA0 ?unload@Wave@@QAEXXZ 0x004C6EA0-0x004C6ED9 BYTE_EXACT
// symbol    ?unload@Wave@@UAEHXZ
// size      57 bytes
// prototype void (__thiscall ?unload@Wave@@QAEXXZ)(Wave* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6EAF 0x004C6EC6
// TRIED: reordering `flags_40_ &= ...` ahead of the trailing vtable
//   dispatch, and binding `this` into a named local first, both tried against
//   the sole remaining divergence (`mov edx,[esi]`/`call [edx+0x80]` in the
//   image vs this tree's `eax` for the same pointer) - neither changes the
//   register VC6 picks. 22/24 instructions, 1.000 similar, MNEMONIC_ONLY;
//   this is the ceiling found so far.
Return Value: whatever the device's unload returned, or 0 when none was wrapped
Status: Complete
*/
int Wave::unload() {
    // The wrapped device still has an opaque external interface; our own
    // chain operation uses the inherited Sound virtual contract.
    int result = 0;
    if (device_) {
        // The original call is through provider slot 5; this uses the typed
        // Sound prefix and leaves emitted-call matching to native review.
        result = wave_playback_device(device_)->unload();
    }
    // `mov al, byte ptr [esi+0x54]` at 0x004C6EB4: unload is the one place
    // that narrows the 0x54 dword, and only bit 1 is wanted.
    const uint8_t flags = static_cast<uint8_t>(flags_54_);
    device_ = nullptr;
    if (!(flags & 2)) {
        detach();
    }
    flags_40_ &= 0xFFFFFFFEu;
    return result;
}


/*
Purpose: Set the playback pitch, clamped to the range the engine accepts
         (-1200 to 1200). The clamped value is stored at 0x58 and handed to the
         wrapped device, if there is one, through its vtable slot 0x98.
// ORIGINAL: 0x004C6EE0 ?set_pitch@Wave@@QAEXH@Z 0x004C6EE0-0x004C6F19 BYTE_EXACT
// symbol    ?set_pitch@Wave@@UAEHH@Z
// LEVER: returns-int - same as Wave_Device::release/enable/disable. The
//        image's shared tail does `xor eax, eax` before `pop ebp; ret 4`,
//        the int-returning epilogue; a void body tail-jumps instead. The
//        catalogued name spells `X` (void) but the body is int-shaped, so
//        the return type changes to `int` and the `// symbol` fact carries
//        the int-mangled name the build emits. 17/20 -> 20/20.
// size      57 bytes
// prototype void (__thiscall ?set_pitch@Wave@@QAEXH@Z)(Wave* this, int)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F0D
Return Value: n/a
Status: Complete
*/
int Wave::set_pitch(int a1) {
    int pitch = a1;
    if (pitch < -1200) {
        pitch = -1200;
    } else if (pitch > 1200) {
        pitch = 1200;
    }
    pitch_ = pitch;
    if (device_) {
        // The typed external call keeps the signed, clamped value in the
        // provider's declared argument type.
        wave_playback_device(device_)->set_pitch(pitch);
    }
    return 0;
}


/*
Purpose: Store the attribute mask into the wave's own fields, then tell the
         wrapped device through its vtable slot 0x6C. Bit 1 of the mask sets
         the dword at 0x30; the other bits map onto the flag dword at 0x54
         (bit 0 -> 1, bit 2 -> 2, bit 6 -> 8, bit 7 -> 0x10, and - only when
         bit 2 is clear - bit 4 -> 4 and bit 8 -> 0x20). Bits already set at
         0x54 are never cleared.
// ORIGINAL: 0x004C6F20 ?set_attrib@Wave@@QAEXK@Z 0x004C6F20-0x004C6F80 BYTE_EXACT
// symbol    ?set_attrib@Wave@@UAEHK@Z
// LEVER: returns-int - same as set_pitch / Wave_Device::release. The image's
//        epilogue is the int-returning `xor eax, eax` shared tail; a void body
//        tail-jumps instead. Return type becomes `int`, the `// symbol` fact
//        carries the int-mangled name the build emits.
// size      96 bytes
// prototype void (__thiscall ?set_attrib@Wave@@QAEXK@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F77
Return Value: n/a
Status: Complete
*/
int Wave::set_attrib(unsigned long attrib) {
    if (attrib & 2) {
        loop_flag_30_ = 1;
    }
    if (attrib & 1) {
        flags_54_ |= 1;
    }
    if (attrib & 4) {
        flags_54_ |= 2;
    }
    if (attrib & 0x40) {
        flags_54_ |= 8;
    }
    if (attrib & 0x80) {
        flags_54_ |= 0x10;
    }
    if (!(attrib & 4) && (attrib & 0x10)) {
        flags_54_ |= 4;
    }
    if (!(attrib & 4) && (attrib & 0x100)) {
        flags_54_ |= 0x20;
    }
    if (device_) {
        wave_playback_device(device_)->set_attrib(attrib);
    }
    return 0;
}


/*
Purpose: Compose the attribute mask back out of the wave's own fields, OR-ed
         over whatever the wrapped device answers through its vtable
         slot 0x70 (0 with no device). The mapping inverts set_attrib's:
         the dword at 0x30 -> bit 1, and the 0x54 flag dword's bits
         1 -> 0, 8 -> 6, 2 -> 2, 4 -> 4, 0x10 -> 7, 0x20 -> 8.
// ORIGINAL: 0x004C6F80 ?get_attrib@Wave@@QAEHXZ 0x004C6F80-0x004C6FCA BYTE_EXACT
// symbol    ?get_attrib@Wave@@UAEHXZ
// size      74 bytes
// prototype int (__thiscall ?get_attrib@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6F8E
Return Value: the composed attribute mask
Status: Complete
*/
int Wave::get_attrib() {
    int result = 0;
    if (device_) {
        result = wave_playback_device(device_)->get_attrib();
    }
    if (loop_flag_30_) {
        result |= 2;
    }
    const uint32_t flags = flags_54_;
    if (flags & 1) {
        result |= 1;
    }
    if (flags & 8) {
        result |= 0x40;
    }
    if (flags & 2) {
        result |= 4;
    }
    if (flags & 4) {
        result |= 0x10;
    }
    if (flags & 0x10) {
        result |= 0x80;
    }
    if (flags & 0x20) {
        result |= 0x100;
    }
    return result;
}


/*
Purpose: Ask the wrapped device for a time value through its vtable slot 0xB4,
         passing the query argument on.
// ORIGINAL: 0x004C6FD0 ?get_time@Wave@@QAEHI@Z 0x004C6FD0-0x004C6FF0 BYTE_EXACT
// symbol    ?get_time@Wave@@UAEHI@Z
// size      32 bytes
// prototype int (__thiscall ?get_time@Wave@@QAEHI@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6FE0
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_time(uint32_t a1) {
    if (device_) {
        return wave_playback_device(device_)->get_time(a1);
    }
    return 0;
}


/*
Purpose: Ask the wrapped device for the current marker through its vtable
         slot 0xB8.
// ORIGINAL: 0x004C6FF0 ?get_current_marker@Wave@@QAEHXZ 0x004C6FF0-0x004C7003 BYTE_EXACT
// symbol    ?get_current_marker@Wave@@UAEHXZ
// size      19 bytes
// prototype int (__thiscall ?get_current_marker@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or -1 when no device is wrapped
Status: Complete
*/
int Wave::get_current_marker() {
    if (device_) {
        return wave_playback_device(device_)->get_current_marker();
    }
    return -1;
}


/*
Purpose: Ask the wrapped device for the game window handle through its vtable
         slot 0x3C.
// ORIGINAL: 0x004C7010 ?get_game_hwnd@Wave@@QAEHXZ 0x004C7010-0x004C701F BYTE_EXACT
// symbol    ?get_game_hwnd@Wave@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?get_game_hwnd@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_game_hwnd() {
    if (device_) {
        return wave_playback_device(device_)->get_game_hwnd();
    }
    return 0;
}


/*
Purpose: Ask the wrapped device for the device count through its vtable
         slot 0xBC.
// ORIGINAL: 0x004C7020 ?get_ndevices@Wave@@QAEHXZ 0x004C7020-0x004C7032 BYTE_EXACT
// symbol    ?get_ndevices@Wave@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_ndevices@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped
Status: Complete
*/
int Wave::get_ndevices() {
    if (device_) {
        return wave_playback_device(device_)->get_ndevices();
    }
    return 0;
}


/*
Purpose: Describe a device into the caller's buffer through the wrapped
         device's vtable slot 0xC0. With no device the buffer is terminated
         to an empty string - but only when the third argument is nonzero,
         and without any null check on the buffer - and the answer is 1.
// ORIGINAL: 0x004C7040 ?get_device_description@Wave@@QAEHPADHH@Z 0x004C7040-0x004C7078 BYTE_EXACT
// symbol    ?get_device_description@Wave@@UAEHPADHH@Z
// size      56 bytes
// prototype int (__thiscall ?get_device_description@Wave@@QAEHPADHH@Z)(Wave* this, int8*, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7058
Return Value: the device's answer, or 1 when no device is wrapped
Status: Complete
*/
int Wave::get_device_description(char *a1, int a2, int a3) {
    if (device_) {
        return wave_playback_device(device_)->get_device_description(a1, a2, a3);
    }
    if (a3) {
        *a1 = '\0';
    }
    return 1;
}


/*
Purpose: Unknown. The original chases the pointer chain that starts at the
         wrapped device and continues through each node's own 0x3C slot until
         it runs out, reads nothing else, and always answers 1. The argument
         is ignored.
// ORIGINAL: 0x004C7080 ?UNK1@Wave@@QAEHH@Z 0x004C7080-0x004C7096 BYTE_EXACT
// symbol    ?probe_device_chain@Wave@@QAEHH@Z
// size      22 bytes
// prototype int (__thiscall ?UNK1@Wave@@QAEHH@Z)(Wave* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Wave::probe_device_chain(int) {
    Sound *cursor = device_;
    while (cursor) {
        cursor = cursor->device_;
    }
    return 1;
}


/*
Purpose: Set the reverb mix. The value is stored at 0x5C first, then the
         wrapped device is told through its vtable slot 0xE0.
// ORIGINAL: 0x004C70A0 ?set_reverb_mix@Wave@@QAEHM@Z 0x004C70A0-0x004C70C6 BYTE_EXACT
// symbol    ?set_reverb_mix@Wave@@UAEHM@Z
// size      38 bytes
// prototype int (__thiscall ?set_reverb_mix@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C70B3
// TRIED: the sole divergence is the register holding the vtable pointer at
//   the slot 0xE0 dispatch - the image loads it into `edx` because `eax` is
//   still holding the float argument it loaded for the `reverb_mix_` store,
//   where this tree kills the value after the store and reloads it from
//   [ebp+8]. Measured: the pointer-to-member `float` typedef (kept, 6/15),
//   a named `float` local (6/15, 17 instructions), `reinterpret_cast<const
//   uint32_t &>` on the argument with a `uint32_t` typedef (11/15 - better
//   agreement, but it is a type lie about a float slot and still MISMATCH),
//   passing the field instead of the parameter (6/15, 18), and a ternary
//   (11/15). Same allocator gap `Wave::play` records; left on the honest
//   spelling.
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_reverb_mix(float mix) {
    reverb_mix_ = mix;
    if (device_) {
        return wave_playback_device(device_)->set_reverb_mix(mix);
    }
    return 0x14;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C70D0 ?set_bufflimit@Wave@@QAEHI@Z 0x004C70D0-0x004C70D8 BYTE_EXACT
// symbol    ?set_bufflimit@Wave@@UAEHI@Z
// size      8 bytes
// prototype int (__thiscall ?set_bufflimit@Wave@@QAEHI@Z)(Wave* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_bufflimit(unsigned int) {
    return 11;
}


/*
Purpose: Returns a fixed 11; the legacy implementation is a single constant load.
// ORIGINAL: 0x004C70E0 ?set_asdr@Wave@@QAEHXZ 0x004C70E0-0x004C70E6 BYTE_EXACT
// symbol    ?set_asdr@Wave@@UAEHXZ
// size      6 bytes
// prototype int (__thiscall ?set_asdr@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_asdr() {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C70F0 ?set_attack@Wave@@QAEHIII@Z 0x004C70F0-0x004C70F8 BYTE_EXACT
// symbol    ?set_attack@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_attack@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_attack(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7100 ?set_sustain@Wave@@QAEHIII@Z 0x004C7100-0x004C7108 BYTE_EXACT
// symbol    ?set_sustain@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_sustain@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_sustain(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7110 ?set_decay@Wave@@QAEHIII@Z 0x004C7110-0x004C7118 BYTE_EXACT
// symbol    ?set_decay@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_decay@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_decay(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 11.
// ORIGINAL: 0x004C7120 ?set_release@Wave@@QAEHIII@Z 0x004C7120-0x004C7128 BYTE_EXACT
// symbol    ?set_release@Wave@@UAEHIII@Z
// size      8 bytes
// prototype int (__thiscall ?set_release@Wave@@QAEHIII@Z)(Wave* this, unsigned int, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 11, always
Status: Complete
*/
int Wave::set_release(unsigned int, unsigned int, unsigned int) {
    return 11;
}


/*
Purpose: Set the wave's volume. The low seven bits of the argument are stored
         at 0x04; while the wave holds a device group slot the level handed
         to the device is rescaled by that group's own volume dword (one
         every 24 bytes in the singleton's table), as level/127 * group in
         double precision truncated back to an integer. The wrapped device,
         if any, hears the result through its vtable slot 0x40.
// ORIGINAL: 0x004C7130 ?set_volume@Wave@@QAEXH@Z 0x004C7130-0x004C718D BYTE_EXACT
// symbol    ?set_volume@Wave@@UAEXH@Z
// TRIED: 26/33 MISMATCH, instruction COUNT now matches the image (33/33)
//   but the `fild dword ptr [ebp+8]` + `fmul qword ptr [const]` pair that
//   materialises `scaled` still schedules LATE (right before the final
//   multiply) where the image schedules it EARLY (right after the group-slot
//   range check, before the table read). Tried `volatile double scaled`
//   (worse, back to 13/33 - forces a real store/reload and an extra
//   instruction); a separate named `group` local read after `scaled` (no
//   change, still 26/33 - same late scheduling); `static double scaled`
//   (worse, 14/33, extra relocation-backed storage). VC6's scheduler treats
//   the local's definition point as a lower bound, not a fixed position, and
//   nothing tried pinned it to the top of the block.
// size      93 bytes
// prototype void (__thiscall ?set_volume@Wave@@QAEXH@Z)(Wave* this, int)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006458C8
// indirect  0x004C7184
Return Value: n/a
Status: Complete
*/
void Wave::set_volume(int volume) {
    // MASKED IN PLACE, no `vol` local: the image masks the incoming argument
    // and masks again for the field.
    volume &= 0x7F;
    volume_ = volume & 0x7F;
    int level = volume;
    if (group_slot_ < 0x10) {
        // The group entry is the LAST factor, so the level's own
        // `fild dword ptr [ebp+8]` is emitted first and the group arrives as a
        // `fild qword` + `fmulp st(1)` rather than an `fimul`. The original
        // loads that dword zero-extended through a 64-bit fild, so the scale
        // is the UNSIGNED value of the table entry.
        // LEVER: naming the `volume * (1.0/127.0)` product as its own local -
        // `scaled` - before multiplying by the group value, rather than
        // writing the whole product as one expression, is what makes VC6
        // emit the `fild`+`fmul`-by-constant PAIR the image has instead of
        // folding volume's conversion into a single `fimul` at the end. 13/33 ->
        // 26/33 MISMATCH.
// RECOVERY_LITERAL: v1 0x0066E530 0x0066E538 __real@8@3ff88102040810204000 0x004C7130
        const double reciprocal = 1.0 / 127.0;
        double scaled = static_cast<double>(volume);
        scaled *= reciprocal;
        level = static_cast<int>(static_cast<int64_t>(
            scaled * static_cast<double>(WaveDeviceGlobal.groups_[group_slot_].volume)));
    }
    if (device_) {
        wave_playback_device(device_)->set_volume(level);
    }
}


/*
Purpose: Position the wave in 3D through the wrapped device's vtable
         slot 0xCC.
// ORIGINAL: 0x004C7190 ?set_position3d@Wave@@QAEHMMM@Z 0x004C7190-0x004C71BB BYTE_EXACT
// symbol    ?set_position3d@Wave@@UAEHMMM@Z
// size      43 bytes
// prototype int (__thiscall ?set_position3d@Wave@@QAEHMMM@Z)(Wave* this, float, float, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C71A8
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_position3d(float x, float y, float z) {
    if (device_) {
        return wave_playback_device(device_)->set_position3d(x, y, z);
    }
    return 0x14;
}


/*
Purpose: Set the wave's X position through the wrapped device's vtable
         slot 0xD0.
// ORIGINAL: 0x004C71C0 ?set_xpos@Wave@@QAEHM@Z 0x004C71C0-0x004C71E3 BYTE_EXACT
// symbol    ?set_xpos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_xpos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C71D0
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_xpos(float x) {
    if (device_) {
        return wave_playback_device(device_)->set_xpos(x);
    }
    return 0x14;
}


/*
Purpose: Set the wave's Y position through the wrapped device's vtable
         slot 0xD4.
// ORIGINAL: 0x004C71F0 ?set_ypos@Wave@@QAEHM@Z 0x004C71F0-0x004C7213 BYTE_EXACT
// symbol    ?set_ypos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_ypos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7200
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_ypos(float y) {
    if (device_) {
        return wave_playback_device(device_)->set_ypos(y);
    }
    return 0x14;
}


/*
Purpose: Set the wave's Z position through the wrapped device's vtable
         slot 0xD8.
// ORIGINAL: 0x004C7220 ?set_zpos@Wave@@QAEHM@Z 0x004C7220-0x004C7243 BYTE_EXACT
// symbol    ?set_zpos@Wave@@UAEHM@Z
// size      35 bytes
// prototype int (__thiscall ?set_zpos@Wave@@QAEHM@Z)(Wave* this, float)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7230
Return Value: the device's answer, or 0x14 when no device is wrapped
Status: Complete
*/
int Wave::set_zpos(float z) {
    if (device_) {
        return wave_playback_device(device_)->set_zpos(z);
    }
    return 0x14;
}


/*
Purpose: Ask the wrapped device, through its vtable slot 0xDC, whether the
         wave is 3D positioned.
// ORIGINAL: 0x004C7250 ?is_3d@Wave@@QAEHXZ 0x004C7250-0x004C7262 BYTE_EXACT
// symbol    ?is_3d@Wave@@UAE_NXZ
// size      18 bytes
// Historical catalog prototype (inferred, not original debug information):
// int (__thiscall ?is_3d@Wave@@QAEHXZ)(Wave* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 0 when no device is wrapped (the
              original defines only AL on that path; callers test the byte)
Status: Complete
*/
bool Wave::is_3d() {
    // Pinned 0x4C7250 returns AL zero or tail-calls provider slot 0xDC.
    // SoundX's mode-1 table 0x10065738 selects 0x10021B90 through
    // 0x1000245A: flags 0xCC bit 0x20 and 0xC8 bit 2 yield only zero/one.
    // Model the predicate consistently at both ends; the wrapper defines AL
    // on its null path, while this provider sets the complete EAX to zero/one.
    // Unsigned char is also ABI-compatible; original source spelling is unknown.
    // Historical TRIED: narrowing only the zero expression while retaining an
    // int return did not change the function's return contract.
    if (device_) {
        return wave_playback_device(device_)->is_3d();
    }
    return false;
}
// ORIGINAL: 0x004C7270 sub_4c7270 0x004C7270-0x004C729E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c7270.cpp", "Pinned4C73C0 allocation of0x110 bytes guards the same4C7270 initialization before copying the name; current source uses this record initialization only at that allocation site."], "reason": "Model the observed new-record initialization as a genuine default constructor, preserving the memset and field-store order, null-allocation path, complete original identity/span and historical byte claim. The original nominal class/TU names and constructor interpretation remain provisional; verify the emitted constructor and its allocation caller together."}
// symbol    ??0NameTableEntry@@QAE@XZ
NameTableEntry::NameTableEntry() throw() {
    memset(name_, 0, 0x104);
    low_index_ = 0;
    high_index_ = 0;
    status_ = 1;
}
// ORIGINAL: 0x004C72A0 sub_4c72a0 0x004C72A0-0x004C72F5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/units/004c72a0.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?initialize@NameTable@@QAEPAU1@XZ
NameTable *NameTable::initialize() {
    memset(&flags_, 0, 4);
    flags_ = (flags_ & ~4u) | 2u;
    unknown_208_ = 0;
    unknown_20c_ = 0;
    name_ = 0;
    unknown_210_ = 0;
    unknown_214_ = 0xFA;
    memset(slots_, 0, sizeof(slots_));
    return this;
}
// ORIGINAL: 0x004C7300 sub_4c7300 0x004C7300-0x004C7373 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp", "Pinned4C7300..4C7373: every instruction is accounted for; optional name deallocation calls void operator delete64557F and the tail performs no EAX result definition before RET.", "Pinned64557F..64558A: push argument; call CRT free644EF2; popECX; ret, without an integer result contract. No directCALL/JMP candidate to4C7300 or current application source consumer of the clear result was observed; indirect caller completeness remains unproved.", "Pinned4C7380..4C73B3 exact remove_range owns the same bound snapshots, unconditional record deallocation and unsigned inclusive zeroing graph inlined at4C731A..4C733E. Pin outer loop uses signedJL at4C734A."], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries. The observed cleanup operation has no defined result; expose it as void rather than inventing a zero result. This does not assert the original nominal return type or complete original caller coverage."}
// symbol    ?clear@NameTable@@QAEXXZ
// TRIED: scaffold had 0 params/cdecl; disassembly reads ecx with no stack access -> changed to `__fastcall sub_4c7300(int param_1)`. Redeclared the callee fn_0064557f (operator delete@0x64557f) to take `void *` instead of the scaffold's 0-arg stub. for-loop over the outer bucket scan diverges at instr #24 ('mov' vs rebuilt 'sub').
void NameTable::clear() {
    int slot_index = 0;
    while (slot_index < 0x80) {
        NameTableEntry *entry = slots_[slot_index];
        if (entry != 0) {
            // Each installed range points at the same entry. Its +0 word is
            // the high bound; use that slot to reacquire its range metadata.
            slot_index = entry->high_index_;
            remove_range(slots_[slot_index]);
        }
        ++slot_index;
    }
    if (name_ != 0) {
        ::operator delete(name_);
        name_ = 0;
    }
}
// ORIGINAL: 0x004C7380 sub_4c7380 0x004C7380-0x004C73B3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c7380.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?remove_range@NameTable@@QAEIPBUNameTableEntry@@@Z
unsigned int NameTable::remove_range(const NameTableEntry *range) {
    unsigned int index = range->low_index_;
    unsigned int end = range->high_index_;
    void *victim = slots_[index];
    ++end;
    ::operator delete(victim);
    if (index < end) {
        for (unsigned int i = index; i < end; ++i) {
            slots_[i] = 0;
        }
    }
    return end;
}
// ORIGINAL: 0x004C73C0 sub_4c73c0 0x004C73C0-0x004C74A4 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?install@NameTable@@QAEXPBUNameTableEntry@@@Z
// TRIED: this is a __thiscall member whose `this` (ecx) is a raw `int *` array (indexed `param_1[uVar6]`, no fixed-offset fields), and param_2 is `Entry*` (3 uints + a 0x104-byte name buffer = 0x110 bytes, matching the `operator new(0x110)` allocation size exactly - `local_8` and `param_2` share this same layout). operator new/delete (mangled ??2@.../??3@...) reached via differently-named externs `op_new`/`op_delete` with the real signatures, since the scaffold's fn_0064557f/fn_0064558a are nullary and the call targets are masked from comparison anyway. strcpy is called UNCONDITIONALLY on `local_8->name` even on the `operator new` failure path (ghidra confirms this, not an artifact) - preserved as-is rather than "fixed".
void NameTable::install(const NameTableEntry *entry) {
    const unsigned int low = entry->low_index_;
    const unsigned int high = entry->high_index_;
    NameTableEntry *clone = new NameTableEntry;

    // Deliberately unconditional, including the original allocation-failure
    // path where clone is null.
    strcpy(clone->name_, entry->name_);

    if (clone->high_index_ < low) {
        clone->high_index_ = low;
    }
    clone->low_index_ = low;
    if (low > high) {
        clone->low_index_ = high;
    }
    clone->high_index_ = high;
    clone->status_ = entry->status_;

    unsigned int slot_index;
    for (slot_index = low; slot_index <= high; ++slot_index) {
        if (slots_[slot_index] != 0) {
            slot_index = remove_range(slots_[slot_index]);
        }
    }
    for (slot_index = low; slot_index <= high; ++slot_index) {
        slots_[slot_index] = clone;
    }
}
// ORIGINAL: 0x004C74B0 sub_4c74b0 0x004C74B0-0x004C74F0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/recovered/004c74b0.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?set_name@NameTable@@QAEXPAD@Z
void NameTable::set_name(char *name) {
    if (name_ != 0) {
        ::operator delete(name_);
    }
    name_ = static_cast<char *>(::operator new(strlen(name) + 1));
    strcpy(name_, name);
}
// ORIGINAL: 0x004C74F0 sub_4c74f0 0x004C74F0-0x004C75A0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:name-table-owner", "Original4C7270/72A0 initialization,4C7300/7380 inclusive cleanup,4C73C0 cloned0x110-byte records,4C74B0 heap string at200 and4C74F0 inline names at0C.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated inline range/name record and128-slot table accesses as genuine members under provisional ownership names; initializer-versus-constructor identity and historical TU are unproved. Preserve original obligations and seed discoveries."}
// symbol    ?rewrite_directory@NameTable@@QAEXPAD@Z
// TRIED: [ecx+N] reads with no matching stack slot -> a __thiscall receiver (Obj4c74f0), not the given __stdcall(int). 0.89 mnemonic sim; first divergence is prologue register/stack-slot allocation for the `this`/`a1` locals, not chased further.
void NameTable::rewrite_directory(char *directory) {
    char dir_buf[0x100];
    strcpy(dir_buf, directory);
    const unsigned int length = strlen(dir_buf);
    if (dir_buf[length - 1] != '\\') {
        dir_buf[length] = '\\';
        dir_buf[length + 1] = 0;
    }

    NameTableEntry **slot = slots_;
    for (int remaining = 0x80; remaining != 0; --remaining) {
        NameTableEntry *entry = *slot;
        if (entry != 0) {
            // The original forms (entry + 0xC) as a runtime pointer and
            // tests it, even though ordinary valid records make it non-null.
            char *name = entry->name_;
            if (name != 0) {
                char rewritten[0x100];
                strcpy(rewritten, dir_buf);
                char *slash = strrchr(name, '\\');
                char *basename = (slash != 0) ? slash + 1 : name;
                strcat(rewritten, basename);
                strcpy(name, rewritten);
            }
        }
        ++slot;
    }
}

// ORIGINAL: 0x004C75A0 sub_4c75a0 0x004C75A0-0x004C75A7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x004C75A0-0x004C75A7 executes mov eax,[ecx+0x200]; ret", "source:src/name_table.h NameTable::name_ is char* at +0x200, anchored by 0x004C74B0", "source:src/name_table.cpp adjacent NameTable::set_name owns allocation/update of the same member"], "reason": "Move the exact +0x200 pointer read onto NameTable's already declared name_ member. get_name is an operation-derived source spelling; no original mangled name is asserted."}
// symbol    ?get_name@NameTable@@QAEPADXZ
// kind      game
char *NameTable::get_name() {
    return name_;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// `mov ecx, dword ptr[0x9bc074]`-free entry, but the body reads `[ecx+N]`
// with no matching stack slot, so ecx is `this`, not a stack argument - the
// contract's nullary `__cdecl` is wrong. This is a constructor: three
// vtable installs in sequence, an SEH prologue (`fs:[0]`) with a 0x659f20
// unwind handler that resets the vtable to the first-phase value, and a
// span at 0x4c8450/0x659f20/0x659f28 that is PART of this same function
// (the "spans" list is multi-range). The SEH frame is not modeled here -
// see TRIED.
// ORIGINAL: 0x004C75B0 sub_4c75b0 0x004C75B0-0x004C766D;0x00659F20-0x00659F32 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/unrecovered/004c75b0.cpp", "src/midi.h", "src/ambience.h"], "reason": "Original Sound-to-Midi constructor staging and byte54 state identify the genuine Midi owner. Historical seed mapping includes the already shared unwind helper; its overlap remains a restoration limitation, not a newly inferred independent function extent."}
// symbol    ??0Midi@@QAE@XZ
// Historical SHARED_TAIL observation below predates the independent source identity for 0x004C8450; retain it as prior trial context, not a current span classification.
// TRIED: SHARED_TAIL - the 0x4c8450/0x659f20 unwind-handler span is COMDAT-folded with another function's handler, so no tier is reachable regardless of body. Left as a plain field-write transcription (no SEH frame modeled) for the next pass.
Midi::Midi() {
    memset(&field_54_, 0, 1);
    field_54_ |= 1;
    flags_40_ |= 8;
}

/*
Purpose: Destroy an ambience. Its own stage frees the remembered filename
         and releases the device through the guarded hook, clearing both
         fields unconditionally; the compiler-appended ~Sound then repeats
         the same teardown on the now-cleared fields (its frees no-op) and
         unlinks from the global sound chain when the chained bit is set -
         the image carries that Sound stage INLINED, which is why the old
         transcription duplicated it by hand.
// ORIGINAL: 0x004C7670 ??1Ambience@@QAE@XZ 0x004C7670-0x004C7760;0x00659F32-0x00659F4C BYTE_EXACT
// symbol    ??1Midi@@UAE@XZ
// size      273 bytes
// prototype void (__thiscall ??1Ambience@@QAE@XZ)(Ambience* this)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064557F
// indirect  0x004C76BE 0x004C76FA
Return Value: n/a
Status: Complete
*/
Midi::~Midi() {
    // THE OWN STAGE ONLY: free the remembered filename and release the
    // device through the guarded hook, clearing both unconditionally. The
    // compiler appends ~Sound, whose stage repeats the same teardown on the
    // now-cleared fields (so its frees no-op) and unlinks from the global
    // sound chain when the chained bit sets - the work the image's ~Ambience
    // carries INLINED. The vtable staging (0x66E538 -> 0x66E3C0 -> 0x66E444)
    // is the compiler's now; AmbienceSoundView - Sound's own member layout
    // under another name - is gone with the inheritance that made it
    // redundant.
    if (fname_) {
        operator delete(fname_);
    }
    fname_ = nullptr;
    if (device_ && WaveDeviceReleaseGuard) {
        (SoundXOrdinal2DeleteSound())(device_);
    }
    device_ = nullptr;
}

/*
 * The 32 forwarding methods of Midi, moved here from
 * delegation_thunks.cpp by the same pass that recovered the child relation:
 * each is a real member whose virtual child call lets VC6 emit the
 * image's `call dword ptr [eax + slot]` itself. 28 measured BYTE_EXACT
 * on arrival (1b8ba49f); the last four were never register allocation
 * but the provider's overload-group vtable ordering - fixed
 * 2026-09-18 by the two neutral slotNNN names (midi.h's header note
 * carries the measured VC6 behavior and the deck that isolated it).
 *
 * SINCE 2026-09-18 Midi declares its real Sound base (midi.h): the
 * sequencer forwarders read Sound::device_ at 0x3C through the common
 * reconstructed Midi type - the flat header's `delegate_` member is gone -
 * and the five lifecycle bodies below the forwarder run are the same
 * pass's second half. Every marker, claim and TRIED note carries over
 * unchanged.
 */

/*
Purpose: ?play@Midi@@QAEHXZ - forward 0 argument(s) to slot 0x1c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7760 ?play@Midi@@QAEHXZ 0x004C7760-0x004C7772 BYTE_EXACT
// symbol    ?play@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?play@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::play() {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->play();
    }
    return 0x14;
}


// ORIGINAL: 0x004C7780 ?set_volume@Midi@@QAEXH@Z 0x004C7780-0x004C77AC BYTE_EXACT
// symbol    ?set_volume@Midi@@UAEXH@Z
void Midi::set_volume(int volume) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        delegate->set_volume(volume);
    }
    volume_ = volume & 0x7f;
}

// ORIGINAL: 0x004C77B0 ?switch_state@Midi@@QAEH_N@Z 0x004C77B0-0x004C77D0 BYTE_EXACT
// symbol    ?switch_state@Midi@@UAEH_N@Z
// NO RETURN on the null path, deliberately: the original leaves eax alone.
int Midi::switch_state(bool on) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->switch_state(on);
    }
}
/*
Purpose: ?set_switch_type@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x100 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C77D0 ?set_switch_type@Midi@@QAEHK@Z 0x004C77D0-0x004C77F3 BYTE_EXACT
// symbol    ?set_switch_type@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_switch_type@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C77E0
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::set_switch_type(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_switch_type(a1);
    }
    return 0x13;
}
/*
Purpose: ?add_switch_range@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x108 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7800 ?add_switch_range@Midi@@QAEHKK@Z 0x004C7800-0x004C7827 BYTE_EXACT
// symbol    ?add_switch_range@Midi@@UAEHKK@Z
// size      39 bytes
// prototype int (__thiscall ?add_switch_range@Midi@@QAEHKK@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7814
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::add_switch_range(unsigned long a1, unsigned long a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->add_switch_range(a1, a2);
    }
    return 0x13;
}
/*
Purpose: ?set_nswitch_threads@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x104 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C7830 ?set_nswitch_threads@Midi@@QAEHK@Z 0x004C7830-0x004C7853 BYTE_EXACT
// symbol    ?set_nswitch_threads@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_nswitch_threads@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7840
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int Midi::set_nswitch_threads(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_nswitch_threads(a1);
    }
    return 0x13;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADKK@Z - forward 3 argument(s) to slot 0xdc of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7860 ?map_patch@Midi@@QAEHPADKK@Z 0x004C7860-0x004C788B BYTE_EXACT
// symbol    ?map_patch@Midi@@UAEHPADKK@Z
// size      43 bytes
// prototype int (__thiscall ?map_patch@Midi@@QAEHPADKK@Z)(Midi* this, int8*, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7878
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::map_patch(char *a1, unsigned long a2, unsigned long a3) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->map_patch(a1, a2, a3);
    }
    return 0x14;
}
/*
Purpose: ?map_patch@Midi@@QAEHPADK@Z - forward 2 argument(s) to slot 0xd8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7890 ?map_patch@Midi@@QAEHPADK@Z 0x004C7890-0x004C78B7 BYTE_EXACT
// symbol    ?map_patch@Midi@@UAEHPADK@Z
// size      39 bytes
// prototype int (__thiscall ?map_patch@Midi@@QAEHPADK@Z)(Midi* this, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C78A4
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::map_patch(char *a1, unsigned long a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->map_patch(a1, a2);
    }
    return 0x14;
}

// ORIGINAL: 0x004C78C0 ?set_base_path@Midi@@QAEHPBD@Z 0x004C78C0-0x004C78DA BYTE_EXACT
// symbol    ?set_base_path@Midi@@UAEHPBD@Z
int Midi::set_base_path(const char *path) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_base_path(path);
    }
}
/*
Purpose: ?clear_patch@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xe0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C78E0 ?clear_patch@Midi@@QAEHK@Z 0x004C78E0-0x004C7903 BYTE_EXACT
// symbol    ?clear_patch@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?clear_patch@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C78F0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::clear_patch(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->clear_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?reset@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7910 ?reset@Midi@@QAEHXZ 0x004C7910-0x004C7925 BYTE_EXACT
// symbol    ?reset@Midi@@UAEHXZ
// size      21 bytes
// prototype int (__thiscall ?reset@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::reset() {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->reset();
    }
    return 0x14;
}
/*
Purpose: ?set_track@Midi@@QAEHPAEK@Z - forward 2 argument(s) to slot 0x84 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7930 ?set_track@Midi@@QAEHPAEK@Z 0x004C7930-0x004C7957 BYTE_EXACT
// symbol    ?set_track@Midi@@UAEHPAEK@Z
// size      39 bytes
// prototype int (__thiscall ?set_track@Midi@@QAEHPAEK@Z)(Midi* this, unsigned int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7944
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_track(uint8_t *a1, unsigned long a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_track(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?mute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7960 ?mute_track@Midi@@QAEHK@Z 0x004C7960-0x004C7983 BYTE_EXACT
// symbol    ?mute_track@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?mute_track@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7970
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::mute_track(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->mute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?unmute_track@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xac of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7990 ?unmute_track@Midi@@QAEHK@Z 0x004C7990-0x004C79B3 BYTE_EXACT
// symbol    ?unmute_track@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?unmute_track@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C79A0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::unmute_track(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->unmute_track(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHKK@Z - forward 2 argument(s) to slot 0x94 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79C0 ?set_active_tracks@Midi@@QAEHKK@Z 0x004C79C0-0x004C79E7 BYTE_EXACT
// symbol    ?set_active_tracks@Midi@@UAEHKK@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_tracks@Midi@@QAEHKK@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C79D4
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_tracks(unsigned long a1, unsigned long a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_tracks(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_tracks@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x90 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C79F0 ?set_active_tracks@Midi@@QAEHI@Z 0x004C79F0-0x004C7A13 BYTE_EXACT
// symbol    ?set_active_tracks@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?set_active_tracks@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A00
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_tracks(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_tracks(a1);
    }
    return 0x14;
}
/*
Purpose: ?play_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x10c of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A20 ?play_trackset@Midi@@QAEHI@Z 0x004C7A20-0x004C7A43 BYTE_EXACT
// symbol    ?play_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?play_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A30
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::play_trackset(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->play_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?xpose_trackset@Midi@@QAEHIH@Z - forward 2 argument(s) to slot 0x114 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A50 ?xpose_trackset@Midi@@QAEHIH@Z 0x004C7A50-0x004C7A77 BYTE_EXACT
// symbol    ?xpose_trackset@Midi@@UAEHIH@Z
// size      39 bytes
// prototype int (__thiscall ?xpose_trackset@Midi@@QAEHIH@Z)(Midi* this, unsigned int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A64
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::xpose_trackset(unsigned int a1, int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->xpose_trackset(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?stop_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x110 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7A80 ?stop_trackset@Midi@@QAEHI@Z 0x004C7A80-0x004C7AA3 BYTE_EXACT
// symbol    ?stop_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?stop_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7A90
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::stop_trackset(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->stop_trackset(a1);
    }
    return 0x14;
}

// ORIGINAL: 0x004C7AB0 ?is_trackset_playing@Midi@@QAE_NI@Z 0x004C7AB0-0x004C7AD0 BYTE_EXACT
// symbol    ?is_trackset_playing@Midi@@UAE_NI@Z
bool Midi::is_trackset_playing(unsigned int trackset) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->is_trackset_playing(trackset);
    }
    return false;
}
/*
Purpose: ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z - forward 1 argument(s) to slot 0x98 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7AD0 ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z 0x004C7AD0-0x004C7AF3 BYTE_EXACT
// symbol    ?add_active_trackset@Midi@@UAEHPAUTrackSet@@@Z
// size      35 bytes
// prototype int (__thiscall ?add_active_trackset@Midi@@QAEHPAUTrackSet@@@Z)(Midi* this, TrackSet*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7AE0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::add_active_trackset(TrackSet *a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->add_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_lo@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B00 ?set_active_range_lo@Midi@@QAEHII@Z 0x004C7B00-0x004C7B27 BYTE_EXACT
// symbol    ?set_active_range_lo@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_range_lo@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B14
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_range_lo(unsigned int a1, unsigned int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_range_lo(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?set_active_range_hi@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0xa4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B30 ?set_active_range_hi@Midi@@QAEHII@Z 0x004C7B30-0x004C7B57 BYTE_EXACT
// symbol    ?set_active_range_hi@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_active_range_hi@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B44
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_active_range_hi(unsigned int a1, unsigned int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_active_range_hi(a1, a2);
    }
    return 0x14;
}
/*
Purpose: ?remove_active_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0xa8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7B60 ?remove_active_trackset@Midi@@QAEHI@Z 0x004C7B60-0x004C7B83 BYTE_EXACT
// symbol    ?remove_active_trackset@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?remove_active_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7B70
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::remove_active_trackset(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->remove_active_trackset(a1);
    }
    return 0x14;
}
/*
Purpose: ?get_trackset@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x9c of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7B90 ?get_trackset@Midi@@QAEHI@Z 0x004C7B90-0x004C7BB0 BYTE_EXACT
// symbol    ?get_trackset@Midi@@UAEHI@Z
// size      32 bytes
// prototype int (__thiscall ?get_trackset@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7BA0
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_trackset(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->get_trackset(a1);
    }
    return 0;
}
/*
Purpose: ?get_ntracks@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xc4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7BB0 ?get_ntracks@Midi@@QAEHXZ 0x004C7BB0-0x004C7BC2 BYTE_EXACT
// symbol    ?get_ntracks@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_ntracks@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_ntracks() {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->get_ntracks();
    }
    return 0;
}
/*
Purpose: ?load_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x120 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7BD0 ?load_patch@Midi@@QAEHI@Z 0x004C7BD0-0x004C7BF3 BYTE_EXACT
// symbol    ?load_patch@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?load_patch@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7BE0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::load_patch(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->load_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?unload_patch@Midi@@QAEHI@Z - forward 1 argument(s) to slot 0x124 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C00 ?unload_patch@Midi@@QAEHI@Z 0x004C7C00-0x004C7C23 BYTE_EXACT
// symbol    ?unload_patch@Midi@@UAEHI@Z
// size      35 bytes
// prototype int (__thiscall ?unload_patch@Midi@@QAEHI@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7C10
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::unload_patch(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->unload_patch(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_patch@Midi@@QAEHII@Z - forward 2 argument(s) to slot 0x128 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7C30 ?set_patch@Midi@@QAEHII@Z 0x004C7C30-0x004C7C57 BYTE_EXACT
// symbol    ?set_patch@Midi@@UAEHII@Z
// size      39 bytes
// prototype int (__thiscall ?set_patch@Midi@@QAEHII@Z)(Midi* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7C44
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_patch(unsigned int a1, unsigned int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_patch(a1, a2);
    }
    return 0x14;
}

// ===== HOMED 2026-09-03 out of the archive. Each is one guarded virtual
// call through the sequencer at 0x3C - Sound::device_, reconstructed as a
// Midi object by the pinned shared constructor and table evidence.

/*
Purpose: Initialise the MIDI from a filename. The name replaces whatever
         copy the object already holds at 0x4C - the old block returns to
         the game CRT heap, the new one is measured, allocated on that
         same heap and copied in - and the one Midi-own flag byte at 0x54
         is cleared. The second argument is ignored; the status word is
         memset's own answer (the destination pointer).
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C7C60 ?init@Midi@@QAEHPADK@Z 0x004C7C60-0x004C7CB5 BYTE_EXACT
// symbol    ?init@Midi@@UAEHPADK@Z
// size      85 bytes
// prototype int (__thiscall ?init@Midi@@QAEHPADK@Z)(Midi* this, int8*, unsigned int)
// callers   0   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A 0x006465F0
// LEVER: `return reinterpret_cast<int>(memset(...))` - VC6 rejects a
//        non-void body with no return at all (C4716), and the image's
//        status word IS memset's eax, so the return spells the shared
//        fall-through of both paths. The null store between delete and
//        new is real (the seed measured it): the field is cleared before
//        the new block is measured.
// LEVER: `#pragma function(memset)` (file-local, above) - /O2 implies /Oi,
//        and the intrinsic expands a ONE-byte memset to
//        `mov byte ptr [esi],0`; the image's clear at 0x004C7CA3 is a real
//        call (`push 1 / add esi,0x54 / push 0 / push esi / call _memset /
//        add esp,0xC`). 80 -> 85 bytes, the marker's own size.
// LEVER: the destination is the declared member `field_54_` (midi.h),
//        not a raw this+0x54 cast - this file's one class-debt raw
//        self-access finding. Deck-measured 2026-09-18: the member
//        address compiles to the same bytes as the cast under /Oy-
//        (differing_offsets empty, every neighbor unchanged).
// PROMOTED from src/recovered/units/004c7c60.cpp (seed BYTE_EXACT); homed
//        onto the Sound base 2026-09-18 - the 0x4C slot is Sound::fname_
//        and the cleared byte is the first Midi-own byte past the 0x54
//        Sound sub-object.
// LEVER: the allocation is the DIRECT operator new - ??2@YAPAXI@Z, the
//        image's own callee at 0x0064558A - symmetric with the operator
//        delete above it (??3, 0x006453E0). Measured 2026-09-18: the
//        re-spelling is byte-neutral under /Oy- (differing_offsets still
//        empty) and swaps the emitted reference from the unbound
//        WaveOperatorNew forwarder to ??2, the identity the original
//        relocation names.
Return Value: memset's destination pointer, reinterpreted
Status: Complete
*/
int Midi::init(char *a1, unsigned long a2) {
    if (a1 != 0) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = nullptr;
        fname_ = static_cast<char *>(operator new(strlen(a1) + 1));
        strcpy(fname_, a1);
    }
    return reinterpret_cast<int>(
        memset(&field_54_, 0, 1));
}


// Original4C7CC0: replace bit0 throughout the chained Midi objects.
// ORIGINAL: 0x004C7CC0 sub_4c7cc0 0x004C7CC0-0x004C7CEA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/004c7cc0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?set_mode@Midi@@QAEHE@Z
int Midi::set_mode(uint8_t mode) {
    field_54_ ^= (field_54_ ^ static_cast<uint8_t>(mode)) & 1;
    // The native body recurses on the object pointer stored at +0x3C; it does
    // not dispatch a Midi provider virtual. This cast preserves the same
    // receiver without asserting that the external provider is a game Midi.
    Midi *const child = static_cast<Midi *>(device_);
    if (child) child->set_mode(mode);
    return 0;
}

/*
Purpose: Remember the MIDI's filename. The previous copy, if any, goes back
         to the game CRT heap first; the new string is measured, allocated
         on that same heap, recorded at 0x4C, and copied in - the field
         already names the block while it is still being filled.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// The staged preamble's `_strcpy`/`_strlen` decls are stale (arity-less,
// and `_strlen`/`_strcpy` as C++ names each pick up a SECOND leading
// underscore from the `extern "C"` __cdecl decoration, landing on
// `__strlen`/`__strcpy` - not the CRT's real `_strlen`/`_strcpy` symbol).
// ORIGINAL: 0x004C7CF0 ?set_fname@Midi@@QAEHPBD@Z 0x004C7CF0-0x004C7D42 BYTE_EXACT
// size      82 bytes
// prototype int (__thiscall ?set_fname@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006453E0 0x00645460 0x0064557F 0x0064558A
// LEVER: branch polarity - `if (fname) { work; return 0; } return 0xA;`,
//        the same positive-branch shape Wave::set_fname records
//        (wave.cpp); the guard clause inlines the 0xA return at the top
//        instead of a shared tail at the end.
// PROMOTED from src/recovered/units/004c7cf0.cpp (seed BYTE_EXACT; its
//        `// symbol` PAC fact was the seed's own non-const parameter -
//        the product's `const char *` mangles the catalogued PBD); homed
//        onto the Sound base 2026-09-18 - the 0x4C slot is Sound::fname_.
// LEVER: the allocation is the DIRECT operator new, ??2@YAPAXI@Z at
//        0x0064558A, symmetric with the operator delete above it - the
//        same re-spelling init's marker records, measured with it
//        2026-09-18 (byte-neutral under /Oy-, emitted reference ??2 in
//        place of the unbound WaveOperatorNew forwarder).
Return Value: 0, or 0xA when the name is null
Status: Complete
*/
int Midi::set_fname(const char *fname) {
    if (fname) {
        if (fname_) {
            operator delete(fname_);
        }
        fname_ = nullptr;
        fname_ = static_cast<char *>(operator new(strlen(fname) + 1));
        strcpy(fname_, fname);
        return 0;
    }
    return 0xA;
}
/*
Purpose: ?set_tempo@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0x88 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7D50 ?set_tempo@Midi@@QAEHK@Z 0x004C7D50-0x004C7D73 BYTE_EXACT
// symbol    ?set_tempo@Midi@@UAEHK@Z
// size      35 bytes
// prototype int (__thiscall ?set_tempo@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7D60
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_tempo(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_tempo(a1);
    }
    return 0x14;
}
#pragma optimize("y", off)

/*
Purpose: Load the MIDI from a caller-supplied filename. With no wrapped
         sequencer yet, the creation hook - the slot beside the release
         hook, behind the same guard dword - builds one directly into the
         0x3C field, with the MIDI's own mode 2 (Wave's loaders pass 1);
         the base Sound::load then reads the file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// NOT BYTE_EXACT - MISMATCH, closest reached (mnemonic_similarity 0.9275,
// 32/34 mnemonics in common). The control-flow shape matches the original
// exactly (same branches, same early return, same shared tail calling
// Sound::load), and the ONLY divergence is which callee-saved register
// holds which value: the original assigns esi=this, edi=a1; every source
// form tried here compiles to edi=this, esi=a1 instead - a straight swap,
// not a logic difference. Ruled out (all six produced the identical
// swap): if/else vs. nested-if vs. goto-threaded control flow, naming the
// Sound* cast in its own local, naming a1 in its own local, and addressing
// field_3c_ through a named member vs. a raw reinterpret_cast<char*>(this)
// + 0x3c pointer. This reads as a fixed backend register-allocation
// preference for this shape (which stack param gets which saved register)
// rather than anything source-form can steer - the same class of thing the
// scheduler notes above describe for `lea`/`push` ordering.
// ORIGINAL: 0x004C7D80 ?load@Midi@@QAEHPBD@Z 0x004C7D80-0x004C7DCD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x4C7DB8 loads filename stack word into EAX; 0x4C7DBB tests it and only a nonzero word bypasses Sound::load at0x4C7DC2.", "Pinned 0x4C7D86 checks receiver device at+3C. Missing device uses the guarded factory with mode2; factory failure remains its returned integer.", "Existing Midi and Sound declarations own this virtual override and device field. The free factory and qualified base call retain their existing ABI; pointer-to-int narrowing is the configured 32-bit target interpretation."], "reason": "Preserve the original pointer-valued integer status on the already-wrapped MIDI path; a null filename must still reach the qualified Sound loader."}
// symbol    ?load@Midi@@UAEHPBD@Z
// size      78 bytes
// prototype int (__thiscall ?load@Midi@@QAEHPBD@Z)(Midi* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C7D8D
// NO BYTE CLAIM: no seed ever measured this body; this is its first
//        product spelling (the task's one genuinely unmeasured body).
//        The Sound::load call is QUALIFIED because Midi::load(const
//        char*) overrides the same virtual - unqualified would recurse.
// Original ABI quirk: with a sequencer already wrapped, the image returns
//        the filename pointer bits as its integer status. A null filename
//        instead reaches Sound::load(nullptr); do not normalize either path
//        to a constant error code. Pointer-to-int narrowing below follows
//        the configured 32-bit VC6 target, not a portable pointer-width claim.
// Historical pre-restoration compiler diagnostic:
//        image hoists `lea eax,[esi+0x3C]` before the null test and caches
//        the argument in edi, so its guard and Sound::load sit at offsets
//        22 and 67 where this spelling puts them at 19 and 62. The
//        creation hook itself lands at the image's own offset 47
//        (measured 2026-09-18) through the SoundDllProcs table - the
//        include note above explains why the fixed-address accessor
//        cannot.
Return Value: filename pointer bits when already wrapped and non-null;
              otherwise 1 with a dead creation hook, the creation error,
              or the base loader's answer
Status: Complete
*/
int Midi::load(const char *a1) {
    int result;
    if (device_ == nullptr) {
        if (!WaveDeviceReleaseGuard) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, a1, 2);
    }
    // With a device the image tests this never-written local; VC6 homes it
    // in a1's dead stack slot, so it reads back as the filename pointer.
    if (result != 0) {
        return result;
    }
    return Sound::load(a1);
}
#pragma optimize("", on)

/*
Purpose: Load the MIDI from its remembered filename. With no wrapped
         sequencer yet, the guarded creation hook builds one directly
         into the 0x3C field, again with mode 2; the base Sound::load
         then reads the file.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C7DD0 ?load@Midi@@QAEHXZ 0x004C7DD0-0x004C7E1B BYTE_EXACT
// symbol    ?load@Midi@@UAEHXZ
// size      75 bytes
// prototype int (__thiscall ?load@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C6280
// indirect  0x004C7DFB
// PROMOTED from src/recovered/004c7dd0.cpp (byte-exact emitter match, the
//        only seed for this family that called the base loader directly);
//        homed onto the Sound base 2026-09-18 - the remembered name is
//        Sound::fname_, the wrapped sequencer is Sound::device_, and the
//        qualified `Sound::load` is the direct 0x004C6280 call the image
//        makes (Midi::load() is an overload, not an override, but the
//        qualification keeps the call non-virtual either way).
// LEVER: the creation hook calls `dword ptr [SoundDllProcs+0]`, the
//        table sound.h owns - wave.h's fixed-address accessor emits the
//        same `call [mem]` but folds 0x90DB24 into the disp32 with NO
//        relocation, and the image has a HIGHLOW fixup there. Measured
//        2026-09-18: guard, hook and Sound::load relocations land at the
//        image's own offsets 23, 45 and 60.
Return Value: 0 on success, 8 with no filename, 1 with a dead creation
              hook, or the creation/load error
Status: Complete
*/
int Midi::load() {
    const char *filename = fname_;
    if (filename != 0) {
        if (device_ == nullptr) {
            if (!WaveDeviceReleaseGuard) {
                return 1;
            }
            const int created = SoundXOrdinal1CreateSound()(
                &device_, filename, 2);
            if (created != 0) {
                return created;
            }
        }
        return Sound::load(filename);
    }
    return 8;
}

/*
Purpose: Release the loaded MIDI. While the sequencer at 0x3C is still
         sounding (the base Sound's own is_playing, family slot 0x5C),
         the sequencer's unload (family slot 5) runs alone and its answer
         is the result; once quiet, the same unload runs and the wrapped
         sequencer then goes back through the release hook when that hook
         is live. The sequencer is forgotten either way, the base's detach
         (slot 0x80) runs, and the loaded bit of the flag dword at 0x40
         is cleared.
// ORIGINAL: 0x004C7E20 ?unload@Midi@@QAEHXZ 0x004C7E20-0x004C7E79 BYTE_EXACT
// symbol    ?unload@Midi@@UAEHXZ
// size      89 bytes
// prototype int (__thiscall ?unload@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7E2F 0x004C7E3B 0x004C7E4B 0x004C7E56 0x004C7E66
// LEVER: the real Sound base retires the scaffold's MidiVCall shims - the
//        is_playing/detach dispatches become the compiler's own virtual
//        calls through the vtable the base now owns, and the sequencer's
//        slot-5 unload is the provider Midi object's virtual unload, int
//        because its answer is this body's result on the busy path.
// LEVER: NO named delegate local. The image holds the result in edi from
//        `xor edi,edi` at entry and re-reads Sound::device_ at each
//        dispatch (`mov ecx,[esi+0x3C]` / `mov edx,[ecx]` after the
//        is_playing call, `mov eax,[esi+0x3C]` before the release hook);
//        a named local steals edi for the pointer and pushes ebx for the
//        result, two instructions the image has not. 96 -> 89 bytes.
// LEVER: the release hook calls `dword ptr [SoundDllProcs+4]`, not
//        wave.h's fixed-address accessor - the accessor folds 0x90DB28
//        into the disp32 with no relocation where the image has a HIGHLOW
//        fixup (see the include note above). Measured 2026-09-18: guard
//        and hook relocations land at the image's own offsets 31 and 45.
// LEVER: own vtable shim (not the scaffolded VCall) so slots 5/23 return int; offset 0x40 field reached by pointer arithmetic, masked &=
// PROMOTED from git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004c7e20.cpp (scaffold BYTE_EXACT with
//        the shim tables); this is the body's first product spelling.
Return Value: the sequencer's unload answer on the busy path, 0 otherwise
Status: Complete
*/
int Midi::unload() {
    int result = 0;
    if (device_ != nullptr) {
        if (is_playing() == 0) {
            static_cast<Midi *>(device_)->unload();
            if (WaveDeviceReleaseGuard) {
                SoundXOrdinal2DeleteSound()(device_);
            }
        } else {
            result = static_cast<Midi *>(device_)->unload();
        }
        device_ = nullptr;
    }
    detach();
    flags_40_ &= ~1u;
    return result;
}
/*
Purpose: ?get_time@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xd4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7E80 ?get_time@Midi@@QAEHK@Z 0x004C7E80-0x004C7EA0 BYTE_EXACT
// symbol    ?get_time@Midi@@UAEHK@Z
// size      32 bytes
// prototype int (__thiscall ?get_time@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7E90
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_time(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->get_time(a1);
    }
    return 0;
}
/*
Purpose: ?get_control_track@Midi@@QAEHXZ - forward 0 argument(s) to slot 0xf4 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EA0 ?get_control_track@Midi@@QAEHXZ 0x004C7EA0-0x004C7EB2 BYTE_EXACT
// symbol    ?get_control_track@Midi@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_control_track@Midi@@QAEHXZ)(Midi* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_control_track() {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->get_control_track();
    }
    return 0;
}
/*
Purpose: ?get_total_track_ticks@Midi@@QAEHK@Z - forward 1 argument(s) to slot 0xf8 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C7EC0 ?get_total_track_ticks@Midi@@QAEHK@Z 0x004C7EC0-0x004C7EE0 BYTE_EXACT
// symbol    ?get_total_track_ticks@Midi@@UAEHK@Z
// size      32 bytes
// prototype int (__thiscall ?get_total_track_ticks@Midi@@QAEHK@Z)(Midi* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7ED0
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int Midi::get_total_track_ticks(unsigned long a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->get_total_track_ticks(a1);
    }
    return 0;
}
/*
Purpose: ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z - forward 1 argument(s) to slot 0xe4 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7EE0 ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z 0x004C7EE0-0x004C7F03 BYTE_EXACT
// symbol    ?set_channel_ctrl@Midi@@UAEHPAUChannelCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_channel_ctrl@Midi@@QAEHPAUChannelCtrl@@@Z)(Midi* this, ChannelCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7EF0
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_channel_ctrl(ChannelCtrl *a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_channel_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z - forward 1 argument(s) to slot 0xe8 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F10 ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z 0x004C7F10-0x004C7F33 BYTE_EXACT
// symbol    ?set_program_ctrl@Midi@@UAEHPAUProgramCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_program_ctrl@Midi@@QAEHPAUProgramCtrl@@@Z)(Midi* this, ProgramCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7F20
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_program_ctrl(ProgramCtrl *a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_program_ctrl(a1);
    }
    return 0x14;
}
/*
Purpose: ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z - forward 1 argument(s) to slot 0xc0 of the
         object at +0x3c, answering 0x14 when that object is absent.
// ORIGINAL: 0x004C7F40 ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z 0x004C7F40-0x004C7F63 BYTE_EXACT
// symbol    ?set_track_ctrl@Midi@@UAEHPAUTrackCtrl@@@Z
// size      35 bytes
// prototype int (__thiscall ?set_track_ctrl@Midi@@QAEHPAUTrackCtrl@@@Z)(Midi* this, TrackCtrl*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C7F50
Return Value: the delegate's, or 0x14 when the member is null
Status: Complete
*/
int Midi::set_track_ctrl(TrackCtrl *a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_track_ctrl(a1);
    }
    return 0x14;
}

// ORIGINAL: 0x004C7F70 sub_4c7f70 0x004C7F70-0x004C7F97 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/004c7f70.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7f70@Midi@@UAEHHH@Z
int Midi::sub_4c7f70(int a1, int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) return delegate->sub_4c7f70(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C7FA0 sub_4c7fa0 0x004C7FA0-0x004C7FC0 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/004c7fa0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7fa0@Midi@@UAE_NI@Z
bool Midi::sub_4c7fa0(unsigned int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) return delegate->sub_4c7fa0(a1);
    return false;
}
// ORIGINAL: 0x004C7FC0 sub_4c7fc0 0x004C7FC0-0x004C7FE7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/004c7fc0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7fc0@Midi@@UAEHHH@Z
int Midi::sub_4c7fc0(int a1, int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) return delegate->sub_4c7fc0(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C7FF0 sub_4c7ff0 0x004C7FF0-0x004C8017 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/004c7ff0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c7ff0@Midi@@UAEHHH@Z
int Midi::sub_4c7ff0(int a1, int a2) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) return delegate->sub_4c7ff0(a1, a2);
    return 0x14;
}
// ORIGINAL: 0x004C8020 sub_4c8020 0x004C8020-0x004C8043 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/units/004c8020.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?sub_4c8020@Midi@@UAEHH@Z
int Midi::sub_4c8020(int a1) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) return delegate->sub_4c8020(a1);
    return 0x14;
}

// ORIGINAL: 0x004C8050 ?set_play_type@Midi@@QAEHI@Z 0x004C8050-0x004C806A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned4C8058 null guard reaches POP EBP/RET4 at4C8066/67; the nonnull virtual call rejoins that epilogue. Eleven reachable instructions end4C806A, then six NOPs and independently mapped initialize4C8070.", "IDA/Ghidra and entry-rooted PE flow agree. Historical2f1f617d:src/midi.cpp extended through4C8072, incorrectly including the next MOV EAX,ECX. The null path does not write EAX; missing-return/compiler-effect fidelity remains unresolved. No return value, body or claim is changed."], "reason": "Bind the Midi delegate forwarder to both local-return paths, excluding padding and a different initializer."}
// symbol    ?set_play_type@Midi@@UAEHI@Z
// Historical TRIED interpretation below is disproven by entry-based decoding:
// the null-delegate branch reaches pop ebp / ret 4 at 0x004C8066/0x004C8067.
// Six padding NOPs follow; mov eax, ecx at 0x004C8070 belongs to the separate
// mapped initializer, and is never reached by this forwarder's null path.
// On that path this method does not write EAX: its result depends on incoming
// register state, not this or a defined zero. The missing C++ return remains an
// unresolved compiler-effects obligation, not proof of faithful reproduction.
// The corrected code extent excludes the next entry; historical provenance
// and claims remain, but this does not resolve the missing-return obligation.
// TRIED: the guarded forwarder is byte-identical for 11 instructions and then
//   the image has six `nop`s and `mov eax, ecx` - on the null delegate path the
//   original returns `this` in eax, not an unset eax, so the artifact's "no
//   return, the original leaves eax untouched" note is wrong about what the
//   caller sees. Spelling that needs `return (int)this;`, a pointer-as-int
//   cast the class-debt ratchet holds at zero. Left at 11 of 18.
// Historical incorrect conclusion: the shipped body leaves eax = `this` on the null path
      //   because ecx still holds it, so callers read a pointer as the int result.
//   Historical reproduction claim is unverified; see the correction above.
int Midi::set_play_type(unsigned int type) {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate != nullptr) {
        return delegate->set_play_type(type);
    }
}

/*
Purpose: Set +0 to 0x24, clear +4, and set bit zero at +8 while returning the
         original receiver pointer. The sequence is retained in source order.

         Original instruction record: mov eax,ecx / mov ecx,[eax+8] /
         mov dword [eax+4],0 / or ecx,1 / mov dword [eax],0x24 /
         mov [eax+8],ecx / ret
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8070 sub_4c8070 0x004C8070-0x004C8089 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned receiver-shaped word operation as a member of a distinct bounded source-state type without claiming its original class or lifetime.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8070-0x004C8089 reads +8, clears +4, writes +0=0x24, stores the modified +8 word, and returns the receiver","idb:exact pinned IDB function boundary 0x004C8070-0x004C8089; no type/owner record","proposal:neutral member name denotes only the observed bounded word operations; no original class/extent/caller/lifetime assertion"]}
// symbol    ?initialize@PrefixFlagState@@QAEPAU1@XZ
// size      25 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
PrefixFlagState *PrefixFlagState::initialize() {
    uint32_t saved_word_08 = word_08_;
    word_04_ = 0;
    saved_word_08 |= 1U;
    word_00_ = 0x24U;
    word_08_ = saved_word_08;
    return this;
}

/*
Purpose: Zero the observed words at +0 through +1C and set the word at +20
         to all ones, then return the original receiver pointer.

         Original instruction record: mov eax,ecx / xor ecx,ecx /
         store zero at +4,+8,+0C,+10,+0,+14,+18,+1C /
         store 0xFFFFFFFF at +20 / ret

         Historical seed provenance: this identity was previously rendered
         as an address-named void-pointer redirect because its class was not
         established. The current member type remains a bounded source model,
         not a claim about that original class.
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8090 sub_4c8090 0x004C8090-0x004C80B3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned nine-word reset as a member of a distinct bounded source-state type without relating it to either nearby leaf type.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8090-0x004C80B3 writes eight zero dwords and then 0xFFFFFFFF at +0x20, returning the receiver","idb:exact pinned image has no xref-tag entries or class/type/lifetime record at this address","proposal:bounded type describes only touched words; no original class/extent/caller/lifetime assertion"]}
// symbol    ?reset@NineWordSentinelState@@QAEPAU1@XZ
// size      35 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
NineWordSentinelState *NineWordSentinelState::reset() {
    word_04_ = 0;
    word_08_ = 0;
    word_0C_ = 0;
    word_10_ = 0;
    word_00_ = 0;
    word_14_ = 0;
    word_18_ = 0;
    word_1C_ = 0;
    word_20_ = 0xFFFFFFFFU;
    return this;
}


// The paired operations update the two bounds in their original order. Their
// unsigned comparisons are pinned behavior, not a claim about endpoint meaning.
/*
 * RECOVERY_HISTORY: v1 address=0x004C80C0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=e0d6eb0a4d32e64334cc1f4cded43e7d701b510f32d7a1a14fb4d75f7ad5a11f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0xc against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 0xc] / cmp edx, eax / jae 0x4c80d0 / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80c0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80C0-0x004C80D7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_lower_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_lower_endpoint(int value) {
    if (static_cast<DWORD>(upper_bound_C_) < static_cast<DWORD>(value)) {
        upper_bound_C_ = value;
    }
    lower_bound_8_ = value;
}

/*
 * RECOVERY_HISTORY: v1 address=0x004C80E0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=8c15061494d8d9b690e02e41f5aa7054bafa4a713979ab8f6841a65deb438b95
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0x8 against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 8] / cmp edx, eax / jbe 0x4c80f0 / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80e0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80E0-0x004C80F7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_upper_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_upper_endpoint(int value) {
    if (static_cast<DWORD>(lower_bound_8_) > static_cast<DWORD>(value)) {
        lower_bound_8_ = value;
    }
    upper_bound_C_ = value;
}

/*
Purpose: Clear the observed words at +4 and +8, preserve +0, and return the
         original receiver pointer.

         Original instruction record: mov eax,ecx / xor ecx,ecx /
         store zero at +4 / store zero at +8 / ret

         Historical seed provenance: this identity was previously rendered
         as an address-named void-pointer redirect because its class was not
         established. The current member type remains a bounded source model,
         not a claim about that original class.
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8100 sub_4c8100 0x004C8100-0x004C810B BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned two-word clear as a member of a third, distinct bounded source-state type without joining it to either nearby type.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8100-0x004C810B clears +4 and +8, leaves +0 alone, and returns the receiver","idb:exact pinned IDB has no type/owner evidence for this address","proposal:bounded type names only the three-word test prefix; no original class/extent/caller/lifetime assertion"]}
// symbol    ?reset_tail@PairWordState@@QAEPAU1@XZ
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
PairWordState *PairWordState::reset_tail() {
    word_04_ = 0;
    word_08_ = 0;
    return this;
}

// RECOVERY_ROLE: SCAFFOLD Provisional four-word initialization view; original containing class, complete extent and constructor-to-instance lifetime flow remain unproved. Matching the endpoint views' offsets is not a receiver ownership link.
// ORIGINAL: 0x004C8110 sub_4c8110 0x004C8110-0x004C813B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8110-0x004C813B: complete thiscall constructor first zeroes four bytes [this+4] through memset, then stores dword [this], dword [this+8], and dword [this+0xC] in this order; [this+4] is zeroed by the actual four-byte memset call at 0x004C811B to 0x006465F0", "git:e37791048e964cd8bfe9de1d210ec2d04213cdca:src/recovered/004c8110.cpp: historical-only seed hint // symbol ??0LegacyBlock@@QAE@XZ; this deleted seed-file spelling is not emitted as the current symbol and is not independently established as the original class", "pinned scan: no direct caller edge or absolute-address data reference to 4C8110 was found"], "reason": "Cover only the constructor's concrete four-dword initialization through a typed source view. Preserve its original span and claim; keep field roles, complete original class name, allocation extent and runtime lifetime unknown. The view is not merged with the separate time-range receiver."}
// symbol    ??0RangePrefixInitializationView@@QAE@XZ
RangePrefixInitializationView::RangePrefixInitializationView() {
    memset(&zeroed_dword_4_, 0, sizeof(zeroed_dword_4_));
    unknown_dword_0_ = 0;
    unknown_dword_8_ = 0;
    unknown_dword_C_ = 0;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x004C8140 nullsub_223 0x004C8140-0x004C8141 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x004C8140-0x004C8141 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x004C8110 sub_4c8110 (src/time.cpp) and 0x004C8150 sub_4c8150 (src/stringstruct.cpp); band 0x004C8070..0x004C8140 holds the time.cpp/word_state.cpp leaf helpers; owner inferred from the immediately preceding function"]}
// symbol    ?nullsub_223@@YAXXZ
// kind      game
void __cdecl nullsub_223() {
}


// ORIGINAL: 0x004C8150 sub_4c8150 0x004C8150-0x004C8188 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c8150.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?initialize@PathRecord@@QAEPAU1@XZ
PathRecord *PathRecord::initialize() {
    // Native 0x004C815D clears four bytes beginning at +4.
    memset(&unknown_04_, 0, sizeof(unknown_04_));
    unknown_00_ = 0;
    unknown_08_ = 0;
    unknown_0C_ = 0;
    unknown_10_ = 0;
    unknown_14_ = 0;
    path_ = 0;
    directory_ = 0;
    name_ = 0;
    unknown_18_ = 0xfa;
    return this;
}

// ORIGINAL: 0x004C8190 sub_4c8190 0x004C8190-0x004C81B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c8190.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?close@PathRecord@@QAEXXZ
// Original close leaves the directory allocation and both freed pointer
// values intact. Do not turn this operation into an inferred RAII destructor.
void PathRecord::close() {
    if (path_ != 0) {
        ::operator delete(path_);
    }
    if (name_ != 0) {
        ::operator delete(name_);
    }
}

// ORIGINAL: 0x004C81C0 sub_4c81c0 0x004C81C0-0x004C8286 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_directory@PathRecord@@QAEXPAD@Z
void PathRecord::set_directory(char *directory) {
    if (directory_ != 0) {
        ::operator delete(directory_);
    }
    directory_ = 0;

    char buffer[256];
    unsigned int length = strlen(directory);
    strcpy(buffer, directory);
    if (buffer[length - 1] != '\\') {
        buffer[length] = '\\';
        buffer[length + 1] = 0;
        ++length;
    }
    ++length;
    directory_ = static_cast<char *>(::operator new(length));
    strcpy(directory_, buffer);

    if (path_ != 0) {
        const unsigned int path_length = strlen(path_);
        char *cursor = path_ + path_length;
        char *scan = cursor;
        while (scan != path_) {
            const char ch = *--scan;
            if (ch == '\\') {
                ++scan;
                break;
            }
        }
        strcat(buffer, scan);
        set_path(buffer);
    }
}

// ORIGINAL: 0x004C8290 sub_4c8290 0x004C8290-0x004C8348 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_path@PathRecord@@QAEXPAD@Z
void PathRecord::set_path(char *path) {
    if (path_ != 0) {
        ::operator delete(path_);
    }
    path_ = 0;
    if (path == 0) {
        path_ = 0;  // pinned 4C82BB repeats the clear on this exit
        return;
    }
    char buffer[256];
    if (directory_ != 0) {
        strcpy(buffer, directory_);
        char *slash = strrchr(path, '\\');
        if (slash == 0) {
            slash = path;
        } else {
            ++slash;
        }
        strcat(buffer, slash);
    } else {
        strcpy(buffer, path);
    }
    path_ = static_cast<char *>(::operator new(strlen(buffer) + 1));
    strcpy(path_, buffer);
}

// Historical seed called this an unknown Subject::roll and kept an IAT alias;
// that is not pinned class/API evidence. The actual source uses the SDK clock.
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C8350 sub_4c8350 0x004C8350-0x004C8396 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8350-0x004C8396: thiscall reads byte[this+4]&1; signed-compares dwords+8/+C; forms a wrapped unsigned inclusive difference before timeGetTime; divides unsigned; reloads +C after the call and adds the remainder", "pinned calls at 0x004C836D and 0x004C8384 target PE import slot 0x00669368, which uniquely resolves to WINMM timeGetTime; the normal SDK API is used, not an executable-address alias", "promoted historical seed src/recovered/units/004c8350.cpp retained the complete70-byte span and MISMATCH state; prior range-before-clock source-shape observation is preserved here for later restoration", "pinned scan: no direct caller edge or absolute-address data reference to 4C8350 was found", "source:src/time.cpp provisional endpoint member operations at 0x004C80C0/0x004C80E0 update the same +8/+C fields read by sample"], "reason": "Implement the observed time-based range read through a provisional receiver view, paired with 0x004C80C0/0x004C80E0 because those endpoint operations write the same +8/+C fields this body reads. Keep the separate RangePrefixInitializationView at 0x004C8110 unmerged. Preserve signed branch comparison, unsigned 32-bit range wrap, zero-divisor failure, the post-clock upper-field reload, and wrapped return addition. This relation does not prove original complete class identity, object extent, constructor, callers, or lifetime."}
// symbol    ?sample@TimeRangeReceiverView@@QAEHXZ
// Historical TRIED: the seed candidate calculated range before the clock call;
// the compiler scheduled the call earlier, retaining both endpoints and using
// another callee-saved register. A volatile range instead spilled through a
// frame. Those were compiler-shape findings, not evidence for a source class.
// The original adds the unsigned remainder to the reloaded upper endpoint;
// accumulating into the remainder (`sample += upper`) gives the image's
// `mov eax,[edi+0xC]` / `add eax,edx`, where a memcpy wrap helper did not.
int TimeRangeReceiverView::sample() {
    if ((enabled_flags_4_ & 1) == 0) {
        return 0;
    }
    // Unsigned DWORD arithmetic keeps the x86 32-bit range and result wrap.
    DWORD range;
    if (lower_bound_8_ > upper_bound_C_) {
        range = static_cast<DWORD>(lower_bound_8_) - upper_bound_C_ + 1;
    } else {
        range = static_cast<DWORD>(upper_bound_C_) - lower_bound_8_ + 1;
    }
    DWORD sample = timeGetTime() % range;
    sample += upper_bound_C_;
    return static_cast<int>(sample);
}

// ORIGINAL: 0x004C83A0 sub_4c83a0 0x004C83A0-0x004C83EB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c83a0.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_name@PathRecord@@QAEXPAD@Z
void PathRecord::set_name(char *name) {
    if (name_ != 0) {
        ::operator delete(name_);
    }
    if (name != 0) {
        const unsigned int length = strlen(name) + 1;
        void *fresh = ::operator new(length);
        name_ = static_cast<char *>(fresh);
        strcpy(name_, name);
    } else {
        name_ = 0;
    }
}

// ORIGINAL: 0x004C83F0 sub_4c83f0 0x004C83F0-0x004C840F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-ui-sound-helper-owner", "Pinned4C83F0..4C840F loads source-owned SoundChainHead90DB20, dispatches Sound virtual unload slot5 and reloads the shared head after callbacks; preserve return11.", "historical seed:src/recovered/004c83f0.cpp"], "reason": "Rehome genuine typed helper into its existing source owner, preserving original spans, historical claims and useful discoveries. No fixed executable address or manual virtual-call model."}
// symbol    ?unload_active_sound_chain@@YAHXZ
int __cdecl unload_active_sound_chain() {
    while (SoundChainHead != 0) {
        SoundChainHead->unload();
    }
    return 0x0B;
}
// ORIGINAL: 0x004C8410 ??0Patch@@QAE@XZ 0x004C8410-0x004C8442 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:4C8410..4C8442: stores root table66E444, volume+4=7F, pan+8=0, zeroes+C..+2F and clears+30; +34/+38 remain untouched", "pinned:Sound constructor4C6080 publishes66E444 before Sound table66E3C0; Sound destructor4C6120 and shared unwind path4C8450 restore the same root; Ambience/Wave/Voice constructors stage the same prefix", "src/sound.h SoundFadeState actual members and RECOVERY_MEMBER bindings account for fields+4..+38; the root's two fade slots select the shared typed SoundFadeState behavior", "promoted historical seed src/recovered/units/004c8410.cpp retains original catalog name ??0Patch@@QAE@XZ, full50-byte span, and BYTE_EXACT claim"], "reason": "Own the complete initialized 0x3C SoundFadeState prefix through its existing real base class. The old Patch catalog name is retained only as historical function identity; it does not establish a Patch class or hierarchy. Leave delay_ and fade_38_ untouched. Externalizing this formerly inline constructor changes compiler call placement, so native matching/restoration across Sound-derived constructors remains required."}
// symbol    ??0SoundFadeState@@QAE@XZ
SoundFadeState::SoundFadeState() {
  volume_ = 0x7F;
  pan_8_ = 0;
  memset(state_C_, 0, sizeof(state_C_));
  loop_flag_30_ = 0;
  // The original base does not initialize delay_ or fade_38_. Sound sets
  // the default interval after constructing this prefix.
}
// ORIGINAL: 0x004C8450 sub_4c8450 0x004C8450-0x004C8457 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8450..0x004C8456 stores root table 0x0066E444 through ECX and returns; the next NOP at 0x004C8457 is outside the mapped span", "pinned constructor EH continuations load saved receiver into ECX and tail-jump to 0x004C8450; the helper has its own entry and RET", "src/sound.h declares the genuine protected nonvirtual SoundFadeState destructor; SoundFadeState root table 0x0066E444 is staged by its real constructor", "historical Sound destructor mapping at 0x004C6120 included 0x004C8450..0x004C8457 and retains its BYTE_EXACT token as an obligation after this span is independently mapped"], "reason": "Bind the pinned independent ECX SoundFadeState base-stage unwind entry to the genuine protected nonvirtual SoundFadeState destructor; keep the full historical Sound destructor claim as a restoration obligation after separating this generated entry."}
// symbol    ??1SoundFadeState@@IAE@XZ

// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ??_7VideoSoundState@@6B@ 0x004c8620
// Video playback, composed resources and Sound-derived audio ownership.
// The Sound prefix owns table 0x66E790 and its own deleting thunk 0x4C9360.
// Do not add an outer virtual deleter: that thunk calls prefix 0x4C86D0 only.

// The constructor below owns the emitted Ambience table and deleting entry.
// ORIGINAL: 0x00448390 sub_448390 0x00448390-0x004483B1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Ambience constructor 0x004C8460 publishes final table 0x66E664 at 0x004C850F; slot 2 at 0x66E66C selects 0x00448390.", "This entry calls the Ambience complete-destructor tail 0x004483C0 and conditionally deletes storage according to the original flag bit.", "The earlier table 0x66E538 staged at 0x004C84EF is the Midi base view. Its slot 2 selects the distinct 0x004C9330, now mapped to the emitted Midi deleting symbol in src/sound.cpp.", "src/ambience.h leaves the derived destructor implicit over Midi. The compiler emits the Ambience deleting symbol with its constructor in src/sound.cpp; its class table slot selects that real generated lifetime."], "reason": "Bind the final Ambience deleting entry to its genuine compiler-generated derived lifetime, distinct from the Midi base entry. Preserve its original identity and historical exactness obligation."}
// symbol    ??_GAmbience@@UAEPAXI@Z
// kind      game

// Historical purpose/verification notes below describe the former flat seed.
// The current constructor uses actual base lifetimes, table staging and unwind.
/*
Purpose: Build the ambience. The original constructs in four vtable stages -
         0x0066E444 at 0x004C848A, 0x0066E3C0 at 0x004C84AC, 0x0066E538 at
         0x004C84EF, then its own 0x0066E664 at 0x004C850F - but only CRT
         memsets run between them, so the three intermediate installs are
         unobservable and, like the registered SEH frame, are omitted, keeping
         only the final Ambience publication. A device dispatch of the 0x3E8
         default through slot 0 at 0x004C84DB is provably dead - the device
         slot at 0x3C is zeroed at 0x004C84B8 and the memset in between covers
         0x40..0x43 only, so the reload at 0x004C84D4 cannot see anything else
         - and is omitted under the same policy. The net field state: full
         volume, zeroed regions, a 1000ms default at 0x38, flag dword 8, flag
         byte 1, and Sound::set_type run with type 5. Unlike Wave, whose 0x54
         clear is four bytes wide, this one is a single byte, so 0x55..0x57
         keep whatever the storage held - and nothing at or above 0x58 is
         written at all, which is what pins the shared base's extent there.
// ORIGINAL: 0x004C8460 ??0Ambience@@QAE@XZ 0x004C8460-0x004C852D;0x00659F4C-0x00659F5E BYTE_EXACT
// TRIED: dropping `volatile` off `object`/`bytes` measured 0/72
//   agreeing both ways (the function diverges at instruction 0, the
//   omitted SEH prologue, before reaching these stores), but it shrank
//   the compiled body from 31 to 25 instructions - the optimiser folds
//   the documented `object[0x40/4] |= 8` / `bytes[0x54] |= 1`
//   read-modify-writes into plain immediate stores, which the
//   Verification note above says the image does NOT do (it genuinely
//   reloads at 0x004C84FA/0x004C84FC). Reverted; volatile is load-bearing
//   here, not incidental.
// symbol    ??0Ambience@@QAE@XZ
// size      230 bytes
// prototype void (__thiscall ??0Ambience@@QAE@XZ)(Ambience* this)
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C84E2
Return Value: n/a (the redirect answers the object pointer, as the original
              does in eax)
Status: Complete
Verification note: 0x0066E664 has exactly one writer in the whole image, this
         constructor, and no reader on the teardown path, which is why the
         already-recovered ~Ambience at 0x004C7670 starts its descent one
         stage lower at 0x0066E538. That is not a missing stage: 0x0066E664
         differs from 0x0066E538 in three slots only - the deleting destructor
         at 0x08, load at 0x10 and unload at 0x14 - and 0x004C7670 is the
         base-level destructor, reached from both slot 0x08 of 0x0066E664 and
         the global's ??__F thunk at 0x004455E0 without a vtable store of its
         own, MSVC having elided a store that 0x004C7670 overwrites on its
         first instruction. The two bodies are consistent.
Verification note: the sweep leaves exactly two survivors and both are
         equivalent mutants that no assertion can separate, because the
         original's own shape makes them so. Widening the 0x0C..0x2F loop by
         one iteration lands on 0x30, which the very next statement stores
         zero to anyway; and seeding the 0x54 byte with 1 instead of 0 washes
         out under the `|= 1` two statements later. Both are transcribed the
         way the bytes read - a memset then a separate store, and a memset
         then a read-modify-write - rather than folded into whatever would
         make the sweep look cleaner.
Verification note: the base-level constructor at 0x004C75B0 is byte-identical
         to this one through the 0x0066E538 publication and then stops - it
         neither publishes 0x0066E664 nor calls set_type. Recovering it later
         must duplicate this shape rather than share a helper with it, or a
         mutation sweep of either loses its signal.
*/
Ambience::Ambience() {
    set_type(5);
}
#pragma optimize("y", off)

// ORIGINAL: 0x004C8530 ?load@Ambience@@QAEHPBD@Z 0x004C8530-0x004C85EF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/unrecovered/004c8530.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// Defined here, not in ambience.cpp: the image inlines set_mode (byte
// parameter, stored in the dead filename slot) into this body.
// symbol    ?load@Ambience@@UAEHPBD@Z
int Ambience::load(const char *filename) {
    char path[256];
    strcpy(path, filename);
    const char *const found = filefind_get(path);
    if (!found) return 10;
    if (device_) unload();
    if (!device_) {
        if (!WaveDeviceReleaseGuard) return 1;
        const int error = SoundXOrdinal1CreateSound()(&device_, found, 5);
        if (error) return error;
    }
    // The pinned mode5 factory returns the common Sound object whose shared
    // constructor and 75-slot table match Midi; the child address is unchanged.
    static_cast<Midi *>(device_)->set_mode(field_54_ & 1);
    return Sound::load(found);
}
#pragma optimize("", on)

// ORIGINAL: 0x004C85F0 ?unload@Ambience@@QAEXXZ 0x004C85F0-0x004C861C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:291c01ea:src/recovered/units/004c85f0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?unload@Ambience@@UAEHXZ
int Ambience::unload() {
    Midi *const delegate = static_cast<Midi *>(device_);
    if (delegate) delegate->unload();
    detach();
    device_ = nullptr;
    flags_40_ &= ~1u;
    return 0;
}

// ORIGINAL: 0x004C8620 sub_4c8620 0x004C8620-0x004C86C9;0x00659F5E-0x00659F70 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf native636400/636530 member construction/destruction;66E790 prefix table and4C9360->4C86D0.", "native636960 full loader jump table636F9C;637530 SDK and Indeo packet calls.", "Native spans and all historical claims/lessons retained; close6365B0 broad-span and4C8910 truncated-span debts remain explicit."], "reason": "Video owns composed Sound prefix, real Palette/queue/Buffer members, SDK media resources and native observed source globals; scalar deletion belongs only to the prefix."}
// symbol    ??0VideoSoundState@@QAE@XZ
// TRIED: byte-exact - the SEH frame prologue (push -1/handler; mov fs:[0]) has no C++ source form to reproduce without a real try/catch that this Sound-constructor body doesn't need (documented wall). Landed as a __thiscall ctor-shaped member returning `this`, with the progressive vtable_storage_ reassignment (0x66e444 -> 0x66e3c0 -> 0x66e790) and the self-clobbering dead `if (device_ != 0)` check (device_ is zeroed immediately before the read) transcribed as-is.
VideoSoundState::VideoSoundState() : Sound() {
  // Native 0x4C86AB installs the derived table before 0x4C86B1 calls
  // Sound::set_type(4). Compiler-owned construction supplies these table stages.
  Sound::set_type(4);
}

// ORIGINAL: 0x004C86D0 sub_4c86d0 0x004C86D0-0x004C87A2;0x00659F70-0x00659F8A FILE BYTE_EXACT
// symbol    ??1VideoSoundState@@UAE@XZ
// Historical SHARED_TAIL observation below predates the independent source identity for 0x004C8450; retain it as prior trial context, not a current span classification.
// TRIED: literal `call 0x64557f`/SEH __try/__finally reproduction - the span at 0x004C8450-0x004C8457 is COMDAT-folded with src/wave.cpp's Wave::~Wave and src/ambience.cpp's Ambience::~Ambience (both list it in their own spans too), so classify_body reports SHARED_TAIL for this address regardless of body content - no per-function verdict is well defined here, confirmed by running the bare verifier against the empty placeholder before writing anything.
VideoSoundState::~VideoSoundState() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    delete device;
    device_ = 0;
  }
  // Sound::~Sound then owns filename/device-release/chain teardown and the
  // compiler emits base/root vtable descent. Do not duplicate Sound cleanup.
}

// ORIGINAL: 0x004C87B0 sub_4c87b0 0x004C87B0-0x004C8839 FILE BYTE_EXACT
// symbol    ?configure_streams@VideoSoundState@@UAEHPBX00H@Z
int VideoSoundState::configure_streams(const void *audio_stream,
                                       const void *video_stream,
                                       const void *audio_format,
                                       int flags) {
  int result = 0;
  if (device_ == 0) {
    if (!WaveDeviceReleaseGuard) {
      return 1;
    }
    result = (SoundXOrdinal1CreateSound())(&device_, 0, 4);
    if (result != 0) {
      return result;
    }
  }

  // Re-read the owned device after the factory branch, as at 0x004C87E9.
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);

  // Native 0x4C87E9..0x4C87ED allows success without a device object.
  if (device == 0) return result;

  result = device->configure_streams(audio_stream, video_stream,
                                     audio_format, flags);
  if (result != 0) {
    if (WaveDeviceReleaseGuard) {
      (SoundXOrdinal2DeleteSound())(device_);
    }
    device_ = 0;
  }

  attach();
  return result;
}

// ORIGINAL: 0x004C8840 sub_4c8840 0x004C8840-0x004C8888 FILE BYTE_EXACT
// symbol    ?release@VideoSoundState@@UAEHXZ
// LEVER: the g_0090db28 function pointer is __cdecl, not __stdcall - the original has `add esp,4` after that call; __stdcall dropped it and cost a MISMATCH at the trailing `add`/`jmp`.
int VideoSoundState::release() {
  if (device_ != 0) {
    if (static_cast<VideoSoundDevice *>(device_)->is_playing() == 0) {
      if (WaveDeviceReleaseGuard) {
        (SoundXOrdinal2DeleteSound())(device_);
      }
    } else {
      // The query can call back into the owner. Native 4C886B re-reads
      // the field; do not retain the pre-query device across that callback.
      static_cast<VideoSoundDevice *>(device_)->release();
    }
    device_ = 0;
  }
  detach();
  return 0;
}

// ORIGINAL: 0x004C8890 sub_4c8890 0x004C8890-0x004C88AC FILE BYTE_EXACT
// symbol    ?set_target_frame@VideoSoundState@@UAEHH@Z
int VideoSoundState::set_target_frame(int frame) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    device->set_target_frame(frame);
  }
  // Native wrapper 0x4C8890 discards the device result and returns zero.
  return 0;
}

// ORIGINAL: 0x004C88B0 sub_4c88b0 0x004C88B0-0x004C88BF BYTE_EXACT
// symbol    ?stop@VideoSoundState@@UAEHXZ
int VideoSoundState::stop() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    device->stop();
  }
  return 0;
}

// ORIGINAL: 0x004C88C0 sub_4c88c0 0x004C88C0-0x004C88E0 BYTE_EXACT
// symbol    ?acquire_audio_buffer@VideoSoundState@@UAEPAXH@Z
void *VideoSoundState::acquire_audio_buffer(int bytes) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->acquire_audio_buffer(bytes);
  }
  return 0;
}

// ORIGINAL: 0x004C88E0 sub_4c88e0 0x004C88E0-0x004C8903 FILE BYTE_EXACT
// symbol    ?submit_audio_buffer@VideoSoundState@@UAEHPAX@Z
int VideoSoundState::submit_audio_buffer(void *record) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->submit_audio_buffer(record);
  }
  return 0x13;
}

// ORIGINAL: 0x004C8910 sub_4c8910 0x004C8910-0x004C8923 FILE BYTE_EXACT
// symbol    ?current_frame@VideoSoundState@@UAEHXZ
// TRIED: the tail-called vtable slot37 matches exactly (mov/test/je/mov/jmp, 5/5 mnemonics), so the divergence is entirely the null branch: the original's `return -1` JUMPS OUT of this function into a shared tail block at 0x4C891F, four bytes past its own 15, and no C++ spelling reaches another function's interior. This body inlines `or eax,-1; ret` instead and is 4 bytes long. Not a lever - it names why the remainder cannot close from here.
// Original4C8915's null branch selects4C891F..4C8923 directly. That
// four-byte return tail belongs to this complete source behavior; the IDA
// underflow label does not establish another function. The historical TRIED
// interpretation above is retained, not a current boundary proof.
int VideoSoundState::current_frame() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->current_frame();
  }
  // Native4C891F..22: no device is a -1 frame, not zero.
  return -1;
}

// ORIGINAL: 0x004C8930 sub_4c8930 0x004C8930-0x004C8942 BYTE_EXACT
// symbol    ?frame_status@VideoSoundState@@UAEHXZ
int VideoSoundState::frame_status() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->frame_status();
  }
  return 0;
}

// ORIGINAL: 0x004C8950 sub_4c8950 0x004C8950-0x004C8953 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; VideoSoundState primary table 0x0066E790 slot 29 at 0x0066E804 relocates to 0x004C8950", "the pre-proposal src/video.cpp RECOVERY_SHARED record already bound this exact span to VideoSoundState::get_time; this replaces that one shared record with the canonical identity", "src/sound.cpp owns the actual int VideoSoundState::get_time() method body returning zero; no Time class or global timer ownership is inferred", "historical catalog facts retained: sub_4c8950 is a three-byte zero-return leaf; its name, span and claim state are carried from the parsed source record by the native writer", "retired source symbol retained as historical evidence: ?nullsub_004c8950_redirect@@YIHPAX0@Z"], "reason": "Promote the existing VideoSoundState::get_time shared-body mapping to the unique canonical identity. Keep its real typed method and avoid a duplicate annotation or adapter."}
// symbol    ?get_time@VideoSoundState@@UAEHXZ
// kind      game
int VideoSoundState::get_time() {
  return 0;
}

// RECOVERY_LITERAL: v1 0x00687B24 0x00687B2D ??_C@_08BBLG@Voice?5Rx?$AA@ 0x004C8960
// ORIGINAL: 0x004C8960 ??0VoiceRx@@QAE@XZ 0x004C8960-0x004C8A43;0x00659F8A-0x00659FA4 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 4C8960/4C8CC0 constructors, 4C8A50/4C8DB0 destructors and 66E82C/66E8C4 tables; native VC6 emits Sound base at0, Rx extent54, Tx extent58 and own flag54.", "631A60 consumes record18/20 and returns the same record; soundx.dll kind7 slots34/35 allocate MMIOINFO48 and enforce pointer identity. Shared cleanup/EH spans remain historical obligations."], "reason": "Voice Rx and Tx genuinely derive from Sound; natural lifecycle stages and typed DLL device contracts replace raw wrappers. Tx returns and consumes the exact borrowed SDK MMIOINFO pointer, not an integer handle."}
// symbol    ??0VoiceRx@@QAE@XZ
VoiceRx::VoiceRx() {
    char name[] = "Voice Rx";
    Sound::set_type(6);
    fname_ = static_cast<char *>(::operator new(0xA));
    strcpy(fname_, name);
}

// ORIGINAL: 0x004C8A50 ??1VoiceRx@@QAE@XZ 0x004C8A50-0x004C8B35;0x00659FA4-0x00659FBE FILE BYTE_EXACT
// symbol    ??1VoiceRx@@UAE@XZ
VoiceRx::~VoiceRx() {
    if (device_ != 0) {
        delete static_cast<VoiceRxDeviceInterface *>(device_);
        device_ = 0;
    }
    if (fname_ != 0) {
        ::operator delete(fname_);
        fname_ = 0;
    }
}

// ORIGINAL: 0x004C8B40 ?init@VoiceRx@@QAEHKPAXK@Z 0x004C8B40-0x004C8BC5 FILE BYTE_EXACT
// symbol    ?init@VoiceRx@@UAEHKPAXK@Z
// TRIED: explicit __thiscall on a plain function-pointer typedef (C4234); a member-function-pointer/union shim compiles. MISMATCH #4 'mov' vs 'push' remains open.
int VoiceRx::init(unsigned long size, void *context, unsigned long flags) {
    int result = 0;
    if (device_ == 0) {
        if (WaveDeviceReleaseGuard == 0) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, 0, 6);
        if (result != 0) {
            return result;
        }
    }
    VoiceRxDeviceInterface *const provider =
        static_cast<VoiceRxDeviceInterface *>(device_);
    if (provider == 0) {
        return result;
    }
    result = provider->init(size, context, flags);
    if (result != 0) {
        if (WaveDeviceReleaseGuard != 0) {
            SoundXOrdinal2DeleteSound()(device_);
        }
        device_ = 0;
    }
    // Original attaches after a device-init attempt even when that attempt
    // failed and cleared the device. Do not add a device-presence guard.
    attach();
    return result;
}

// ORIGINAL: 0x004C8BD0 ?release@VoiceRx@@QAEHXZ 0x004C8BD0-0x004C8C18 FILE BYTE_EXACT
// symbol    ?release@VoiceRx@@UAEHXZ
int VoiceRx::release() {
    if (device_ != 0) {
        if (static_cast<VoiceRxDeviceInterface *>(device_)->is_playing() == 0) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device_);
            }
        } else {
            static_cast<VoiceRxDeviceInterface *>(device_)->release();
        }
        device_ = 0;
    }
    detach();
    return 0;
}

/*
Purpose: ?start@VoiceRx@@QAEHXZ - dispatch to slot 0x88 of the object at
         0x3c when it is present, and answer 0x0 either way: both
         paths land on the same zeroing, so the delegate's own result
         never reaches the caller.
// ORIGINAL: 0x004C8C20 ?start@VoiceRx@@QAEHXZ 0x004C8C20-0x004C8C32 BYTE_EXACT
// symbol    ?start@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?start@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C29
Return Value: 0x0, whether the delegate ran or not
Status: Complete
*/
int VoiceRx::start() {
    if (device_ != 0) {
        static_cast<VoiceRxDeviceInterface *>(device_)->start();
    }
    return 0;
}

/*
Purpose: ?stop@VoiceRx@@QAEHXZ - dispatch to slot 0x20 of the object at
         0x3c when it is present, and answer 0x0 either way: both
         paths land on the same zeroing, so the delegate's own result
         never reaches the caller.
// ORIGINAL: 0x004C8C40 ?stop@VoiceRx@@QAEHXZ 0x004C8C40-0x004C8C4F BYTE_EXACT
// symbol    ?stop@VoiceRx@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?stop@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C49
Return Value: 0x0, whether the delegate ran or not
Status: Complete
*/
int VoiceRx::stop() {
    if (device_ != 0) {
        static_cast<VoiceRxDeviceInterface *>(device_)->stop();
    }
    return 0;
}

/*
Purpose: ?unload@VoiceRx@@QAEHXZ - forward 0 argument(s) to slot 0x14 of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C8C50 ?unload@VoiceRx@@QAEHXZ 0x004C8C50-0x004C8C62 BYTE_EXACT
// symbol    ?unload@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?unload@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceRx::unload() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->unload();
    }
    return 0x13;
}

/*
Purpose: ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C8C70 ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z 0x004C8C70-0x004C8C93 BYTE_EXACT
// symbol    ?add_buffer@VoiceRx@@UAEHPAU_MMIOINFO@@@Z
// size      35 bytes
// prototype int (__thiscall ?add_buffer@VoiceRx@@QAEHPAU_MMIOINFO@@@Z)(VoiceRx* this, _MMIOINFO*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8C80
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceRx::add_buffer(_MMIOINFO *buffer) {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->add_buffer(buffer);
    }
    return 0x13;
}

/*
Purpose: ?get_buffer_size@VoiceRx@@QAEHXZ - forward 0 argument(s) to slot 0x90 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C8CA0 ?get_buffer_size@VoiceRx@@QAEHXZ 0x004C8CA0-0x004C8CB2 BYTE_EXACT
// symbol    ?get_buffer_size@VoiceRx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_buffer_size@VoiceRx@@QAEHXZ)(VoiceRx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int VoiceRx::get_buffer_size() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->get_buffer_size();
    }
    return 0;
}

// RECOVERY_LITERAL: v1 0x00687B30 0x00687B39 ??_C@_08GGEJ@Voice?5Tx?$AA@ 0x004C8CC0
/*
Purpose: Build a VoiceTx: full volume, centred pan, cleared regions, a
         1000ms default fade, the "Voice Tx" name on the CRT heap, and
         type 7 published through the device slot.
// ORIGINAL: 0x004C8CC0 ??0VoiceTx@@QAE@XZ 0x004C8CC0-0x004C8DAF;0x00659FBE-0x00659FD8 BYTE_EXACT
// symbol    ??0VoiceTx@@QAE@XZ
// size      239 bytes
// prototype void (__thiscall ??0VoiceTx@@QAE@XZ)(VoiceTx* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C61E0 0x00645460 0x0064558A 0x006465F0
// PROMOTED from src/recovered/units/004c8cc0.cpp. With VoiceTx : public Sound
// the three vtable stages (0x66E444 -> 0x66E3C0 -> 0x66E8C4) are the
// compiler's own base-construction sequence. Sound owns the inherited
// initialization and default fade setup; VoiceTx clears its own flags at
// original 0x004C8D49..0x004C8D5D before allocating the display name.
Return Value: n/a
Status: Complete
*/
VoiceTx::VoiceTx() {
    memset(&flags_54_, 0, sizeof(flags_54_));
    char name[] = "Voice Tx";
    fname_ = static_cast<char *>(::operator new(0xA));
    strcpy(fname_, name);
    Sound::set_type(7);
}

// ORIGINAL: 0x004C8DB0 ??1VoiceTx@@QAE@XZ 0x004C8DB0-0x004C8EA0;0x00659FD8-0x00659FF2 FILE BYTE_EXACT
// symbol    ??1VoiceTx@@UAE@XZ
VoiceTx::~VoiceTx() {
    if (device_ != 0 && WaveDeviceReleaseGuard != 0) {
        SoundXOrdinal2DeleteSound()(device_);
    }
    device_ = 0;
    if (fname_ != 0) {
        ::operator delete(fname_);
        fname_ = 0;
    }
}

/*
Purpose: Drop the Sound-level device (vtable slot 0x38) and forget it, so a
         later init re-wraps; the answer is discarded - release always
         answers 0.
// ORIGINAL: 0x004C8EA0 ?release@VoiceTx@@QAEXXZ 0x004C8EA0-0x004C8EAF BYTE_EXACT
// symbol    ?release@VoiceTx@@UAEHXZ
// size      15 bytes
// prototype int (__thiscall ?release@VoiceTx@@QAEXXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8EA9
// LEVER: the catalogue spells this `QAEXXZ` (void) - withdrawn here. Both
//        epilogues are `xor eax, eax; ret`, the shape that CORRECTED unload's
//        catalogue name to int. The dispatch differs from Sound::release only
//        in the default: the base returns 0x14 when no device is wrapped,
//        this returns 0. The ?voice_tx_release_redirect@@YIHPAX0@Z spelling
//        that held these bytes first was retired with the delegation_thunks
//        block when this member superseded it.
Return Value: 0, always
*/
int VoiceTx::release() {
    if (device_ != 0) {
        static_cast<VoiceTxDeviceInterface *>(device_)->release();
    }
    return 0;
}

/*
Purpose: Begin voice capture: disable the wave device, start the wave-in
         device, arm the Sound-level device through its own vtable slot 0x90,
         and only then light the started bit at 0x54.
// ORIGINAL: 0x004C8EB0 ?start@VoiceTx@@QAEHXZ 0x004C8EB0-0x004C8EF4 BYTE_EXACT
// symbol    ?start@VoiceTx@@UAEHXZ
// size      68 bytes
// prototype int (__thiscall ?start@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51D0 0x004C5B60
// indirect  0x004C8EDE
// LEVER: PROMOTED out of src/recovered/004c8eb0.cpp, which reached the
//        fields as `reinterpret_cast<char *>(this) + 0x54/0x3c` and the
//        device dispatch through a 37-slot local shim. The named-member
//        spelling reproduces the same bytes: flags_54_ is the dword at 0x54
//        (bit 0, and the image TESTS it as a byte and ORs bit 0 as a byte
//        inside a dword store), device_ is Sound's at 0x3C, and the armed
//        dispatch is slot 36 of the device's live vtable - 0x90, the slot
//        this class's own start occupies in VoiceTx's table 0x0066E8C4.
Return Value: 0, or the first failing stage's answer
*/
int VoiceTx::start() {
    if ((flags_54_ & 1) == 0) {
        int result = WaveDeviceGlobal.disable();
        if (result != 0) return result;
        result = g_WAVE_IN_DEVICE.start_record();
        if (result != 0) return result;
        if (device_ != 0) {
            result = static_cast<VoiceTxDeviceInterface *>(device_)->start();
            if (result != 0) return result;
        }
        flags_54_ |= 1;
    }
    return 0;
}

/*
Purpose: Stop voice capture in the reverse order of start, and tear the
         started bit down first.
// ORIGINAL: 0x004C8F00 ?stop@VoiceTx@@QAEHXZ 0x004C8F00-0x004C8F3C BYTE_EXACT
// symbol    ?stop@VoiceTx@@UAEHXZ
// size      60 bytes
// prototype int (__thiscall ?stop@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C51C0 0x004C5B70
// indirect  0x004C8F33
// LEVER: PROMOTED out of src/recovered/004c8f00.cpp the same way as start
//        above. The started-bit clear is a full DWORD and-cycle
//        (`and ecx, 0xfffffffe`) while start's set is a byte `or al, 1`
//        inside a dword read-modify-write - both fall out of the one
//        `flags_54_` dword with plain `&=` / `|=`.
Return Value: 0, or the failing stage's answer
*/
int VoiceTx::stop() {
    if ((flags_54_ & 1) != 0) {
        int result = g_WAVE_IN_DEVICE.end_record();
        if (result != 0) return result;
        flags_54_ &= ~1u;
        WaveDeviceGlobal.enable();
        if (device_ != 0) {
            return static_cast<VoiceTxDeviceInterface *>(device_)->stop();
        }
    }
    return 0;
}

/*
Purpose: Detach from the Sound-level device, stopping voice first, and hand
         the device to the release hook while the guard dword says the hook
         is live - the same hook and guard ~Sound consults.
// ORIGINAL: 0x004C8F40 ?unload@VoiceTx@@QAEHXZ 0x004C8F40-0x004C8F70 BYTE_EXACT
// symbol    ?unload@VoiceTx@@UAEHXZ
// size      48 bytes
// prototype int (__thiscall ?unload@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C8F45 0x004C8F4C 0x004C8F5C
// LEVER: PROMOTED out of src/recovered/004c8f40.cpp. The first two
//        dispatches are VIRTUAL SELF-CALLS of slots 8 and 14 - spelled
//        `stop(); release();`, which is why the class model matters here:
//        the shim spelling could not see that they are this class's own
//        stop and release.
Return Value: 0, always
*/
int VoiceTx::unload() {
    stop();
    release();
    if (WaveDeviceReleaseGuard != 0) {
        SoundXOrdinal2DeleteSound()(device_);
    }
    device_ = 0;
    return 0;
}

// ORIGINAL: 0x004C8F70 ?init@VoiceTx@@QAEHKPAXK@Z 0x004C8F70-0x004C9007 FILE
// symbol    ?init@VoiceTx@@UAEHKPAXK@Z
int VoiceTx::init(unsigned long size, void *context, unsigned long flags) {
    int result = 0;
    unload();
    if (device_ == 0) {
        if (WaveDeviceReleaseGuard == 0) {
            return 1;
        }
        result = SoundXOrdinal1CreateSound()(&device_, 0, 7);
        if (result != 0) return result;
    }
    if (device_ != 0) {
        result = static_cast<VoiceTxDeviceInterface *>(device_)->init(size, context, flags);
        if (result != 0 && result != 0x10) {
            if (WaveDeviceReleaseGuard != 0) {
                SoundXOrdinal2DeleteSound()(device_);
            }
            device_ = 0;
        }
        if (device_ != 0) {
            attach();
        }
    }
    return result;
}

/*
Purpose: ?get_nbuffers@VoiceTx@@QAEHXZ - forward 0 argument(s) to slot 0x94 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C9010 ?get_nbuffers@VoiceTx@@QAEHXZ 0x004C9010-0x004C9022 BYTE_EXACT
// symbol    ?get_nbuffers@VoiceTx@@UAEHXZ
// size      18 bytes
// prototype int (__thiscall ?get_nbuffers@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
int VoiceTx::get_nbuffers() {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->get_nbuffers();
    }
    return 0;
}

/*
Purpose: ?get_next_buffer@VoiceTx@@QAEHXZ - forward 0 argument(s) to slot 0x88 of the
         object at +0x3c, answering 0x0 when that object is absent.
// ORIGINAL: 0x004C9030 ?get_next_buffer@VoiceTx@@QAEHXZ 0x004C9030-0x004C9042 BYTE_EXACT
// symbol    ?get_next_buffer@VoiceTx@@UAEPAU_MMIOINFO@@XZ
// size      18 bytes
// prototype int (__thiscall ?get_next_buffer@VoiceTx@@QAEHXZ)(VoiceTx* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the delegate's, or 0x0 when the member is null
Status: Complete
*/
_MMIOINFO *VoiceTx::get_next_buffer() {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->get_next_buffer();
    }
    return 0;
}

/*
Purpose: ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z - forward 1 argument(s) to slot 0x8c of the
         object at +0x3c, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C9050 ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z 0x004C9050-0x004C9073 BYTE_EXACT
// symbol    ?return_buffer@VoiceTx@@UAEHPAU_MMIOINFO@@@Z
// size      35 bytes
// prototype int (__thiscall ?return_buffer@VoiceTx@@QAEHPAU_MMIOINFO@@@Z)(VoiceTx* this, _MMIOINFO*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C9060
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int VoiceTx::return_buffer(_MMIOINFO *buffer) {
    if (device_ != 0) {
        return static_cast<VoiceTxDeviceInterface *>(device_)->return_buffer(buffer);
    }
    return 0x13;
}

/*
Purpose: The sound-DLL version complaint box. load_sound_dll shows it through
         whenever JACKAL.DLL's exports disagree with the game's sound headers;
         the strings name them plainly ("Sound Version Warning", "The sound
         header files used by jackal do not match..."). One BasePop local
         carries the whole 0x3230-byte frame, built by the real constructor
         and torn down by the real destructor; the two copies of the caller's
         strings are the 0x100/0xe0 buffers the frame sits beside.
// ORIGINAL: 0x004C9080 sub_4c9080 0x004C9080-0x004C92CF;0x00659FF2-0x0065A090 FILE
// symbol    _sound_version_warning
// TRIED: the teardown tail is ~BasePop's INLINE EXPANSION - the image's
// compiler ran the destructor body at the destruction site (staged-vtable
// refreshes, then Spot/Dialogs/Dialog/both StringStructs/both FlatButtons/
// Heap/GraphicWin in reverse order) under the fs:[0] frame - so this body
// cannot go BYTE_EXACT until ~BasePop is spellable where VC6 inlines it,
// which is the BasePop class pass. Landed as the real-locals spelling: the
// same constructor, strings, start/exec sequence, and one call to the
// ~BasePop forwarder instead of the expansion. The flat-buffer transcription
// (the retired src/unrecovered/004c9080.cpp) reproduced the tail op-by-op
// with reinterpret_casts but wore the scaffold style and lost the SEH
// prologue; it stays in git history as the byte map of what the expansion
// must produce.
// size      749 bytes
// prototype
// callers   1   call targets   16
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401060 0x00402970 0x00406910 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600860 0x00600F00 0x00601BF0 0x00602600 0x00607040 0x00607DA0 0x00608E10 0x00645460 0x00645550
Return Value: none
Status: Complete
*/
extern "C" void __cdecl sound_version_warning(const char *text, const char *caption) {
    BasePop popup;
    char message[0x100];
    strcpy(message, text);
    char caption_buffer[0xe0];
    strcpy(caption_buffer, caption);
    if (popup.start(const_cast<char *>("jackal"),
                    "VERSIONCHECK_SOUND_HEADER", -1, nullptr, 0,
                    nullptr) == 0) {
        popup.exec(0, 0);
    }
}
/*
Purpose: The compiler-generated scalar deleting destructor: destroy the wave
         and, when bit 0 of the mode argument asks for it, return the storage
         to the game CRT heap.
// ORIGINAL: 0x004C9300 ??_GWave@@UAEPAXI@Z 0x004C9300-0x004C9321 FILE BYTE_EXACT
// symbol    ??_GWave@@UAEPAXI@Z
// size      33 bytes
// prototype void* (__thiscall ??_GWave@@UAEPAXI@Z)(Wave* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C67C0 0x0064557F
// LEVER: the qualified destructor call. ~Wave is virtual through ~Sound, so
//        `self->~Wave()` dispatched through the live vtable
//        (`mov eax,[esi]; call [eax+8]`) and dropped the frame pointer - 6 of
//        14. `self->Wave::~Wave()` names the class and emits the direct
//        `call` the image makes; 6/14 -> BYTE_EXACT 14/14.
Return Value: the object pointer
Status: Complete
*/
// The existing virtual destructor owns this compiler-generated entry.

// Wave construction at 0x004C676E installs this table. Its derived slots
// 33..56 are also present in Effect's table; inherited placeholders still
// need recovery, so no complete-table acceptance is implied here.
// RECOVERY_GENERATED: v1 vtable 0x0066E44C ? ??_7Wave@@6B@ 0x004C66E0


// ORIGINAL: 0x004C9330 ??_GAmbience@@UAEPAXI@Z 0x004C9330-0x004C9351 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Midi constructor 0x004C75B0 publishes table 0x66E538 at 0x004C763F and returns without a later derived-table replacement.", "Pinned derived Ambience constructor 0x004C8460 stages the same Midi table 0x66E538 at 0x004C84EF, then publishes final Ambience table 0x66E664 at 0x004C850F.", "Midi table slot 2 at 0x66E540 selects 0x004C9330; final Ambience slot 2 at 0x66E66C instead selects 0x00448390. Both retain Midi slot 27 at 0x004C93E0.", "0x004C9330 directly calls full destructor 0x004C7670, recovered as Midi::~Midi in src/sound.cpp. The distinct Ambience deleting entry calls 0x004483C0, the five-byte tail to the same base destructor.", "The real Midi : Sound and Ambience : Midi hierarchy emits distinct deleting symbols. No authored deleting wrapper, copied body, class-layout change or claim grant is introduced.", "Earlier shared-Ambience binding at master 2b499fd9c was an emission observation, not original class-stage proof; the intermediate table does not establish final Ambience ownership. Historical labels and exactness obligations remain unchanged."], "reason": "Bind this deleting entry to the genuine Midi lifetime stage, not the later Ambience stage. Preserve the original catalog label and historical claim while correcting current class-symbol ownership."}
// symbol    ??_GMidi@@UAEPAXI@Z
// kind      game


// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381 FILE BYTE_EXACT
// symbol    ??_GVideoSoundState@@UAEPAXI@Z
// Compiler-owned prefix scalar destructor mapping anchor.

// RECOVERY_GENERATED: v1 vtable 0x66e82c 0x66e8c4 ??_7VoiceRx@@6B@ 0x4c8960
// RECOVERY_GENERATED: v1 vtable 0x66e8c4 0x66e95c ??_7VoiceTx@@6B@ 0x4c8cc0
// Slot2 is compiler weak _E with library-search mode2 and physical _G fallback;
// that observation does not establish final linked table selection.

// It forwards the device's slot 37 result verbatim, or zero when no device
// exists; this is the operation Net's receive worker uses as its level.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C9390 sub_4c9390 0x004C9390-0x004C93A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004c9390.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?get_nbuffers@VoiceRx@@UAEHXZ
int VoiceRx::get_nbuffers() {
    if (device_ != 0) {
        return static_cast<VoiceRxDeviceInterface *>(device_)->get_nbuffers();
    }
    return 0;
}

// ORIGINAL: 0x004C93E0 sub_4c93e0 0x004C93E0-0x004C93E8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:Sound table 0x0066E3C0 slot 27 at 0x0066E42C selects 0x00645844, the pure-call path", "pinned:Midi table 0x0066E538 slot 27 at 0x0066E5A4 selects 0x004C93E0", "pinned:Ambience table 0x0066E664 preserves the Midi slot-27 selection at 0x004C93E0", "pinned:0x004C93E0 is mov eax,0x0b; ret 4; the source method has the observed one-dword thiscall stack cleanup and integer result", "source:Midi derives from Sound; Ambience derives from Midi; global Ambience g_AMBIENCE and faction ambience objects require a concrete Midi contract", "history:the complete existing 0x004C93E0 identity, original span, claim and adapter-origin note are transferred from retired src/field_accessors.cpp; no new claim is granted"], "reason": "Transfer the existing constant-return identity to Midi's real inherited virtual override. Sound's original table is pure; Midi and Ambience tables select this ABI-compatible method. The source API name is the current genuine owner, not a claim about the original undecorated catalog spelling."}
// symbol    ?set_attrib@Midi@@UAEHK@Z
// size      8 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
int Midi::set_attrib(unsigned long) { return 0x0B; }

// ORIGINAL: 0x004C93B0 ??_GVoiceRx@@UAEPAXI@Z 0x004C93B0-0x004C93D1 FILE BYTE_EXACT
// symbol    ??_GVoiceRx@@UAEPAXI@Z
// Compiler-owned VoiceRx scalar deletion mapping.
// ORIGINAL: 0x004C93F0 ??_GVoiceTx@@UAEPAXI@Z 0x004C93F0-0x004C9411 FILE BYTE_EXACT
// symbol    ??_GVoiceTx@@UAEPAXI@Z
// Compiler-owned VoiceTx scalar deletion mapping.
// RECOVERY_GENERATED: v1 vtable 0x0066E538 ? ??_7Midi@@6B@ 0x004C75B0

// This method belongs to the same Sound compiler context. Its original
// out-of-line call boundaries remain matching obligations; current source
// placement does not establish an original object-module boundary.


// The pinned cascade is a Midi trackset operation. 0x00683C2C's image
// initializer is -0x4B0; no wider allocation/lifetime is claimed here.
// RECOVERY_OBJECT: v1 0x00683C2C ?MidiTracksetTransposition@@3HA
int MidiTracksetTransposition = -0x4B0;

