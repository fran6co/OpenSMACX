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
// ORIGINAL: 0x004043A0 ??_GAlphaMovie@@UAEPAXI@Z 0x004043A0-0x004043C1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004043A0-0x004043C1", "Pinned class constructor ??0AlphaMovie@@QAE@XZ at 0x00404010 stores original class/view table VA 0x669458 at 0x00404049 (mov dword ptr [esi], 0x669458); the current genuine owner vftable ??_7AlphaMovie@@6BWin@@@ selects generated ??_GAlphaMovie@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EAlphaMovie@@UAEPAXI@Z (mode 2).", "Pinned ??_GAlphaMovie@@UAEPAXI@Z at 0x004043A0 calls the source-mapped ??1AlphaMovie@@QAE@XZ at 0x00404310, then tests byte ptr [ebp + 8], 1; je 0x4043ba bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/alphamovie.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original AlphaMovie scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GAlphaMovie@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "interface_palette.h"

// AlphaMovie constructor-installed Win branch: capture slot4, update slot63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x00669458 0x006695C0 ??_7AlphaMovie@@6BWin@@@ 0x00404010
// The pinned constructor also stores 0x00669450 at 0x0040404F; its COFF relocation at +0x45 names this Buffer table.
// RECOVERY_GENERATED: v1 vtable 0x00669450 ? ??_7AlphaMovie@@6BBuffer@@@ 0x00404010
#include "alphamovie.h"
#include "alpha.h"  // prefs_get
#include "general.h"  // filefind_get
#include "graphicwin.h"
#include "filewin.h"
#include "spritebox.h"
#include "net_class.h"
#include "uv2player.h"  // amovie_project's player
#include "mapwin.h"  // MainInterfaceGlobal, the owner passed to GraphicWin::init
#include "console.h"  // flush_input
#include "win.h"
#include "text.h"
#include "wave_device.h"  // WaveDeviceGlobal
#include <mmsystem.h>
#include <digitalv.h>

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403BA0 ?amovie_project@@YAXH@Z 0x00403BA0-0x00403BDF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Native403BA3..3BD4 opens MOVLIST/PROJECTS, consumes one row plus positive project_id rows, then dispatches text_item to the real char* movie overload403BE0; initial literals682754/682748 verified in pinned image.", "src/recovered/units/00403ba0.cpp", "Root-only console-movie-wrapper-20261001 central dependency closure; immutable task baseline preserved."], "reason": "Recover complete source-owned movie selection and project-index wrapper with proper Popup lifetime, typed facilities, actual text parsing and existing movie playback. Retain exact seed identities and all original spans; no match or acceptance claim is granted by this binding."}
// RECOVERY_LITERAL: v1 0x00682748 0x00682751 ??_C@_08DKI@PROJECTS?$AA@ 0x00403BA0
// RECOVERY_LITERAL: v1 0x00682754 0x0068275C ??_C@_07FCPL@MOVLIST?$AA@ 0x00403BA0
void __cdecl amovie_project(int project_id) {
    if (text_open("MOVLIST", "PROJECTS") != 0) return;
    text_get();
    while (project_id > 0) {
        text_get();
        --project_id;
    }
    amovie_project(text_item());
}


// ORIGINAL: 0x004041C0 ?on_modal@AlphaMovie@@QAE_NXZ 0x004041C0-0x004041D2
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x004041C0-0x004041D2 calls 0x00600320 through this+0xA14, then normalizes with neg/sbb/neg", "source:src/alphamovie.h AlphaMovie has MCIVideo mciVideo_ at +0xA14", "source:src/leaf_recoveries.cpp historical metadata: 18-byte body; prototype bool (__thiscall ?on_modal@AlphaMovie@@QAE_NXZ)(AlphaMovie* this); caller count 0; one direct call target 0x00600320; flags hidden;sp_ready;purged_ok; return value 1 when playing, 0 otherwise", "pinned behavior note: the original neg/sbb/neg normalizes any nonzero callee result to one; the callee currently masks bit 0, but the normalization remains explicit"], "reason": "Move the original AlphaMovie::on_modal body from the transitional leaf into the real AlphaMovie owner; use its typed embedded MCIVideo and preserve the native nonzero-to-bool normalization."}
// symbol    ?on_modal@AlphaMovie@@UAEHXZ
// kind      game
// Primary table 0x00669458, installed by the constructor at 0x00404049,
// selects this body at +0xEC. Win::set_modal tests its full EAX result.
// The shared int virtual API retains the original normalized 0/1 result;
// the historical catalogue's bool/nonvirtual spelling is not debug-type proof.
int AlphaMovie::on_modal() {
    return mciVideo_.is_playing() != 0;
}


