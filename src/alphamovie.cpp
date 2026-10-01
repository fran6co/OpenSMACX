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
#include "mapwin.h"  // MainInterfaceGlobal, the owner passed to GraphicWin::init
#include "console.h"  // flush_input
#include "temp.h"  // do_all_non_input
#include "text.h"
#include "wave_device.h"  // WaveDeviceGlobal
#include <mmsystem.h>
#include <digitalv.h>

// ORIGINAL: 0x00403BA0 ?amovie_project@@YAXH@Z 0x00403BA0-0x00403BDF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Native403BA3..3BD4 opens MOVLIST/PROJECTS, consumes one row plus positive project_id rows, then dispatches text_item to the real char* movie overload403BE0; initial literals682754/682748 verified in pinned image.", "src/recovered/units/00403ba0.cpp", "Root-only console-movie-wrapper-20261001 central dependency closure; immutable task baseline preserved."], "reason": "Recover complete source-owned movie selection and project-index wrapper with proper Popup lifetime, typed facilities, actual text parsing and existing movie playback. Retain exact seed identities and all original spans; no match or acceptance claim is granted by this binding."}
void __cdecl amovie_project(int project_id) {
    if (text_open("MOVLIST", "PROJECTS") != 0) return;
    text_get();
    while (project_id > 0) {
        text_get();
        --project_id;
    }
    amovie_project(text_item());
}

// The original play/stop bodies write one pointer-sized slot at 0x009B8190;
// the same slot is consumed by the video-window handlers. The dword write in
// play at 0x006001E0 independently bounds this initial storage to four bytes.
// RECOVERY_OBJECT: v2 0x009B8190 0x009B8194 ?ActiveVideoPlayer@@3PAVMCIVideo@@A 0x006001E0
MCIVideo *ActiveVideoPlayer;

// MCI video window procedure fallback, set to the application's main HWND by
// MCIVideo::load before installing the window hook.
// RECOVERY_OBJECT: v2 0x009B8194 0x009B8198 ?MciVideoCallbackWindow@@3PAUHWND__@@A 0x005FFF90
HWND MciVideoCallbackWindow;

// ORIGINAL: 0x005FFEC0 sub_5ffec0 0x005FFEC0-0x005FFF8B
// RECOVERY_BINDING: v1 {"evidence": ["The pinned 0x005FFEC0 body forwards WM_KEYDOWN and WM_LBUTTONDOWN through DefWindowProcA using the HWND stored by MCIVideo::load at 0x009B8194; its 0x3B9 case clears the active MCIVideo flag, and remaining messages call the saved WNDPROC at MCIVideo+0x464 or fall back to DefWindowProcA.", "The MCI video owner stores its active receiver at 0x009B8190; MCIVideo::play is the source-owned writer of that storage and close/stop clear it."], "reason": "Move the MCI window hook into its owning source file and route through the typed active MCIVideo fields and SDK window-procedure APIs."}
// symbol    ?MciVideoWindowProc@@YGJPAUHWND__@@IIJ@Z
LRESULT CALLBACK MciVideoWindowProc(HWND window, UINT message,
                                    WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
        DefWindowProcA(MciVideoCallbackWindow, message, wparam, lparam);
        return 0;
    case 0x3b9:
        if (ActiveVideoPlayer != 0) {
            ActiveVideoPlayer->field_0_ &= ~1u;
        }
        break;
    default:
        break;
    }

    if (ActiveVideoPlayer != 0) {
        return CallWindowProcA(ActiveVideoPlayer->previous_window_proc_,
                               window, message, wparam, lparam);
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

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
        reinterpret_cast<UINT_PTR>(window_handle_) & 0xffff);
    if (mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_PLAY,
                        MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&playParams)) != 0) {
        return;
    }

    field_0_ |= 1;
}

