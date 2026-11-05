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

// The sound module loader and Sound class share their real storage and
// compiler context here. Original object-file boundaries remain inferred.
#include "stdafx.h"
#include "sound.h"
#include "basepop.h"
#include "wave.h"
#include "wave_device.h"
#include "sounddevice.h"

// SDK dependency for releasing the dynamically loaded sound module.
// RECOVERY_IMPORT: v1 __imp__FreeLibrary@4 KERNEL32.dll name:FreeLibrary

// ORIGINAL: 0x004C5E50 ?load_sound_dll@@YAHXZ 0x004C5E50-0x004C5F68 FILE
// TRIED: do-while ROTATION is the remaining wall. The image keeps the plain
// bottom-tested loop (back-edge `jl` to the handle reload at 0x4c5e8c, entry
// `jmp` past it reusing the LoadLibraryA eax - the PRE skip); this tree's cl
// rotates the same do-while into a peeled first iteration (arg folded to
// `push 1`, `mov edi,1` after, bottom test inverted to `jge`), +5
// instructions, best 31/86 at /O2 /Gy /GR- /GX. Tried unchanged: for(;;)+break
// (canonicalizes to the same rotation), /O1 sets (7/86), /Oy- (10/86),
// /Ob0, /Oi-. Pointer-VALUE uses of a fixed address must be spelled as the
// literal - a binding used for its own value compiles a memory read of the
// binding where the image has the immediate - and the slot-zeroing loop
// needs one to lower to `rep stosd`. The three kernel32 entrypoints are the
// image's own imports (tools/iat_names.py: 0x00669120 LoadLibraryA,
// 0x00669124 GetProcAddress, 0x00669128 FreeLibrary), so the source calls
// the APIs directly and the IAT operands mask like every relocation. The
// image hoists the GetProcAddress entry into ebx for the walk (mov 0x4c5e7d,
// `call ebx` 0x4c5e9c); the direct call landed here emits a fresh
// `call dword ptr [__imp_GetProcAddress]` per iteration instead - the
// fn-pointer local that reproduced the hoist measured 33/86 against this
// spelling's score below.
// Semantically equivalent: same stores, same calls, same guards, same
// failure paths.
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
        void **procs = SoundDllProcs;
        void **slot = procs;
        int index = 0;
        do {
            void *proc = reinterpret_cast<void *>(GetProcAddress(
                SoundDllModule, MAKEINTRESOURCEA((index + 1) & 0xffff)));
            *slot = proc;
            if (proc == 0) {
                HMODULE handle = SoundDllModule;
                WaveDeviceReleaseGuard = 0;
                if (handle != 0) {
                    FreeLibrary(handle);
                    SoundDllModule = 0;
                }
                WaveDeviceReleaseGuard = 0;
                for (int i = 0; i < 0xb; ++i) {
                    procs[i] = 0;
                }
                return 1;
            }
            ++slot;
            ++index;
        } while (index < 11);

        unsigned int version;
        if (SoundDllModule == 0) {
            version = 0;
        } else {
            version = SoundXOrdinal11Version()();
        }
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

// ORIGINAL: 0x004C5D90 ?release_sound@@YAHXZ 0x004C5D90-0x004C5E42 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/unrecovered/004c5d90.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "sound.cpp; sound.h adds only a friend declaration for existing protected Sound::device_/fname_ and the free-function prototype. Calls the current SoundDllProcs entries, Sound::detach, existing device release methods and unload_sound_dll. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
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


// ORIGINAL: 0x004C9330 ??_GAmbience@@UAEPAXI@Z 0x004C9330-0x004C9351 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Midi constructor 0x004C75B0 publishes table 0x66E538 at 0x004C763F and returns without a later derived-table replacement.", "Pinned derived Ambience constructor 0x004C8460 stages the same Midi table 0x66E538 at 0x004C84EF, then publishes final Ambience table 0x66E664 at 0x004C850F.", "Midi table slot 2 at 0x66E540 selects 0x004C9330; final Ambience slot 2 at 0x66E66C instead selects 0x00448390. Both retain Midi slot 27 at 0x004C93E0.", "0x004C9330 directly calls full destructor 0x004C7670, recovered as Midi::~Midi in src/sound.cpp. The distinct Ambience deleting entry calls 0x004483C0, the five-byte tail to the same base destructor.", "The real Midi : Sound and Ambience : Midi hierarchy emits distinct deleting symbols. No authored deleting wrapper, copied body, class-layout change or claim grant is introduced.", "Earlier shared-Ambience binding at master 25f3ab658 was an emission observation, not original class-stage proof; the intermediate table does not establish final Ambience ownership. Historical labels and exactness obligations remain unchanged."], "reason": "Bind this deleting entry to the genuine Midi lifetime stage, not the later Ambience stage. Preserve the original catalog label and historical claim while correcting current class-symbol ownership."}
// symbol    ??_GMidi@@UAEPAXI@Z
// kind      game

