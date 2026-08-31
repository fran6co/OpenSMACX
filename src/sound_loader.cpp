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
#include "stdafx.h"
#include "original_seam.h"
#include "sound.h"
#include "basepop.h"
#include "wave.h"
#include "wave_device.h"
#include "sounddevice.h"

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

// Rehomed from src/unrecovered/004c5e50.cpp into a buildable TU (not sound.cpp,
// which owns the call site) so init_sound can call the real body instead of a
// forward into the original image. Its one unrecovered callee, the message
// helper below, is homed in the same file so the pair loads as one story.

// ---- callees, declared and never defined here ----
// sound_header_version runs from general.cpp: the sound-header version the
// game was built with, a real `mov eax,1 / ret` recovered under its own name.
extern "C" int __cdecl sound_header_version();
// sound_version_warning is homed below, under its own marker in this file.
extern "C" void __cdecl sound_version_warning(const char *text, const char *caption);

// The module handle and the version slot are sound.h's SoundDllModule() and
// SoundDllVersionSlot(). The three kernel32 entrypoints are direct API calls:
// the image calls them through its own import table, and this tree does the
// same through its own. The strings are the image's .rdata/.data text,
// recovered verbatim.

int __cdecl load_sound_dll() {
    if (SoundDllModule() == 0) {
        SoundDllModule() = LoadLibraryA(".\\soundx.dll");
        if (SoundDllModule() == 0) {
            return 1;
        }
        void **procs = SoundDllProcs();
        void **slot = procs;
        void **procs_end = procs + 11;
        int index = 0;
        do {
            void *proc = reinterpret_cast<void *>(GetProcAddress(
                SoundDllModule(), MAKEINTRESOURCEA((index + 1) & 0xffff)));
            *slot = proc;
            if (proc == 0) {
                HMODULE handle = SoundDllModule();
                WaveDeviceReleaseGuard = 0;
                if (handle != 0) {
                    FreeLibrary(handle);
                    SoundDllModule() = 0;
                }
                WaveDeviceReleaseGuard = 0;
                for (int i = 0; i < 0xb; ++i) {
                    procs[i] = 0;
                }
                return 1;
            }
            ++slot;
            ++index;
        } while ((int)slot < (int)procs_end);

        unsigned int version;
        if (SoundDllModule() == 0) {
            version = 0;
        } else {
            version = (*reinterpret_cast<func_sound_dll_version *>(
                SoundDllVersionSlot()))();
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
