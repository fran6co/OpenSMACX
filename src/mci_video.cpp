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
#include "interface_palette.h"
#include "alphamovie.h"
#include "alpha.h"  // HandleMain
#include "wave_device.h"  // WaveDeviceGlobal
#include <mmsystem.h>
#include <digitalv.h>

// RECOVERY_IMPORT: v1 __imp__mciSendCommandA@16 WINMM.dll name:mciSendCommandA

// MCI device state, callback helpers and MCIVideo methods share the current
// source owner that binds them. This records present C++ ownership, not original
// translation-unit provenance or unmeasured runtime/log behavior.

// The original play/stop bodies write one pointer-sized slot at 0x009B8190;
// the same slot is consumed by the video-window handlers. The dword write in
// play at 0x006001E0 independently bounds this initial storage to four bytes.
// RECOVERY_OBJECT: v2 0x009B8190 0x009B8194 ?ActiveVideoPlayer@@3PAVMCIVideo@@A 0x006001E0
MCIVideo *ActiveVideoPlayer;

// MCI video window procedure fallback, set to the application's main HWND by
// MCIVideo::load before installing the window hook.
// RECOVERY_OBJECT: v2 0x009B8194 0x009B8198 ?MciVideoCallbackWindow@@3PAUHWND__@@A 0x005FFF90
HWND MciVideoCallbackWindow;

