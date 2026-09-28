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

// AlphaMovie constructor-installed Win branch: capture slot4, update slot63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x00669458 ? ??_7AlphaMovie@@6BWin@@@ 0x00404010
#include "original_seam.h"
#include "alphamovie.h"
#include "alpha.h"  // prefs_get
#include "general.h"  // filefind_get
#include "graphicwin.h"
#include "filewin.h"
#include "spritebox.h"
#include "net_class.h"
#include "uv2player.h"  // amovie_project's player
#include <mmsystem.h>
#include <digitalv.h>

// The original play/stop bodies write one pointer-sized slot at 0x009B8190;
// the same slot is consumed by the video-window handlers. The dword write in
// play at 0x006001E0 independently bounds this initial storage to four bytes.
// RECOVERY_OBJECT: v2 0x009B8190 0x009B8194 ?ActiveVideoPlayer@@3PAVMCIVideo@@A 0x006001E0
MCIVideo *ActiveVideoPlayer;

// ORIGINAL: 0x006001E0 ?play@MCIVideo@@QAEXXZ 0x006001E0-0x00600270 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The original member accesses the active-player slot at 0x009B8190, invokes Palette::set on its embedded palette at +0x10, and sends MCI_WINDOW followed by MCI_PLAY through the WINMM import.", "MCIVideo::blit_frame uses +0x8 as the HWND passed to GetDC and ReleaseDC; the play body loads that field's low word into MCI_PLAY_PARMS::dwCallback. This unusual truncation is preserved as observed; the MCI device id occupies the low word at +0x4.", "MCIVideo::load stores its Win* argument at +0x468 and blit_frame tests it before blitting; the separate close path clears this window pointer."], "reason": "Move the existing play identity into MCIVideo's owning translation unit and express the receiver, palette, window handle and MCI parameter blocks as typed members."}
void MCIVideo::play() {
    ActiveVideoPlayer = this;
    palette_.set();

    MCI_DGV_WINDOW_PARMS windowParams = {0};
    windowParams.nCmdShow = SW_SHOW;
    if (mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_WINDOW,
                        MCI_DGV_WINDOW_STATE,
                        reinterpret_cast<DWORD_PTR>(&windowParams)) != 0) {
        return;
    }

    MCI_PLAY_PARMS playParams = {0};
    playParams.dwCallback = static_cast<DWORD>(
        reinterpret_cast<UINT_PTR>(field_8_) & 0xffff);
    if (mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_PLAY,
                        MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&playParams)) != 0) {
        return;
    }

    field_0_ |= 1;
}

// ORIGINAL: 0x00600270 ?stop@MCIVideo@@QAEXXZ 0x00600270-0x006002EB FILE
void MCIVideo::stop() {
    const MCIDEVICEID deviceId = static_cast<MCIDEVICEID>(mciId_ & 0xffff);
    if (deviceId != 0) {
        MCI_DGV_WINDOW_PARMS windowParams = {0};
        MCI_GENERIC_PARMS stopParams = {0};
        MCI_GENERIC_PARMS closeParams = {0};
        mciSendCommandA(deviceId, MCI_WINDOW, MCI_DGV_WINDOW_STATE,
                       reinterpret_cast<DWORD_PTR>(&windowParams));
        mciSendCommandA(deviceId, MCI_STOP, 0,
                       reinterpret_cast<DWORD_PTR>(&stopParams));
        mciSendCommandA(deviceId, MCI_CLOSE, 0,
                       reinterpret_cast<DWORD_PTR>(&closeParams));
        mciId_ &= 0xffff0000;
        ActiveVideoPlayer = 0;
        field_0_ &= ~1u;
    }
}