/*
Purpose: Shut the MCI device down; the palette member is destroyed by the
         compiler after the body.
// ORIGINAL: 0x004042C0 ??1MCIVideo@@QAE@XZ 0x004042C0-0x00404309;0x006506F0-0x00650705 FILE BYTE_EXACT
// body src/alphamovie.h
// size      94 bytes
// prototype void (__thiscall ??1MCIVideo@@QAE@XZ)(MCIVideo* this)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FE2E0 0x005FFDB0
Return Value: n/a
Status: Complete
*/


/*
Purpose: Construct the GraphicWin base and the embedded MCIVideo, then
         install AlphaMovie's own vtables.
// ORIGINAL: 0x00404010 ??0AlphaMovie@@QAE@XZ 0x00404010-0x00404067;0x006506DC-0x006506EE BYTE_EXACT
// size      87 bytes
// prototype void (__thiscall ??0AlphaMovie@@QAE@XZ)(AlphaMovie* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x005FFD80
// LEVER (2026-09-03): drop the ConstructedGraphicWin shim - AlphaMovie now
//   derives from GraphicWin directly (see alphamovie.h). 12 of 24 agreeing,
//   26 instructions against the image's 24: the shim's inlined implicit
//   constructor stored its OWN two vftables (`mov [esi], 0` /
//   `mov [esi + 0x444], 0`) between the GraphicWin base call and the MCIVideo
//   member construction, and the image has no such pair. Identical to the
//   defect stringbox.h documents for StringBox; the layout is unchanged, the
//   shim adds no data and no virtual. 24 of 24, BYTE_EXACT.
Return Value: n/a
Status: Complete
*/
AlphaMovie::AlphaMovie() {
}

/*
Purpose: Initialize the movie window, try the supplied path and then the
         game's retry path, play the loaded clip, and restore the active
         palette and window state. The retry appends its prefix and movie name
         to the existing global StringTemp, but passes a separate local buffer
         whose first byte alone was cleared to the second load.

The embedded MCIVideo owns the MCI load/play/stop state and its Palette. Keep
the native ordering and separate destinations, including the original empty
retry argument and live window-handle reload after callbacks.

The existing amovie_project caller now dispatches this member on a real
AlphaMovie local, so member and base lifetime follow the declared structure.
*/
// ORIGINAL: 0x00404070 ?exec@AlphaMovie@@QAEHPAD@Z 0x00404070-0x004041BF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00404070.cpp", "source:src/alphamovie.h:AlphaMovie", "Pinned 4040EF..404126 clears only the first stack retry byte, appends prefix6827AC and name into StringTemp9B86A0 without clearing it, then passes the separate empty stack buffer to MCIVideo::load.", "Pinned404199 reloads the live window handle after stop and global palette callbacks. The 0x100 local buffer is consistent with the observed 0x104 frame and live width word at EBP-4; this is not an independent original array-capacity claim."], "reason": "Own AlphaMovie playback on its real GraphicWin receiver and embedded MCIVideo, preserving the original separate shared-string append and empty stack retry rather than repairing the original behavior."}
int AlphaMovie::exec(char *movie_name) {
    if (movie_name == 0) {
        return 0x10;
    }

    const int screen_width = WinScreenWidth;
    const int screen_height = WinScreenHeight;
    init(0, 0, screen_width, screen_height, 0, 0x00404000,
         &MainInterfaceGlobal, 0, 0);
    fill(0);
    show(0);
    do_all_non_input();

    int result = mciVideo_.load(movie_name, this, 0, 0);
    if (result != 0) {
        char retry_name[0x100];
        retry_name[0] = 0;
        strcat(StringTemp, "k:\\game\\");
        strcat(StringTemp, movie_name);
        result = mciVideo_.load(retry_name, this, 0, 0);
        if (result != 0) {
            return result;
        }
    }

    const int video_height = static_cast<int>(mciVideo_.blit_height_);
    const int video_width = static_cast<int>(mciVideo_.blit_width_);
    const int y = (screen_height - video_height) / 2;
    const int x = (screen_width - video_width) / 2;
    MoveWindow(mciVideo_.window_handle_, x, y, video_width, video_height, FALSE);

    mciVideo_.palette_.set();
    mciVideo_.play();
    flush_input();
    set_modal(0, 0, 0);
    mciVideo_.stop();
    g_PALETTE1.set();
    ShowWindow(mciVideo_.window_handle_, SW_HIDE);
    hide();
    do_all_non_input();
    return 0;
}