#include "stdafx.h"
#include "sound.h"
#include "voice.h"
#include "general.h"
#include "wave.h"
#include "wave_device.h"    // Wave_Device in init_sound
#include "sounddevice.h"    // Midi_Device / Wave_In_Device in init_sound
#include "fx.h"
#include "dialog.h"
#include "sounddevice.h"
#include "net_class.h"
#include "basepop.h"
#include <cstring>

#include "midi.h"
#include "ambience.h"

// Keep the real base-to-derived lifecycle definitions visible together.
// Original constructors publish the SoundFadeState, Sound and derived tables
// around real memset calls; derived destruction contains the Sound stage inline.
// These are ordinary class lifetimes, not manual table or unwind substitutes.
// Standalone identities and shared unwind obligations remain mapped below.
#pragma function(memset)
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
// ORIGINAL: 0x004C66E0 ??0Wave@@QAE@XZ 0x004C66E0-0x004C67B6;0x00659EF4-0x00659F06
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
// ORIGINAL: 0x004C75B0 sub_4c75b0 0x004C75B0-0x004C766D;0x00659F20-0x00659F32 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/unrecovered/004c75b0.cpp", "src/midi.h", "src/ambience.h"], "reason": "Original Sound-to-Midi constructor staging and byte54 state identify the genuine Midi owner. Historical seed mapping includes the already shared unwind helper; its overlap remains a restoration limitation, not a newly inferred independent function extent."}
// symbol    ??0Midi@@QAE@XZ
// Historical SHARED_TAIL observation below predates the independent source identity for 0x004C8450; retain it as prior trial context, not a current span classification.
// TRIED: SHARED_TAIL - the 0x4c8450/0x659f20 unwind-handler span is COMDAT-folded with another function's handler, so no tier is reachable regardless of body. Left as a plain field-write transcription (no SEH frame modeled) for the next pass.
Midi::Midi() {
    memset(&field_54_, 0, 1);
    volatile uint8_t &mode = field_54_;
    volatile uint32_t &flags = flags_40_;
    mode |= 1;
    flags |= 8;
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
// ORIGINAL: 0x004C8460 ??0Ambience@@QAE@XZ 0x004C8460-0x004C852D;0x00659F4C-0x00659F5E
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
#pragma intrinsic(memset)

// Independent zero-initialized pointer objects used by the common chain.
// Original attach writes head at 0x004C6387 and tail at 0x004C638D;
// Sound::detach and Wave_Device::select use the same storage identities.
// RECOVERY_OBJECT: v2 0x0090DB20 0x0090DB24 ?SoundChainHead@@3PAVSound@@A 0x004C6370
Sound *SoundChainHead;
// RECOVERY_OBJECT: v2 0x0090DB1C 0x0090DB20 ?SoundChainTail@@3PAVSound@@A 0x004C6370
Sound *SoundChainTail;

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







namespace {
}  // namespace

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
// ORIGINAL: 0x004C6280 ?load@Sound@@QAEHPBD@Z 0x004C6280-0x004C6361 SEMANTIC
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



// Original family tables share slot 1 at 0x004482F0. Unlike set_fade,
// this changes the remembered interval without notifying the wrapped device.
// The behavioral name is provisional; zero refuses without changing state.
// ORIGINAL: 0x004482F0 sub_4482f0 0x004482F0-0x0044830C BYTE_EXACT
// symbol    ?set_fade_interval@SoundFadeState@@UAEHK@Z
int SoundFadeState::set_fade_interval(unsigned long interval) {
    if (interval == 0) {
        return 10;
    }
    fade_38_ = interval;
    return 0;
}

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



// Purpose: Return the next Sound in the common chain (family slot 25).
// Ownership is also witnessed by attach/detach's typed chain manipulation.
// ORIGINAL: 0x00448310 sub_448310 0x00448310-0x00448314 BYTE_EXACT
// symbol    ?get_next@Sound@@UAEPAV1@XZ
Sound *Sound::get_next() {
    return chain_next_;
}

// Purpose: Return the previous Sound in the common chain (family slot 26).
// ORIGINAL: 0x00448320 sub_448320 0x00448320-0x00448324 BYTE_EXACT
// symbol    ?get_prev@Sound@@UAEPAV1@XZ
Sound *Sound::get_prev() {
    return chain_prev_;
}

// Historical seed catalog (inactive): /*
// Historical seed catalog (inactive): Purpose: Sign-extend bit 0 of field 0x40: 0 or -1.
// Historical seed catalog (inactive):          Emitted by tools/generate_field_accessors.py from
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):              mov eax, dword ptr [ecx + 0x40] / shl eax, 0x1f / sar eax, 0x1f / ret
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):          The name is the ADDRESS, not an invention, and `self` is a void
// Historical seed catalog (inactive):          pointer because this body needs `this` to be nothing more than a
// Historical seed catalog (inactive):          pointer and an offset - the class it belongs to is not established.
// Historical seed catalog (inactive):          The adapter declares 0 stack dword(s) so it
// Historical seed catalog (inactive):          cleans 0 bytes, taken from the body's own `ret` -
// Historical seed catalog (inactive):          the only statement of this function's arity that exists, since it has
// Historical seed catalog (inactive):          no mangled name. Declaring fewer would leave them on the caller's
// Historical seed catalog (inactive):          stack.
// Historical seed catalog (inactive): // prior-original: 0x00448380 sub_448380 0x00448380-0x0044838A historical-byte-exact
// Historical seed catalog (inactive): // symbol    ?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): // size      10 bytes
// Historical seed catalog (inactive): // prototype
// Historical seed catalog (inactive): // callers   0   call targets   0
// Historical seed catalog (inactive): // kind      game
// Historical seed catalog (inactive): // flags     hidden;sp_ready;purged_ok
// Historical seed catalog (inactive): // calls     (none)
// Historical seed catalog (inactive): Return Value: the value described above
// Historical seed catalog (inactive): Status: Complete
// Historical seed catalog (inactive): */
// Historical seed catalog (inactive): address=0x00448380
// Historical seed catalog (inactive): mode=body; state=implemented
// Historical seed catalog (inactive): name=sub_448380; symbol=?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): spans=((4490112, 4490122),)
// Historical seed catalog (inactive): kind='game'; exclusion=''
// Historical seed catalog (inactive): byte_exact=True; semantic=False
// Historical seed catalog (inactive): recipe=census
// Historical seed catalog (inactive): levers=()
// Historical seed catalog (inactive): ruled_out=()
// Historical seed catalog (inactive): unrecoverable=()
// Historical seed catalog (inactive): deferred=()
// ORIGINAL: 0x00448380 sub_448380 0x00448380-0x0044838A BYTE_EXACT
// symbol    ?unk_slot24@Sound@@UAEHXZ
// The original Sound table selects 0x00448380 at slot 24. This
// inherited integer contract sign-extends flags_40_ bit zero.
int Sound::unk_slot24() {
    return static_cast<int>(flags_40_ << 31) >> 31;
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
// ORIGINAL: 0x004C8450 sub_4c8450 0x004C8450-0x004C8457 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8450..0x004C8456 stores root table 0x0066E444 through ECX and returns; the next NOP at 0x004C8457 is outside the mapped span", "pinned constructor EH continuations load saved receiver into ECX and tail-jump to 0x004C8450; the helper has its own entry and RET", "src/sound.h declares the genuine protected nonvirtual SoundFadeState destructor; SoundFadeState root table 0x0066E444 is staged by its real constructor", "historical Sound destructor mapping at 0x004C6120 included 0x004C8450..0x004C8457 and retains its BYTE_EXACT token as an obligation after this span is independently mapped"], "reason": "Bind the pinned independent ECX SoundFadeState base-stage unwind entry to the genuine protected nonvirtual SoundFadeState destructor; keep the full historical Sound destructor claim as a restoration obligation after separating this generated entry."}
// symbol    ??1SoundFadeState@@IAE@XZ

// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ??_7VideoSoundState@@6B@ 0x004c8620
// Video playback, composed resources and Sound-derived audio ownership.
// The Sound prefix owns table 0x66E790 and its own deleting thunk 0x4C9360.
// Do not add an outer virtual deleter: that thunk calls prefix 0x4C86D0 only.
#include "video.h"

// ORIGINAL: 0x004C8620 sub_4c8620 0x004C8620-0x004C86C9;0x00659F5E-0x00659F70 FILE
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

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?unload@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::unload() {
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

// ORIGINAL: 0x004C8950 sub_4c8950 0x004C8950-0x004C8953 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; VideoSoundState primary table 0x0066E790 slot 29 at 0x0066E804 relocates to 0x004C8950", "the pre-proposal src/video.cpp RECOVERY_SHARED record already bound this exact span to VideoSoundState::get_time; this replaces that one shared record with the canonical identity", "src/sound.cpp owns the actual int VideoSoundState::get_time() method body returning zero; no Time class or global timer ownership is inferred", "historical catalog facts retained: sub_4c8950 is a three-byte zero-return leaf; its name, span and claim state are carried from the parsed source record by the native writer", "retired source symbol retained as historical evidence: ?nullsub_004c8950_redirect@@YIHPAX0@Z"], "reason": "Promote the existing VideoSoundState::get_time shared-body mapping to the unique canonical identity. Keep its real typed method and avoid a duplicate annotation or adapter."}
// symbol    ?get_time@VideoSoundState@@UAEHXZ
// kind      game
int VideoSoundState::get_time() {
  return 0;
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

// ORIGINAL: 0x004C8910 sub_4c8910 0x004C8910-0x004C8923 FILE
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

// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381 FILE BYTE_EXACT
// symbol    ??_GVideoSoundState@@UAEPAXI@Z
// Compiler-owned prefix scalar destructor mapping anchor.

// RECOVERY_GENERATED: v1 vtable 0x66e82c 0x66e8c4 ??_7VoiceRx@@6B@ 0x4c8960
// RECOVERY_GENERATED: v1 vtable 0x66e8c4 0x66e95c ??_7VoiceTx@@6B@ 0x4c8cc0
// Slot2 is compiler weak _E with library-search mode2 and physical _G fallback;
// that observation does not establish final linked table selection.
#include "stdafx.h"
#include <cstring>
#include <new>
#include "voice.h"
#include "wave.h"
#include "sounddevice.h"
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(memset, strcpy)
#endif

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