// ORIGINAL: 0x005FFF90 ?load@MCIVideo@@QAEHPADPAUWin@@HH@Z 0x005FFF90-0x006001D2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned-image recover inspect at 0x005FFF90; matching MCIVideo callers and field accesses at 0x005FFFC0, 0x0060008A, 0x00600161-0x006001AC; AlphaMovie::exec source member ownership."], "reason": "Recover MCI device setup, palette import, destination scaling and window procedure installation in MCIVideo using source-owned members and Win32 structures."}
// symbol    ?load@MCIVideo@@QAEHPADPAVWin@@HH@Z
int MCIVideo::load(char *moviePath, Win *targetWindow, int x, int y) {
    if (moviePath == 0 || targetWindow == 0) {
        return 0x10;
    }

    target_window_ = targetWindow;
    blit_y_ = y;
    blit_x_ = x;
    WaveDeviceGlobal.disable();

    char deviceType[] = "avivideo";
    MCI_DGV_OPEN_PARMS openParams = {0};
    openParams.lpstrDeviceType = deviceType;
    openParams.lpstrElementName = moviePath;
    openParams.dwStyle = WS_CHILD;
    openParams.hWndParent = HandleMain;
    MciVideoCallbackWindow = HandleMain;

    const DWORD openFlags = MCI_OPEN_TYPE | MCI_OPEN_ELEMENT |
                            MCI_DGV_OPEN_WS | MCI_DGV_OPEN_PARENT;
    if (mciSendCommandA(0, MCI_OPEN, openFlags,
                        reinterpret_cast<DWORD_PTR>(&openParams)) != 0) {
        return 1;
    }
    mciId_ = (mciId_ & 0xffff0000u) | (openParams.wDeviceID & 0xffffu);

    MCI_DGV_STATUS_PARMS statusParams = {0};
    statusParams.dwItem = MCI_DGV_STATUS_HWND;
    mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_STATUS,
                    MCI_STATUS_ITEM,
                    reinterpret_cast<DWORD_PTR>(&statusParams));
    window_handle_ = reinterpret_cast<HWND>(statusParams.dwReturn);

    statusParams.dwItem = MCI_DGV_STATUS_HPAL;
    mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_STATUS,
                    MCI_STATUS_ITEM,
                    reinterpret_cast<DWORD_PTR>(&statusParams));
    palette_handle_ = reinterpret_cast<HPALETTE>(statusParams.dwReturn);

    PALETTEENTRY entries[256];
    GetPaletteEntries(palette_handle_, 0, 0x100, entries);
    palette_.init();
    // The original copies RGB for entries 10..245 but skips each fourth
    // peFlags byte, retaining the flags established by Palette::init().
    for (int index = 10; index < 246; ++index) {
        palette_.entries_[index].peRed = entries[index].peRed;
        palette_.entries_[index].peGreen = entries[index].peGreen;
        palette_.entries_[index].peBlue = entries[index].peBlue;
    }
    palette_.reseed();

    MCI_DGV_WHERE_PARMS sourceParams = {0};
    mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_WHERE,
                    MCI_DGV_WHERE_SOURCE,
                    reinterpret_cast<DWORD_PTR>(&sourceParams));
    const RECT sourceRect = sourceParams.rc;
    const int width = (sourceRect.right - sourceRect.left) * 2;
    const int height = (sourceRect.bottom - sourceRect.top) * 2;

    MCI_DGV_PUT_PARMS destinationParams = {0};
    destinationParams.rc.left = sourceRect.left;
    destinationParams.rc.top = sourceRect.top;
    destinationParams.rc.right = sourceRect.right * 2 - sourceRect.left;
    destinationParams.rc.bottom = sourceRect.bottom * 2 - sourceRect.top;
    mciSendCommandA(static_cast<MCIDEVICEID>(mciId_ & 0xffff), MCI_PUT,
                    MCI_DGV_RECT | MCI_DGV_PUT_DESTINATION,
                    reinterpret_cast<DWORD_PTR>(&destinationParams));

    previous_window_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongA(
        window_handle_, GWL_WNDPROC,
        reinterpret_cast<LONG>(&MciVideoWindowProc)));
    blit_width_ = width;
    blit_height_ = height;
    MoveWindow(window_handle_, x, y, width, height, FALSE);
    return 0;
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