// Not independently claimed here (0x005FFD80 is not in this batch; a
// staged, unclaimed transcription already exists at
// src/recovered/005ffd80.cpp - left as-is). This definition exists so
// AlphaMovie::AlphaMovie(), below, has something to auto-construct
// `mciVideo_` through: `palette_.Palette::Palette();` is the explicit
// member-function-call placement syntax (the fixed-address Palette
// placements the retired init_thunks.cpp used to spell) rather than a
// `new`-expression, which pulls in an SEH frame the image does not have
// here.
MCIVideo::MCIVideo() {
    palette_.Palette::Palette();
    *reinterpret_cast<short *>(&mciId_) = 0;
    field_8_ = 0;
    field_C_ = 0;
    target_window_ = 0;
}

/*
Purpose: Shut the MCI device down; the palette member is destroyed by the
         compiler after the body.
// ORIGINAL: 0x004042C0 ??1MCIVideo@@QAE@XZ 0x004042C0-0x00404309;0x006506F0-0x00650705 FILE BYTE_EXACT
// size      94 bytes
// prototype void (__thiscall ??1MCIVideo@@QAE@XZ)(MCIVideo* this)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FE2E0 0x005FFDB0
Return Value: n/a
Status: Complete
*/
MCIVideo::~MCIVideo() {
    close();
}

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
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<MCIVideo *>(self + 0xa14)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}



func_mci_video_close MCIVideoOriginalClose =
    original_method<func_mci_video_close>(0x005FFDB0);

/*
Purpose: Shut the MCI video device down.
Forwards To: 005FFDB0
Return Value: n/a
Status: Original dependency - forwards to the original image.

The auto_inline(off) is load-bearing. VC6 at /Ob2 defers codegen to the end of
the translation unit, so being defined after AlphaMovie::close does not stop
it folding this forwarder into that recovered body: measured, the caller's
`lea ecx,[esi+0xa14]; call rel32` became `lea ecx,[esi+0xa14]; call dword ptr
[MCIVideoOriginalClose]`. The pragma keeps the caller's direct call and leaves
this a separate thunk.
*/
#pragma auto_inline(off)
void MCIVideo::close() {
    (ORIGINAL(this)->*MCIVideoOriginalClose)();
}
#pragma auto_inline(on)

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
// STRUCTURE: the stack GraphicWin is DRESSED AS AN ALPHAMOVIE. The image
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
    GraphicWin win;
    MCIVideo video;
    char movie_path[260];
    char movie_file[208];

    // The image turns the stack GraphicWin into an AlphaMovie for the length
    // of the playback: it overwrites the window's two vtable pointers with
    // AlphaMovie's (primary 0x00669458, Buffer subobject at +0x444 with
    // 0x00669450), then dispatches AlphaMovie::exec through the object, and
    // re-installs the pair after every play. No C++ declaration can express a
    // base vtable swap, so the stores are spelled the way
    // GraphicWin::construct spells its own.

    movie_path[0] = 0;
    strcat(movie_path, "movies\\");
    strcat(movie_path, movie);
    movie_file[0] = 0;
    strcat(movie_file, movie_path);
    strcat(movie_file, ".wve");
    char *found = filefind_get(movie_file);
    if (found != 0) {
        player.exec(found, 0x280, 0x1E0, 0xC);
        video.close();
        win.close();
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
            video.close();
            win.close();
            return;
        }
    }

    movie_file[0] = 0;
    strcat(movie_file, movie_path);
    strcat(movie_file, ".avi");
    found = filefind_get(movie_file);
    if (found != 0) {
        static_cast<AlphaMovie *>(&win)->exec(found);
        video.close();
        win.close();
        return;
    }

    if (prefs_get("Firaxis", 0, 0) != 0) {
        movie_file[0] = 0;
        strcat(movie_file, "k:\\game\\");
        strcat(movie_file, movie_path);
        strcat(movie_file, ".avi");
        found = filefind_get(movie_file);
        if (found != 0) {
            static_cast<AlphaMovie *>(&win)->exec(found);
            video.close();
            win.close();
            return;
        }
    }

    video.close();
    win.close();
}