/*
Purpose: Close the MCI stream and window. The MCIVideo member and GraphicWin
         base are then destroyed naturally by C++ in member/base order.
*/
// ORIGINAL: 0x00404310 ??1AlphaMovie@@QAE@XZ 0x00404310-0x00404395;0x00650710-0x0065073B
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00404310.cpp", "source:src/alphamovie.cpp:amovie_project"], "reason": "Model AlphaMovie teardown as a C++ destructor with explicit close and compiler-managed MCIVideo/member and GraphicWin/base destruction"}
// symbol    ??1AlphaMovie@@UAE@XZ
AlphaMovie::~AlphaMovie() {
    close();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00404280 ?UNK7@AlphaMovie@@QAEXXZ 0x00404280-0x00404281 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK7() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404220 ?UNK2@AlphaMovie@@QAEHHHH@Z 0x00404220-0x00404225 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@AlphaMovie@@QAEHHHH@Z)(AlphaMovie* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK2(int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404240 ?UNK4@AlphaMovie@@QAEHHHHH@Z 0x00404240-0x00404245 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@AlphaMovie@@QAEHHHHH@Z)(AlphaMovie* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK4(int, int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation returns its argument unchanged.
         Unlike the rest of this family it is not a constant return - it builds
         a frame, loads the parameter and returns it:

             push ebp / mov ebp, esp / mov eax, [ebp+8] / pop ebp / ret 4

         so the returned value is the caller's own argument, at full 32-bit
         width and with no truncation or re-extension. It touches no field,
         which is why it can be replaced ahead of the class layout like its
         neighbours.
// ORIGINAL: 0x00404230 ?UNK3@AlphaMovie@@QAEHH@Z 0x00404230-0x0040423A BYTE_EXACT
// size      10 bytes
// prototype int (__thiscall ?UNK3@AlphaMovie@@QAEHH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the argument, unchanged
Status: Complete
*/
int AlphaMovie::UNK3(int a1) {
    return a1;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404250 ?UNK5@AlphaMovie@@QAEHH@Z 0x00404250-0x00404255 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK5@AlphaMovie@@QAEHH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK5(int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00404260 ?UNK6@AlphaMovie@@QAEXHH@Z 0x00404260-0x00404263 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@AlphaMovie@@QAEXHH@Z)(AlphaMovie* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK6(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00404270 ?UNK6@AlphaMovie@@QAEXH@Z 0x00404270-0x00404273 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@AlphaMovie@@QAEXH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK6(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x004042A0 ?UNK8@AlphaMovie@@QAEHHH@Z 0x004042A0-0x004042A8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK8@AlphaMovie@@QAEHHH@Z)(AlphaMovie* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int AlphaMovie::UNK8(int, int) {
    return 1;
}


/*
// ORIGINAL: 0x004041E0 ?close@AlphaMovie@@QAEXXZ 0x004041E0-0x004041F7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?close@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005FFDB0
Status: Complete
*/
void AlphaMovie::close() {
    mciVideo_.close();
    GraphicWin::close();
}


/*
Purpose: Step the receiver back to the subobject ??_GAlphaMovie@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x00404430 ??_GAlphaMovie@@WEEE@AEPAXI@Z 0x00404430-0x0040443B BYTE_EXACT
// symbol    ??_EAlphaMovie@@WEEE@AEPAXI@Z
// CORRECTED from ??3AlphaMovie@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004043A0` into
//   ??_GAlphaMovie@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

/*
Purpose: Play the opening movie. Builds the player and a stack window dressed
         as an AlphaMovie, then hunts for the movie as a .wve under movies\,
         a .wve under the game directory, a .avi under movies\, and a .avi
         under the game directory - the last two only when the Firaxis
         preference is set - playing the first file found and tearing the
         objects down on every path out.
// ORIGINAL: 0x00403BE0 ?amovie_project@@YAXPAD@Z 0x00403BE0-0x0040400D;0x006505E0-0x006506DC FILE
// size      1321 bytes
// prototype
// callers   5   call targets   16
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00404070 0x004042C0 0x004BEA30 0x004BEA50 0x004BF400 0x0059DB40 0x005D4CF0 0x005D4DD0 0x005D4E40 0x005D7410 0x005FE2E0 0x005FFD80 0x005FFDB0 0x006005D0 0x00645470 0x00645550
// LEVER: the frame is three REAL locals - UV2Player at ebp-0x1f0c,
//   GraphicWin at ebp-0x10a4, MCIVideo at ebp-0x690 - built by their own
//   constructors, and the teardown per path is the compiler's: each return
//   destroys video, win, player in reverse order, which is why the image
//   repeats the dtor sequences per path instead of tail-merging them. The
//   tail's `call 0x4bf400` (UV2Player::close) then `call 0x5d7410`
//   (~Buffer on player.buffer_) is ~UV2Player itself, already BYTE_EXACT at
//   0x004043D0 - the image inlines it here.
// STRUCTURE (historical source model before the typed migration): the stack
// GraphicWin is DRESSED AS AN ALPHAMOVIE. The image
//   overwrites the window's two vtable pointers (primary 0x00669458, Buffer
//   subobject at +0x444 with 0x00669450 - AlphaMovie's, NOT GraphicWin's
//   0x66fc50/0x66fc48) and dispatches AlphaMovie::exec on it; the mciVideo_
//   slot the exec reaches at +0xa14 coincides exactly with the MCIVideo
//   local at ebp-0x690. The wve paths tear the MCIVideo down through
//   ??1MCIVideo (0x004042c0); the avi paths spell the same thing out
//   (close, close, ~Palette, ~GraphicWin) because VC6 inlined the dtor
//   there and called it on the wve paths.
// TRIED: this body - real locals, per-path returns, explicit vtable
//   reinstall after each play. Measured 8 of 247, compiled 159 instructions
//   against the image's 247. The image's prologue pushes a REAL SEH
//   scopetable (0x006506d2, the flags' `frame`; the cold span
//   0x006505e0-0x006506dc is the handler/funclet region) and keeps an EBP
//   frame; this body compiles under the best-scoring flag set to an
//   ESP frame whose EH record pushes a null scopetable, so every
//   ebp-relative encoding diverges and the unwind funclets are missing
//   entirely. Reproducing it needs the playback guarded by structured
//   exception handling, which the spelled body does not have.
Return Value: n/a
Status: Complete
*/
void __cdecl amovie_project(char *movie) {
    UV2Player player;
    AlphaMovie win;
    char movie_path[260];
    char movie_file[208];

    // Keep the window and its embedded MCI player in one AlphaMovie object.
    // The original body dressed a GraphicWin and an adjacent MCIVideo local
    // with AlphaMovie's vtables; the typed object gives exec a real receiver
    // and the correct member lifetime.

    movie_path[0] = 0;
    strcat(movie_path, "movies\\");
    strcat(movie_path, movie);
    movie_file[0] = 0;
    strcat(movie_file, movie_path);
    strcat(movie_file, ".wve");
    char *found = filefind_get(movie_file);
    if (found != 0) {
        player.exec(found, 0x280, 0x1E0, 0xC);
        return;
    }

    if (prefs_get("Firaxis", 0, 0) != 0) {
        movie_file[0] = 0;
        strcat(movie_file, "k:\\game\\");
        strcat(movie_file, movie_path);
        strcat(movie_file, ".wve");
        found = filefind_get(movie_file);
        if (found != 0) {
            player.exec(found, 0x280, 0x1E0, 0xC);
            return;
        }
    }

    movie_file[0] = 0;
    strcat(movie_file, movie_path);
    strcat(movie_file, ".avi");
    found = filefind_get(movie_file);
    if (found != 0) {
        win.exec(found);
        return;
    }

    if (prefs_get("Firaxis", 0, 0) != 0) {
        movie_file[0] = 0;
        strcat(movie_file, "k:\\game\\");
        strcat(movie_file, movie_path);
        strcat(movie_file, ".avi");
        found = filefind_get(movie_file);
        if (found != 0) {
            win.exec(found);
            return;
        }
    }

}

// ORIGINAL: 0x00404200 ?UNK1@AlphaMovie@@QAEXXZ 0x00404200-0x00404208 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native 0x00404200 body loads the primary Win vptr and tail-dispatches slot 58 with the unchanged receiver.","src/alphamovie.h declares AlphaMovie : public GraphicWin; src/graphicwin.h and src/win.h establish the unadjusted GraphicWin/Win receiver and typed virtual release_modal slot 58.","The transferred seed's kind='game' is catalogue-only metadata; the native source writer does not invent a BODY kind marker."],"reason":"Home the original virtual dispatch in AlphaMovie as a typed inherited call, preserving dynamic dispatch and the original receiver hierarchy."}
void AlphaMovie::UNK1() {
    release_modal();
}


/*
Purpose: ?on_key_click@AlphaMovie@@QAEHHH@Z - dispatch to slot 0xe8 of this object's OWN
         vtable with NO arguments: the original pushes nothing between the
         vtable load and the call, so the parameters this function declares are
         discarded and only the `ret` cleans them up.
// ORIGINAL: 0x00404210 ?on_key_click@AlphaMovie@@QAEHHH@Z 0x00404210-0x00404220 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x00669458, pointer word at +0x64, identifies original entry 0x00404210", "source:src/win.h inherited virtual interface; actual emitted method ?on_key_click@AlphaMovie@@UAEHHH@Z in primary Win table ??_7AlphaMovie@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 25. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_key_click@AlphaMovie@@UAEHHH@Z
// size      16 bytes
// prototype int (__thiscall ?on_key_click@AlphaMovie@@QAEHHH@Z)(AlphaMovie* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00404212
Return Value: 0x1, a constant
Status: Complete
*/
int AlphaMovie::on_key_click(int, int) {
  release_modal();
  return 1;
}