// Explicitly invoke the by-value Palette constructor; spelling this as a
// new-expression adds an SEH frame that the original constructor does not have.
// ORIGINAL: 0x005FFD80 ??0MCIVideo@@QAE@XZ 0x005FFD80-0x005FFDA1
// RECOVERY_BINDING: v1 {"evidence": ["git:b55fcf8bd32d532e5050a5aaadf011bcff4394b5:src/recovered/005ffd80.cpp", "source:src/alphamovie.h:MCIVideo"], "reason": "Bind the existing source-owned MCIVideo constructor to the image entry: it constructs palette_ at +0x10 and initializes the evidenced handle fields, matching the original constructor sequence."}
MCIVideo::MCIVideo() {
    palette_.Palette::Palette();
    *reinterpret_cast<short *>(&mciId_) = 0;
    window_handle_ = 0;
    palette_handle_ = 0;
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
Purpose: Initialize the movie window, try the supplied path and then the
         game's fallback path, play the loaded clip, and restore the active
         palette and window state.

The embedded MCIVideo owns the MCI load/play/stop state and its Palette. The
old transcription assembled the fallback in StringTemp but passed an empty
stack buffer to MCI. Use the evidenced k:\\game\\ prefix in the retry path.

The existing amovie_project caller now dispatches this member on a real
AlphaMovie local, so member and base lifetime follow the declared structure.
*/
// ORIGINAL: 0x00404070 ?exec@AlphaMovie@@QAEHPAD@Z 0x00404070-0x004041BF
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00404070.cpp", "source:src/alphamovie.h:AlphaMovie"], "reason": "Home AlphaMovie playback on a real GraphicWin-derived receiver with its embedded MCIVideo member; repair the seed's empty fallback load argument using its evidenced path prefix"}
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
        char fallback_path[260];
        const char prefix[] = "k:\\game\\";
        if (strlen(movie_name) > sizeof(fallback_path) - sizeof(prefix)) {
            return result;
        }
        strcpy(fallback_path, prefix);
        strcat(fallback_path, movie_name);
        result = mciVideo_.load(fallback_path, this, 0, 0);
        if (result != 0) {
            return result;
        }
    }

    const int video_height = static_cast<int>(mciVideo_.blit_height_);
    const int video_width = static_cast<int>(mciVideo_.blit_width_);
    const int y = (screen_height - video_height) / 2;
    const int x = (screen_width - video_width) / 2;
    HWND const window = mciVideo_.window_handle_;
    MoveWindow(window, x, y, video_width, video_height, FALSE);

    mciVideo_.palette_.set();
    mciVideo_.play();
    flush_input();
    set_modal(0, 0, 0);
    mciVideo_.stop();
    g_PALETTE1.set();
    ShowWindow(window, SW_HIDE);
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



// ORIGINAL: 0x005FFDB0 ?close@MCIVideo@@QAEXXZ 0x005FFDB0-0x005FFE3D
// RECOVERY_BINDING: v1 {"evidence": ["git:b55fcf8bd32d532e5050a5aaadf011bcff4394b5:src/unrecovered/005ffdb0.cpp", "source:src/alphamovie.h:MCIVideo", "original:0x005FFDB0"], "reason": "Recover the MCI device shutdown and handle reset on its owning MCIVideo member using the pinned call sequence and existing typed state."}
// Stop and close the MCI device, clear the window/player state, and restore
// wave output.
void MCIVideo::close() {
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
        mciId_ &= 0xffff0000u;
        ActiveVideoPlayer = 0;
        field_0_ &= ~1u;
    }

    mciId_ &= 0xffff0000u;
    window_handle_ = 0;
    palette_handle_ = 0;
    target_window_ = 0;
    WaveDeviceGlobal.enable();
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