// palette_ is an in-object member, so its one Palette constructor call occurs
// before this body, matching the pinned call at 0x005FFD86.
// ORIGINAL: 0x005FFD80 ??0MCIVideo@@QAE@XZ 0x005FFD80-0x005FFDA1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b55fcf8bd32d532e5050a5aaadf011bcff4394b5:src/recovered/005ffd80.cpp", "Pinned 0x005FFD86 calls the Palette constructor once on this+0x10, then 0x005FFD8D zeroes only the device-ID WORD at +4.", "source:src/alphamovie.h declares palette_ as an in-object MCIVideo member, so natural member construction supplies that single Palette lifetime before this constructor body."], "reason": "Bind the original MCIVideo constructor to its source owner using natural Palette member construction and the observed low-WORD/device-handle initialization, without a duplicate explicit constructor call."}
MCIVideo::MCIVideo() {
    mciId_ = 0;
    window_handle_ = 0;
    palette_handle_ = 0;
    target_window_ = 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005FFDB0 ?close@MCIVideo@@QAEXXZ 0x005FFDB0-0x005FFE3D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b55fcf8bd32d532e5050a5aaadf011bcff4394b5:src/unrecovered/005ffdb0.cpp", "source:src/alphamovie.h:MCIVideo", "original:0x005FFDB0"], "reason": "Recover the MCI device shutdown and handle reset on its owning MCIVideo member using the pinned call sequence and existing typed state."}
// Stop and close the MCI device, clear the window/player state, and restore
// wave output.
void MCIVideo::close() {
    if (mciId_ != 0) {
        MCI_DGV_WINDOW_PARMS windowParams;
        MCI_GENERIC_PARMS stopParams;
        MCI_GENERIC_PARMS closeParams;
        windowParams.nCmdShow = 0;
        mciSendCommandA(mciId_, MCI_WINDOW, MCI_DGV_WINDOW_STATE,
                       reinterpret_cast<DWORD_PTR>(&windowParams));
        mciSendCommandA(mciId_, MCI_STOP, 0,
                       reinterpret_cast<DWORD_PTR>(&stopParams));
        mciSendCommandA(mciId_, MCI_CLOSE, 0,
                       reinterpret_cast<DWORD_PTR>(&closeParams));
        mciId_ = 0;
        ActiveVideoPlayer = 0;
        field_0_ &= ~1u;
    }

    mciId_ = 0;
    window_handle_ = 0;
    palette_handle_ = 0;
    target_window_ = 0;
    WaveDeviceGlobal.enable();
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005FFE40 sub_5ffe40 0x005FFE40-0x005FFE78 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:mci-message-helper-owner original identity/spans and historical claims retained from src/recovered/005ffe40.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5ffe40 complete native body; bare RET and observed EAX forwarding.", "source:src/alphamovie.cpp MciVideoCallbackWindow 9B8194 set by MCIVideo::load, distinct from ActiveVideoPlayer 9B8190."], "reason": "Recover the complete MCI message helper with the existing callback HWND and actual SDK import. Unsigned low-word packing preserves native bit patterns without signed-shift undefined behavior. Names and unused first-argument interpretation remain provisional; original callers/registration are unresolved."}
// symbol    ?sub_5ffe40@@YAJPAUHWND__@@IHHH@Z
// kind      game
LRESULT __cdecl sub_5ffe40(HWND, WPARAM virtual_key, int forward_event,
                          int low_payload, int high_payload) {
    LRESULT result = forward_event;
    if (forward_event != 0) {
        const uint32_t packed =
            (static_cast<uint32_t>(static_cast<uint16_t>(high_payload)) << 16)
            | static_cast<uint32_t>(static_cast<uint16_t>(low_payload));
        result = DefWindowProcA(MciVideoCallbackWindow, WM_KEYDOWN, virtual_key,
                                static_cast<LPARAM>(packed));
    }
    return result;
}

// Superseded seed deleted; binding references retain historical provenance.
// ORIGINAL: 0x005FFE80 sub_5ffe80 0x005FFE80-0x005FFEBD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:mci-message-helper-owner original identity/spans and historical claims retained from src/recovered/units/005ffe80.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5ffe80 complete native body; bare RET and observed EAX forwarding.", "source:src/alphamovie.cpp MciVideoCallbackWindow 9B8194 set by MCIVideo::load, distinct from ActiveVideoPlayer 9B8190."], "reason": "Recover the complete MCI message helper with the existing callback HWND and actual SDK import. Unsigned low-word packing preserves native bit patterns without signed-shift undefined behavior. Names and unused first-argument interpretation remain provisional; original callers/registration are unresolved."}
// symbol    ?sub_5ffe80@@YAJPAUHWND__@@HHHI@Z
// kind      game
LRESULT __cdecl sub_5ffe80(HWND, int double_click, int x, int y,
                           WPARAM keys) {
    const uint32_t packed =
        (static_cast<uint32_t>(static_cast<uint16_t>(y)) << 16)
        | static_cast<uint32_t>(static_cast<uint16_t>(x));
    const UINT message = (double_click != 0 ? WM_LBUTTONDBLCLK - WM_LBUTTONDOWN : 0) + WM_LBUTTONDOWN;
    return DefWindowProcA(MciVideoCallbackWindow, message, keys,
                          static_cast<LPARAM>(packed));
}

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
    MCI_DGV_OPEN_PARMS openParams;
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
    mciId_ = static_cast<uint16_t>(openParams.wDeviceID);

    // Both STATUS requests reuse this block, including any untouched outputs.
    MCI_DGV_STATUS_PARMS statusParams;
    statusParams.dwItem = MCI_DGV_STATUS_HWND;
    mciSendCommandA(mciId_, MCI_STATUS,
                    MCI_STATUS_ITEM,
                    reinterpret_cast<DWORD_PTR>(&statusParams));
    window_handle_ = reinterpret_cast<HWND>(statusParams.dwReturn);

    statusParams.dwItem = MCI_DGV_STATUS_HPAL;
    mciSendCommandA(mciId_, MCI_STATUS,
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

    MCI_DGV_WHERE_PARMS sourceParams;
    mciSendCommandA(mciId_, MCI_WHERE,
                    MCI_DGV_WHERE_SOURCE,
                    reinterpret_cast<DWORD_PTR>(&sourceParams));
    const RECT sourceRect = sourceParams.rc;
    const int width = (sourceRect.right - sourceRect.left) * 2;
    const int height = (sourceRect.bottom - sourceRect.top) * 2;

    MCI_DGV_PUT_PARMS destinationParams;
    destinationParams.rc.left = sourceRect.left;
    destinationParams.rc.top = sourceRect.top;
    destinationParams.rc.right = sourceRect.right * 2 - sourceRect.left;
    destinationParams.rc.bottom = sourceRect.bottom * 2 - sourceRect.top;
    mciSendCommandA(mciId_, MCI_PUT,
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

// ORIGINAL: 0x006001E0 ?play@MCIVideo@@QAEXXZ 0x006001E0-0x00600270 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The original member accesses the active-player slot at 0x009B8190, invokes Palette::set on its embedded palette at +0x10, and sends MCI_WINDOW followed by MCI_PLAY through the WINMM import.", "MCIVideo::blit_frame uses +0x8 as the HWND passed to GetDC and ReleaseDC; the play body loads that field's low word into MCI_PLAY_PARMS::dwCallback. This unusual truncation is preserved as observed; the MCI device id occupies the low word at +0x4.", "MCIVideo::load stores its Win* argument at +0x468 and blit_frame tests it before blitting; the separate close path clears this window pointer."], "reason": "Move the existing play identity into MCIVideo's owning translation unit and express the receiver, palette, window handle and MCI parameter blocks as typed members."}
void MCIVideo::play() {
    ActiveVideoPlayer = this;
    palette_.set();

    MCI_DGV_WINDOW_PARMS windowParams;
    windowParams.nCmdShow = SW_SHOW;
    if (mciSendCommandA(mciId_, MCI_WINDOW,
                        MCI_DGV_WINDOW_STATE,
                        reinterpret_cast<DWORD_PTR>(&windowParams)) != 0) {
        return;
    }

    MCI_PLAY_PARMS playParams = {0};
    playParams.dwCallback = static_cast<DWORD>(
        reinterpret_cast<UINT_PTR>(window_handle_) & 0xffff);
    if (mciSendCommandA(mciId_, MCI_PLAY,
                        MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&playParams)) != 0) {
        return;
    }

    field_0_ |= 1;
}

// ORIGINAL: 0x00600270 ?stop@MCIVideo@@QAEXXZ 0x00600270-0x006002EB FILE BYTE_EXACT
void MCIVideo::stop() {
    if (mciId_ != 0) {
        MCI_DGV_WINDOW_PARMS windowParams;
        MCI_GENERIC_PARMS stopParams;
        MCI_GENERIC_PARMS closeParams;
        windowParams.nCmdShow = 0;
        mciSendCommandA(mciId_, MCI_WINDOW, MCI_DGV_WINDOW_STATE,
                       reinterpret_cast<DWORD_PTR>(&windowParams));
        mciSendCommandA(mciId_, MCI_STOP, 0,
                       reinterpret_cast<DWORD_PTR>(&stopParams));
        mciSendCommandA(mciId_, MCI_CLOSE, 0,
                       reinterpret_cast<DWORD_PTR>(&closeParams));
        mciId_ = 0;
        ActiveVideoPlayer = 0;
        field_0_ &= ~1u;
    }
}

// ORIGINAL: 0x006002F0 ?status@MCIVideo@@QAEHXZ 0x006002F0-0x0060031F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native 0x006002F0 reserves a 0x18-byte MCI_DGV_STATUS_PARMS local, stores 2 at local +8, loads the WORD at MCIVideo+4 for the API device id, calls MCI_STATUS with MCI_STATUS_ITEM, then returns local +4.","src/alphamovie.h declares the 16-bit device ID at +0x4; src/alphamovie.cpp includes the Windows SDK digitalv.h definition whose x86 MCI_DGV_STATUS_PARMS has dwReturn at +4 and dwItem at +8.","The transferred seed's kind='game' is catalogue-only metadata; the native source writer does not invent a BODY kind marker."],"reason":"Use the genuine SDK status structure and typed source member, preserving the observed device-ID width, item, command flags, and return value."}
int MCIVideo::status() {
    MCI_DGV_STATUS_PARMS params;
    params.dwItem = 2;
    const MCIDEVICEID device_id = mciId_;
    mciSendCommandA(device_id, MCI_STATUS,
                    MCI_STATUS_ITEM,
                    reinterpret_cast<DWORD_PTR>(&params));
    return static_cast<int>(params.dwReturn);
}

// ORIGINAL: 0x00600320 ?is_playing@MCIVideo@@QAEHXZ 0x00600320-0x00600326 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00600320-0x00600326 executes mov eax,[ecx]; and eax,1; ret", "source:src/alphamovie.h MCIVideo owns uint32_t field_0_ at +0, anchored by this identity", "source:AlphaMovie embeds MCIVideo at +0xA14; 0x004041C0 is its recovered direct source caller and is migrated to the typed member"], "reason": "Move the existing MCIVideo::is_playing identity onto the class and dword already owned by MCIVideo; keep the exact bit-0 return."}
// symbol    ?is_playing@MCIVideo@@QAEHXZ
// kind      game
int MCIVideo::is_playing() {
    return static_cast<int>(field_0_ & 1U);
}
