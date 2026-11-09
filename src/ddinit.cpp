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
#include "net_class.h"
#include "multidebug.h"
typedef signed char int8;  // a scaffold spelling the artifacts use
#include "scroll.h"
#include "win.h"
#include "ddinit.h"
#include "game.h"
#include "imagebutton.h"
#include <cstring>
#include "palette.h"
#include "general.h"
#include "jackal.h" // JackalInitFlags lifecycle state
#include "dialog.h"
#include "sounddevice.h"
#include "sound.h"
#include "menu.h"
#include "popup.h"   // pop_caption_title, for DDInit::report_error
#include "basepop.h"  // Win::set_bubble_text's popup interfaces
#include <ddraw.h>  // IDirectDrawSurface::GetDC / ReleaseDC in the hdc pair

// Native6354D0 leaves pitch untouched, including on nonstatic objects.
// The global instance is separately zero-initialized by C++ static storage.
// ORIGINAL: 0x006354D0 sub_6354d0 0x006354D0-0x006354E9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x6354d0", "historical source:src/field_accessors.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original6354D0 initializes seven receiver words and returns this; true constructor preserves untouched pitch at14. Unknown18/1C roles and complete size remain unproved."}
// symbol    ??0DDInit@@QAE@XZ
DDInit::DDInit() {
    hwnd_ = 0;
    surf_ = 0;
    field_8_ = 0;
    locked_surface_ = 0;
    locked_bits_ = 0;
    unknown_18_ = 0;
    unknown_1C_ = 0;
}

DDInit::~DDInit() {
    // Original635450 uses the same cleanup order as635750, including its
    // omission of secondary-surface Release. No manual atexit registration:
    // the compiler owns global startup and exactly one automatic teardown.
    teardown();
}

// ORIGINAL: 0x006354F0 sub_6354f0 0x006354F0-0x00635503 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x6354f0", "historical source:src/recovered/006354f0.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original6354F0 leaves ECX intact, passes0/0/depth/1 to mapped DDInit::init and returns ret4. Receiver-preserving member replaces contradictory free-stdcall seed. Name is descriptive."}
// symbol    ?init_default_mode@DDInit@@QAEHH@Z
int DDInit::init_default_mode(int depth) {
    return init(0, 0, depth, 1);
}

// ORIGINAL: 0x00635410 sub_635410 0x00635410-0x00635441 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x635410", "historical source:src/recovered/units/00635410.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original635410 clears seven words of global9BE618 and registers635450. Current compiler-owned global initializer is independently observed _$E21 with startup and registration helpers _$E18/_$E20. Preserve historical single-body claim; helper splitting remains restoration work, not a pass."}
// symbol    _$E16
// ORIGINAL: 0x00635450 sub_635450 0x00635450-0x006354CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:ddinit-global-lifetime", "Pinned native original0x635450", "historical source:src/unrecovered/00635450.cpp", "Fresh COFF symbols and helper references in both configured flag sets."], "reason": "Original635450 cleanup belongs to global DDInit lifetime. Current compiler-owned exit _$E19 dispatches genuine destructor and teardown; original direct inline cleanup not presently reproduced. Seed stdcall-virtual-call insight is retained; named SDK COM methods supply that ABI. No new claim."}
// symbol    _$E14
// TRIED: falling off the end without a return (C4716, error under the scoring flags); "return 0" adds an extra "xor eax,eax" the original's bare "pop esi; ret" tail does not have (#36). Modelled the 3 vtable dispatches as raw __stdcall function pointers (COM-style, this pushed as an explicit arg) rather than the VCall thiscall shim, since the disassembly pushes "this" onto the stack instead of loading it into ecx.
// RECOVERY_OBJECT: v1 0x009BE618 ?WinDisplayInit@@3VDDInit@@A
DDInit WinDisplayInit;     // 0x009BE618


// The class name `UnregisterClassA` is handed at teardown - a THIRD copy of
// a window-class-name literal, distinct from `WinClassName` below. Read out
// of the image at 0x00697F3C rather than assumed.
// RECOVERY_OBJECT: v1 0x00697F3C _DDInitClassName
static const char DDInitClassName[] = "DirectDClass";

/*
Purpose: Tear down whatever DirectDraw surface and device window this
         object is already holding, then (unless `depth` is zero) bring up
         a new one at the requested size, and paint the splash logo onto
         `Win::screen_buffer_` once the new mode is live.
// ORIGINAL: 0x00635510 ?init@DDInit@@QAEHHHHH@Z 0x00635510-0x00635746;0x00663870-0x00663885
// size      587 bytes
// prototype int (__thiscall ?init@DDInit@@QAEHHHHH@Z)(DDInit *this, int width, int height, int depth, int tgl)
// callers   2   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D44FC 0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DFB50 0x005DFF00 0x005EFD00 0x005EFD20 0x00635870
//
// TWO SPANS, and the second is a 21-byte EH funclet the linker lays away
// from the body - the same shape `Buffer::load_pcx` and `Win::init_class`
// already reproduce byte-exact from nothing more than a local `Buffer`
// with a non-trivial destructor. `buf` here is that local: VC6 emits the
// SEH frame (`push -1; push <handler>; mov fs:[0]...`) and the three
// `buf.~Buffer()` calls at every return path (7, 0x12 and the two shared
// at the bottom) purely because `buf` is in scope, not because anything
// here spells them out.
//
// `surf_` is `IDirectDraw *`, not the `IDirectDrawSurface *` the field
// name in early transcriptions guessed: `DirectDrawCreate`'s second
// argument is `LPDIRECTDRAW *`, and the two vtable slots it dispatches
// through afterwards - offset 0x4C (index 19) and 0x50 (index 20) in the
// real `IDirectDraw` vtable - are `RestoreDisplayMode()` and
// `SetCooperativeLevel(HWND, DWORD)`, not anything `IDirectDrawSurface`
// declares at those slots.
//
// `locked_surface_`/`locked_bits_` (offsets 0xC/0x10) are proven the same
// way: slot 0x80 (index 32) is `IDirectDrawSurface::Unlock(LPVOID)`, one
// pointer argument, which is exactly what the image pushes beside the
// receiver. Nothing in this function ever sets either field, so whatever
// locks that surface is a sibling method this recovery does not need.
//
// `buf.copy(&ScreenBuffer, ...)` at the end is `Win::init_class`'s own
// splash-centring call with `buf` standing in for `logo` - same six-argument
// overload, same halves-rounded-toward-zero division on both axes.
//
// Two callees stay pending_bodies forwarders: 0x005EFD00 (already
// BYTE_EXACT in src/recovered/005efd00.cpp, unpromoted) and 0x00635870
// (a 1691-byte DDERR_* switch, unrecovered). Neither is this recovery's
// job; see the notes beside their forwarders in pending_bodies.cpp.
//
// TRIED: 96 of 178 positionally under `/c /O2 /Ob0 /Gy /GR- /GX` (the
//            flag set `measure` picks), 0.986 similar - every call, branch
//            and field access agrees; what does not is which callee-saved
//            register holds `this` and which holds the constant zero. The
//            image keeps `this` in edi and the zero it stores into cleared
//            fields and compares pointers against in ebp; this body's
//            allocator gives `this` to ebp and the zero to ebx, which is the
//            same register-allocation-order puzzle `Win::init_class`
//            documents above at length and never fully resolves. No field
//            order, `nullptr`-vs-`0` spelling or flag set tried here moves
//            it.
// TRIED: declaring `Buffer buf` AFTER the two early guards, which is where
//            the image constructs it - its HandleMain test at 0x0063554B
//            precedes any call, and the returns at 0 and 7 carry no unwind
//            state, where declaring it first makes VC6 track two extra EH
//            scopes (`[esp+0x5a0]` taking 1 and 2 against the image's 0 and
//            -1 only). Under listing_diff's single flag set it cuts the
//            count from 192 to 182 - and it destroys the alignment, 96
//            agreeing down to 8. Count is not progress; the declaration
//            stays where it is.
// NOTE:      `listing_diff` and `measure` disagree about the instruction
//            count here, and MEASURE is the one to believe: it searches the
//            flag sets and settles on 177, while listing_diff compiles one
//            set and reports 192. Do not read a regression into that gap -
//            the body is byte-identical to the one the line below measured
//            (checked against 2bf26a91).
// TRIED: the one instruction of the 178 this tree does not emit is in
//            the splash `copy` call: the image loads
//            `ScreenBuffer.dib_.bmiHeader.biWidth` into ecx before pushing
//            the other arguments and then computes `mov eax, ecx; sub eax,
//            esi`, where this tree subtracts straight from memory. Hoisting
//            it - and the other three header fields - into named `const int`
//            locals does NOT reproduce that: VC6 folds them all back, and
//            all three spellings measure an identical 96 of 178 with 177
//            compiled instructions.
Status: Complete
*/
int DDInit::init(int width, int height, int depth, int tgl) {
    Buffer buf;

    if (depth == 0) {
        return 0;
    }
    if (HandleMain == nullptr) {
        return 7;
    }

    if (locked_surface_ != nullptr && locked_bits_ != nullptr) {
        locked_surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
    }
    if (surf_ != nullptr) {
        surf_->RestoreDisplayMode();
        surf_->Release();
        surf_ = nullptr;
    }
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        UnregisterClassA(DDInitClassName, WinInstance);
    }
    locked_surface_ = nullptr;
    DDInitRefreshScreenMetrics();

    if (width == 0 || height == 0) {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    HRESULT hr = (surf_ == nullptr)
        ? DirectDrawCreate(nullptr, &surf_, nullptr)
        : DD_OK;
    if (hr == DD_OK) {
        hr = surf_->SetCooperativeLevel(HandleMain, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
    }
    if (hr == DD_OK) {
        hr = surf_->SetDisplayMode(width, height, depth);
    }
    if (hr == DD_OK) {
        hr = surf_->SetCooperativeLevel(nullptr, DDSCL_NORMAL);
    }
    if (hr != DD_OK) {
        report_error(hr);
        return 0x12;
    }

    WinScreenWidth = width;
    WinScreenHeight = height;

    if (tgl != 0) {
        Win::screen_buffer_.init(width, height, 0, 0);
        Win::screen_buffer_.fill(0);
        if (buf.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
            buf.copy(&Win::screen_buffer_, 0, 0,
                     (Win::screen_buffer_.dib_.bmiHeader.biWidth - buf.dib_.bmiHeader.biWidth) / 2,
                     (buf.dib_.bmiHeader.biHeight - Win::screen_buffer_.dib_.bmiHeader.biHeight) / 2,
                     buf.dib_.bmiHeader.biWidth,
                     -buf.dib_.bmiHeader.biHeight);
        }
    }

    Win::flip(nullptr);
    return 0;
}



/*
Purpose: Tear down whatever DirectDraw surface and device window this object
         is holding - the same stages as the mode switch inside `init` above,
         as their own entry point, and what jackal_close runs on the
         0x009BE618 object.
// ORIGINAL: 0x00635750 sub_635750 0x00635750-0x006357C1 BYTE_EXACT
// symbol    ?teardown@DDInit@@QAEXXZ
// size      113 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD00
// indirect  0x00635765 0x0063577C 0x00635785 0x00635796 0x006357AD
//
// PROMOTED out of src/unrecovered/00635750.cpp, which measured it byte-exact
// against opaque scaffolding: the object there called `TeardownObj` IS
// `WinDisplayInit` - its five fields are DDInit's own, in this order, and it
// is destroyed through `ecx = 0x9BE618`, the address win.h pins the global
// to. The interface calls go by name, as in `init` above: Unlock is vtable
// offset 0x80, RestoreDisplayMode 0x4c, Release 8 - and 0x005EFD00 is the
// homed DDInitRefreshScreenMetrics.
*/
void DDInit::teardown() {
    if (locked_surface_ != nullptr && locked_bits_ != nullptr) {
        locked_surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
    }
    if (surf_ != nullptr) {
        surf_->RestoreDisplayMode();
        surf_->Release();
        surf_ = nullptr;
    }
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        UnregisterClassA(DDInitClassName, WinInstance);
    }
    locked_surface_ = nullptr;
    DDInitRefreshScreenMetrics();
}

// The 101 literals report_error pushes (99 DDERR_ names, DDRAWERROR and jackal),
// each read NUL-terminated from the image at the pushed operand.
// RECOVERY_LITERAL: v1 0x00697F4C 0x00697F65 ??_C@_0BJ@CBME@DDERR_ALREADYINITIALIZED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697F68 0x00697F82 ??_C@_0BK@DPHE@DDERR_CANNOTATTACHSURFACE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697F84 0x00697F9E ??_C@_0BK@JIOM@DDERR_CANNOTDETACHSURFACE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697FA0 0x00697FB8 ??_C@_0BI@PBAE@DDERR_CURRENTLYNOTAVAIL?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697FB8 0x00697FC8 ??_C@_0BA@EAOD@DDERR_EXCEPTION?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697FC8 0x00697FD6 ??_C@_0O@JJAG@DDERR_GENERIC?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697FD8 0x00697FEA ??_C@_0BC@EIBC@DDERR_HEIGHTALIGN?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00697FEC 0x00698006 ??_C@_0BK@BLOA@DDERR_INCOMPATIBLEPRIMARY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698008 0x0069801A ??_C@_0BC@JNLE@DDERR_INVALIDCAPS?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069801C 0x00698032 ??_C@_0BG@MOJK@DDERR_INVALIDCLIPLIST?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698034 0x00698046 ??_C@_0BC@HCAL@DDERR_INVALIDMODE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698048 0x0069805C ??_C@_0BE@LOLC@DDERR_INVALIDOBJECT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069805C 0x00698070 ??_C@_0BE@OBBL@DDERR_INVALIDPARAMS?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698070 0x00698089 ??_C@_0BJ@EJCO@DDERR_INVALIDPIXELFORMAT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069808C 0x0069809E ??_C@_0BC@KBCO@DDERR_INVALIDRECT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980A0 0x006980B5 ??_C@_0BF@IFLE@DDERR_LOCKEDSURFACES?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980B8 0x006980C3 ??_C@_0L@NEBN@DDERR_NO3D?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980C4 0x006980D4 ??_C@_0BA@EHJG@DDERR_NOALPHAHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980D4 0x006980E5 ??_C@_0BB@EJBM@DDERR_NOCLIPLIST?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980E8 0x006980FC ??_C@_0BE@IEFK@DDERR_NOCOLORCONVHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006980FC 0x00698118 ??_C@_0BM@INCD@DDERR_NOCOOPERATIVELEVELSET?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698118 0x00698129 ??_C@_0BB@NHFC@DDERR_NOCOLORKEY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069812C 0x0069813F ??_C@_0BD@PBLA@DDERR_NOCOLORKEYHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698140 0x0069815A ??_C@_0BK@PEEB@DDERR_NODIRECTDRAWSUPPORT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069815C 0x00698172 ??_C@_0BG@GHJG@DDERR_NOEXCLUSIVEMODE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698174 0x00698183 ??_C@_0P@MLDL@DDERR_NOFLIPHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698184 0x00698190 ??_C@_0M@DMHE@DDERR_NOGDI?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698190 0x006981A1 ??_C@_0BB@BGEM@DDERR_NOMIRRORHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006981A4 0x006981B3 ??_C@_0P@BPIP@DDERR_NOTFOUND?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006981B4 0x006981C6 ??_C@_0BC@JFFJ@DDERR_NOOVERLAYHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006981C8 0x006981DB ??_C@_0BD@HLBF@DDERR_NORASTEROPHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006981DC 0x006981EF ??_C@_0BD@BAKO@DDERR_NOROTATIONHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006981F0 0x00698202 ??_C@_0BC@JCJA@DDERR_NOSTRETCHHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698204 0x00698217 ??_C@_0BD@FJEG@DDERR_NOT4BITCOLOR?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698218 0x00698230 ??_C@_0BI@NPCF@DDERR_NOT4BITCOLORINDEX?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698230 0x00698243 ??_C@_0BD@ECCH@DDERR_NOT8BITCOLOR?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698244 0x00698256 ??_C@_0BC@KLCE@DDERR_NOTEXTUREHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698258 0x00698268 ??_C@_0BA@KCIJ@DDERR_NOVSYNCHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698268 0x0069827A ??_C@_0BC@IPJO@DDERR_NOZBUFFERHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069827C 0x0069828F ??_C@_0BD@HDLA@DDERR_NOZOVERLAYHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698290 0x006982A0 ??_C@_0BA@OHCK@DDERR_OUTOFCAPS?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006982A0 0x006982B2 ??_C@_0BC@PPLJ@DDERR_OUTOFMEMORY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006982B4 0x006982CB ??_C@_0BH@GLCM@DDERR_OUTOFVIDEOMEMORY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006982CC 0x006982E2 ??_C@_0BG@FCIN@DDERR_OVERLAYCANTCLIP?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006982E4 0x00698307 ??_C@_0CD@IKFB@DDERR_OVERLAYCOLORKEYONLYONEACTI@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698308 0x0069831A ??_C@_0BC@LHPH@DDERR_PALETTEBUSY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069831C 0x00698331 ??_C@_0BF@EBGI@DDERR_COLORKEYNOTSET?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698334 0x00698351 ??_C@_0BN@HBEC@DDERR_SURFACEALREADYATTACHED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698354 0x00698372 ??_C@_0BO@PJKH@DDERR_SURFACEALREADYDEPENDENT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698374 0x00698386 ??_C@_0BC@OHLB@DDERR_SURFACEBUSY?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698388 0x0069839E ??_C@_0BG@KGKE@DDERR_CANTLOCKSURFACE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006983A0 0x006983B8 ??_C@_0BI@KAAK@DDERR_SURFACEISOBSCURED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006983B8 0x006983CA ??_C@_0BC@FDEM@DDERR_SURFACELOST?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006983CC 0x006983E5 ??_C@_0BJ@OMKO@DDERR_SURFACENOTATTACHED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006983E8 0x006983FB ??_C@_0BD@BPON@DDERR_TOOBIGHEIGHT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006983FC 0x0069840D ??_C@_0BB@GBAJ@DDERR_TOOBIGSIZE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698410 0x00698422 ??_C@_0BC@GCH@DDERR_TOOBIGWIDTH?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698424 0x00698436 ??_C@_0BC@JHBM@DDERR_UNSUPPORTED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698438 0x00698450 ??_C@_0BI@BECN@DDERR_UNSUPPORTEDFORMAT?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698450 0x00698466 ??_C@_0BG@LKKP@DDERR_UNSUPPORTEDMASK?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698468 0x00698486 ??_C@_0BO@MNN@DDERR_VERTICALBLANKINPROGRESS?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698488 0x0069849E ??_C@_0BG@BBPF@DDERR_WASSTILLDRAWING?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006984A0 0x006984AD ??_C@_0N@BDF@DDERR_XALIGN?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006984B0 0x006984CC ??_C@_0BM@OPAM@DDERR_INVALIDDIRECTDRAWGUID?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006984CC 0x006984EB ??_C@_0BP@HCLN@DDERR_DIRECTDRAWALREADYCREATED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006984EC 0x00698501 ??_C@_0BF@LDPJ@DDERR_NODIRECTDRAWHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698504 0x00698526 ??_C@_0CC@GOKD@DDERR_PRIMARYSURFACEALREADYEXIST@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698528 0x0069853A ??_C@_0BC@KIDO@DDERR_NOEMULATION?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069853C 0x00698551 ??_C@_0BF@MBHN@DDERR_REGIONTOOSMALL?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698554 0x0069856D ??_C@_0BJ@MFMC@DDERR_CLIPPERISUSINGHWND?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698570 0x00698588 ??_C@_0BI@NMMO@DDERR_NOCLIPPERATTACHED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698588 0x00698595 ??_C@_0N@INIK@DDERR_NOHWND?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698598 0x006985AD ??_C@_0BF@DLEF@DDERR_HWNDSUBCLASSED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006985B0 0x006985C5 ??_C@_0BF@MOCP@DDERR_HWNDALREADYSET?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006985C8 0x006985E0 ??_C@_0BI@DACF@DDERR_NOPALETTEATTACHED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006985E0 0x006985F2 ??_C@_0BC@GBNJ@DDERR_NOPALETTEHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006985F4 0x0069860A ??_C@_0BG@MIGH@DDERR_BLTFASTCANTCLIP?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069860C 0x0069861A ??_C@_0O@DCPD@DDERR_NOBLTHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069861C 0x0069862D ??_C@_0BB@LKO@DDERR_NODDROPSHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698630 0x00698648 ??_C@_0BI@MEJA@DDERR_OVERLAYNOTVISIBLE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698648 0x0069865C ??_C@_0BE@DDHB@DDERR_NOOVERLAYDEST?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069865C 0x00698672 ??_C@_0BG@MKKH@DDERR_INVALIDPOSITION?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698674 0x0069868D ??_C@_0BJ@OGJB@DDERR_NOTAOVERLAYSURFACE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698690 0x006986AE ??_C@_0BO@IOJH@DDERR_EXCLUSIVEMODEALREADYSET?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006986B0 0x006986C3 ??_C@_0BD@CJKH@DDERR_NOTFLIPPABLE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006986C4 0x006986D8 ??_C@_0BE@PLOG@DDERR_CANTDUPLICATE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006986D8 0x006986E8 ??_C@_0BA@IHOJ@DDERR_NOTLOCKED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006986E8 0x006986FB ??_C@_0BD@PHJH@DDERR_CANTCREATEDC?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006986FC 0x00698707 ??_C@_0L@PFPD@DDERR_NODC?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698708 0x00698718 ??_C@_0BA@BNFD@DDERR_WRONGMODE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698718 0x00698730 ??_C@_0BI@CFBP@DDERR_IMPLICITLYCREATED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698730 0x00698744 ??_C@_0BE@GJCG@DDERR_NOTPALETTIZED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698744 0x0069875A ??_C@_0BG@GGFL@DDERR_UNSUPPORTEDMODE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069875C 0x0069876D ??_C@_0BB@PNI@DDERR_NOMIPMAPHW?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x00698770 0x00698789 ??_C@_0BJ@CODF@DDERR_INVALIDSURFACETYPE?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x0069878C 0x0069879F ??_C@_0BD@OCPI@DDERR_CANTPAGELOCK?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006987A0 0x006987B5 ??_C@_0BF@LLCF@DDERR_CANTPAGEUNLOCK?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006987B8 0x006987CC ??_C@_0BE@PHHK@DDERR_NOTPAGELOCKED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006987CC 0x006987E1 ??_C@_0BF@LHGP@DDERR_NOTINITIALIZED?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006987E4 0x006987EF ??_C@_0L@FJAE@DDRAWERROR?$AA@ 0x00635870
// RECOVERY_LITERAL: v1 0x006987F0 0x006987F7 ??_C@_06KHMO@jackal?$AA@ 0x00635870


// RECOVERY_ASSOCIATED: v1 switch_targets 0x00635870 0x00635F0C 0x00635F24 0x00635964
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00635870 0x00635F24 0x00635F70 0x0063595E
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00635870 0x00635F70 0x00635FA0 0x006359D5
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00635870 0x00635FA0 0x00636007 0x006359CF
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00635870 0x00636008 0x0063606C 0x00635A9C
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00635870 0x0063606C 0x00636121 0x00635A96
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00635870 0x00636124 0x00636188 0x00635C2A
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00635870 0x00636188 0x0063621F 0x00635C24
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00635870 0x00636220 0x00636284 0x00635DA5
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00635870 0x00636284 0x006362F1 0x00635D9F
/*
Purpose: Name the DirectDraw error and put it in front of the user.
// ORIGINAL: 0x00635870 sub_635870 0x00635870-0x00635F0B BYTE_EXACT
// symbol    ?report_error@DDInit@@QAEHH@Z
// size      1691 bytes
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625EC0 0x00627260 0x00628F30
//
// A MEMBER, despite the catalogue calling it `sub_635870` and the old
// transcription spelling it `__stdcall sub_635870(int)`. The caller settles
// it: `DDInit::init` does `push eax; mov ecx, edi; call 0x635870` at
// 0x0063564D, and edi is `this`. The body never reads ecx, which is why the
// free-function spelling appeared to work.
//
// EVERY CASE IS CHECKED, not transcribed on trust. Each of the 99 codes was
// read out of the image's own jump chain, and the string it selects was read
// from the image at the address the case pushes - and every one of those
// strings is the name of the DDERR_ macro whose value equals that code, all
// 99 agreeing with the SDK's DDRAW.H. So the case labels below are the
// macros rather than the raw HRESULTs, and the strings are what shipped.
//
// A flat switch over all 99 cases reaches instruction #22 before diverging
// (`push` against `mov`): MSVC re-groups this case set into different
// sub-tables than the original's five, and which case maps to which string
// is not in dispute - it is verified above.
//
// LEVER (2026-08-29): THE CASES CALL parse_says DIRECTLY, and VC6
// tail-merges all 99 into the image's shared
// `push 0; call parse_says; add esp,0x10` at 0x00635ed6 -
// `case X: parse_says(0, "DDERR_X", -1, -1); break;` is BYTE_EXACT 469/469.
// The previous shape (`message = "..."` per case, one
// `if (message) parse_says(0, message, -1, -1);` after the switch) is the
// 744-instruction lowering the flat-switch TRIED line describes: with the
// call outside the switch, VC6 never sinks it into the cases, and each case
// shrinks to `mov eax, OFFSET str; jmp`. The call belongs IN the case body.
Return Value: 0 if the popup was dismissed, otherwise whatever the media
              check returns.
Status: Complete
*/
int DDInit::report_error(int hr) {
    switch (hr) {
    case DDERR_SURFACEALREADYATTACHED:
        parse_says(0, "DDERR_SURFACEALREADYATTACHED", -1, -1);
        break;
    case DDERR_NOCOLORKEY:
        parse_says(0, "DDERR_NOCOLORKEY", -1, -1);
        break;
    case DDERR_INVALIDCAPS:
        parse_says(0, "DDERR_INVALIDCAPS", -1, -1);
        break;
    case DDERR_CANNOTATTACHSURFACE:
        parse_says(0, "DDERR_CANNOTATTACHSURFACE", -1, -1);
        break;
    case DDERR_OUTOFMEMORY:
        parse_says(0, "DDERR_OUTOFMEMORY", -1, -1);
        break;
    case DDERR_UNSUPPORTED:
        parse_says(0, "DDERR_UNSUPPORTED", -1, -1);
        break;
    case DDERR_GENERIC:
        parse_says(0, "DDERR_GENERIC", -1, -1);
        break;
    case DDERR_NOTINITIALIZED:
        parse_says(0, "DDERR_NOTINITIALIZED", -1, -1);
        break;
    case DDERR_INVALIDPARAMS:
        parse_says(0, "DDERR_INVALIDPARAMS", -1, -1);
        break;
    case DDERR_ALREADYINITIALIZED:
        parse_says(0, "DDERR_ALREADYINITIALIZED", -1, -1);
        break;
    case DDERR_CANNOTDETACHSURFACE:
        parse_says(0, "DDERR_CANNOTDETACHSURFACE", -1, -1);
        break;
    case DDERR_CURRENTLYNOTAVAIL:
        parse_says(0, "DDERR_CURRENTLYNOTAVAIL", -1, -1);
        break;
    case DDERR_EXCEPTION:
        parse_says(0, "DDERR_EXCEPTION", -1, -1);
        break;
    case DDERR_HEIGHTALIGN:
        parse_says(0, "DDERR_HEIGHTALIGN", -1, -1);
        break;
    case DDERR_INCOMPATIBLEPRIMARY:
        parse_says(0, "DDERR_INCOMPATIBLEPRIMARY", -1, -1);
        break;
    case DDERR_INVALIDCLIPLIST:
        parse_says(0, "DDERR_INVALIDCLIPLIST", -1, -1);
        break;
    case DDERR_INVALIDMODE:
        parse_says(0, "DDERR_INVALIDMODE", -1, -1);
        break;
    case DDERR_INVALIDOBJECT:
        parse_says(0, "DDERR_INVALIDOBJECT", -1, -1);
        break;
    case DDERR_INVALIDPIXELFORMAT:
        parse_says(0, "DDERR_INVALIDPIXELFORMAT", -1, -1);
        break;
    case DDERR_INVALIDRECT:
        parse_says(0, "DDERR_INVALIDRECT", -1, -1);
        break;
    case DDERR_LOCKEDSURFACES:
        parse_says(0, "DDERR_LOCKEDSURFACES", -1, -1);
        break;
    case DDERR_NO3D:
        parse_says(0, "DDERR_NO3D", -1, -1);
        break;
    case DDERR_NOALPHAHW:
        parse_says(0, "DDERR_NOALPHAHW", -1, -1);
        break;
    case DDERR_NOCLIPLIST:
        parse_says(0, "DDERR_NOCLIPLIST", -1, -1);
        break;
    case DDERR_NOCOLORCONVHW:
        parse_says(0, "DDERR_NOCOLORCONVHW", -1, -1);
        break;
    case DDERR_NOCOOPERATIVELEVELSET:
        parse_says(0, "DDERR_NOCOOPERATIVELEVELSET", -1, -1);
        break;
    case DDERR_NOCOLORKEYHW:
        parse_says(0, "DDERR_NOCOLORKEYHW", -1, -1);
        break;
    case DDERR_NODIRECTDRAWSUPPORT:
        parse_says(0, "DDERR_NODIRECTDRAWSUPPORT", -1, -1);
        break;
    case DDERR_NOEXCLUSIVEMODE:
        parse_says(0, "DDERR_NOEXCLUSIVEMODE", -1, -1);
        break;
    case DDERR_NOFLIPHW:
        parse_says(0, "DDERR_NOFLIPHW", -1, -1);
        break;
    case DDERR_NOGDI:
        parse_says(0, "DDERR_NOGDI", -1, -1);
        break;
    case DDERR_NOMIRRORHW:
        parse_says(0, "DDERR_NOMIRRORHW", -1, -1);
        break;
    case DDERR_NOTFOUND:
        parse_says(0, "DDERR_NOTFOUND", -1, -1);
        break;
    case DDERR_NOOVERLAYHW:
        parse_says(0, "DDERR_NOOVERLAYHW", -1, -1);
        break;
    case DDERR_NORASTEROPHW:
        parse_says(0, "DDERR_NORASTEROPHW", -1, -1);
        break;
    case DDERR_NOROTATIONHW:
        parse_says(0, "DDERR_NOROTATIONHW", -1, -1);
        break;
    case DDERR_NOSTRETCHHW:
        parse_says(0, "DDERR_NOSTRETCHHW", -1, -1);
        break;
    case DDERR_NOT4BITCOLOR:
        parse_says(0, "DDERR_NOT4BITCOLOR", -1, -1);
        break;
    case DDERR_NOT4BITCOLORINDEX:
        parse_says(0, "DDERR_NOT4BITCOLORINDEX", -1, -1);
        break;
    case DDERR_NOT8BITCOLOR:
        parse_says(0, "DDERR_NOT8BITCOLOR", -1, -1);
        break;
    case DDERR_NOTEXTUREHW:
        parse_says(0, "DDERR_NOTEXTUREHW", -1, -1);
        break;
    case DDERR_NOVSYNCHW:
        parse_says(0, "DDERR_NOVSYNCHW", -1, -1);
        break;
    case DDERR_NOZBUFFERHW:
        parse_says(0, "DDERR_NOZBUFFERHW", -1, -1);
        break;
    case DDERR_NOZOVERLAYHW:
        parse_says(0, "DDERR_NOZOVERLAYHW", -1, -1);
        break;
    case DDERR_OUTOFCAPS:
        parse_says(0, "DDERR_OUTOFCAPS", -1, -1);
        break;
    case DDERR_OUTOFVIDEOMEMORY:
        parse_says(0, "DDERR_OUTOFVIDEOMEMORY", -1, -1);
        break;
    case DDERR_OVERLAYCANTCLIP:
        parse_says(0, "DDERR_OVERLAYCANTCLIP", -1, -1);
        break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
        parse_says(0, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE", -1, -1);
        break;
    case DDERR_PALETTEBUSY:
        parse_says(0, "DDERR_PALETTEBUSY", -1, -1);
        break;
    case DDERR_COLORKEYNOTSET:
        parse_says(0, "DDERR_COLORKEYNOTSET", -1, -1);
        break;
    case DDERR_HWNDALREADYSET:
        parse_says(0, "DDERR_HWNDALREADYSET", -1, -1);
        break;
    case DDERR_SURFACEALREADYDEPENDENT:
        parse_says(0, "DDERR_SURFACEALREADYDEPENDENT", -1, -1);
        break;
    case DDERR_SURFACEBUSY:
        parse_says(0, "DDERR_SURFACEBUSY", -1, -1);
        break;
    case DDERR_CANTLOCKSURFACE:
        parse_says(0, "DDERR_CANTLOCKSURFACE", -1, -1);
        break;
    case DDERR_SURFACEISOBSCURED:
        parse_says(0, "DDERR_SURFACEISOBSCURED", -1, -1);
        break;
    case DDERR_SURFACELOST:
        parse_says(0, "DDERR_SURFACELOST", -1, -1);
        break;
    case DDERR_SURFACENOTATTACHED:
        parse_says(0, "DDERR_SURFACENOTATTACHED", -1, -1);
        break;
    case DDERR_TOOBIGHEIGHT:
        parse_says(0, "DDERR_TOOBIGHEIGHT", -1, -1);
        break;
    case DDERR_TOOBIGSIZE:
        parse_says(0, "DDERR_TOOBIGSIZE", -1, -1);
        break;
    case DDERR_TOOBIGWIDTH:
        parse_says(0, "DDERR_TOOBIGWIDTH", -1, -1);
        break;
    case DDERR_UNSUPPORTEDFORMAT:
        parse_says(0, "DDERR_UNSUPPORTEDFORMAT", -1, -1);
        break;
    case DDERR_UNSUPPORTEDMASK:
        parse_says(0, "DDERR_UNSUPPORTEDMASK", -1, -1);
        break;
    case DDERR_VERTICALBLANKINPROGRESS:
        parse_says(0, "DDERR_VERTICALBLANKINPROGRESS", -1, -1);
        break;
    case DDERR_WASSTILLDRAWING:
        parse_says(0, "DDERR_WASSTILLDRAWING", -1, -1);
        break;
    case DDERR_XALIGN:
        parse_says(0, "DDERR_XALIGN", -1, -1);
        break;
    case DDERR_INVALIDDIRECTDRAWGUID:
        parse_says(0, "DDERR_INVALIDDIRECTDRAWGUID", -1, -1);
        break;
    case DDERR_DIRECTDRAWALREADYCREATED:
        parse_says(0, "DDERR_DIRECTDRAWALREADYCREATED", -1, -1);
        break;
    case DDERR_NODIRECTDRAWHW:
        parse_says(0, "DDERR_NODIRECTDRAWHW", -1, -1);
        break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
        parse_says(0, "DDERR_PRIMARYSURFACEALREADYEXISTS", -1, -1);
        break;
    case DDERR_NOEMULATION:
        parse_says(0, "DDERR_NOEMULATION", -1, -1);
        break;
    case DDERR_REGIONTOOSMALL:
        parse_says(0, "DDERR_REGIONTOOSMALL", -1, -1);
        break;
    case DDERR_CLIPPERISUSINGHWND:
        parse_says(0, "DDERR_CLIPPERISUSINGHWND", -1, -1);
        break;
    case DDERR_NOCLIPPERATTACHED:
        parse_says(0, "DDERR_NOCLIPPERATTACHED", -1, -1);
        break;
    case DDERR_NOHWND:
        parse_says(0, "DDERR_NOHWND", -1, -1);
        break;
    case DDERR_HWNDSUBCLASSED:
        parse_says(0, "DDERR_HWNDSUBCLASSED", -1, -1);
        break;
    case DDERR_NOPALETTEATTACHED:
        parse_says(0, "DDERR_NOPALETTEATTACHED", -1, -1);
        break;
    case DDERR_NOPALETTEHW:
        parse_says(0, "DDERR_NOPALETTEHW", -1, -1);
        break;
    case DDERR_BLTFASTCANTCLIP:
        parse_says(0, "DDERR_BLTFASTCANTCLIP", -1, -1);
        break;
    case DDERR_NOBLTHW:
        parse_says(0, "DDERR_NOBLTHW", -1, -1);
        break;
    case DDERR_NODDROPSHW:
        parse_says(0, "DDERR_NODDROPSHW", -1, -1);
        break;
    case DDERR_OVERLAYNOTVISIBLE:
        parse_says(0, "DDERR_OVERLAYNOTVISIBLE", -1, -1);
        break;
    case DDERR_NOOVERLAYDEST:
        parse_says(0, "DDERR_NOOVERLAYDEST", -1, -1);
        break;
    case DDERR_INVALIDPOSITION:
        parse_says(0, "DDERR_INVALIDPOSITION", -1, -1);
        break;
    case DDERR_NOTAOVERLAYSURFACE:
        parse_says(0, "DDERR_NOTAOVERLAYSURFACE", -1, -1);
        break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
        parse_says(0, "DDERR_EXCLUSIVEMODEALREADYSET", -1, -1);
        break;
    case DDERR_NOTFLIPPABLE:
        parse_says(0, "DDERR_NOTFLIPPABLE", -1, -1);
        break;
    case DDERR_CANTDUPLICATE:
        parse_says(0, "DDERR_CANTDUPLICATE", -1, -1);
        break;
    case DDERR_NOTLOCKED:
        parse_says(0, "DDERR_NOTLOCKED", -1, -1);
        break;
    case DDERR_CANTCREATEDC:
        parse_says(0, "DDERR_CANTCREATEDC", -1, -1);
        break;
    case DDERR_NODC:
        parse_says(0, "DDERR_NODC", -1, -1);
        break;
    case DDERR_WRONGMODE:
        parse_says(0, "DDERR_WRONGMODE", -1, -1);
        break;
    case DDERR_IMPLICITLYCREATED:
        parse_says(0, "DDERR_IMPLICITLYCREATED", -1, -1);
        break;
    case DDERR_NOTPALETTIZED:
        parse_says(0, "DDERR_NOTPALETTIZED", -1, -1);
        break;
    case DDERR_UNSUPPORTEDMODE:
        parse_says(0, "DDERR_UNSUPPORTEDMODE", -1, -1);
        break;
    case DDERR_NOMIPMAPHW:
        parse_says(0, "DDERR_NOMIPMAPHW", -1, -1);
        break;
    case DDERR_INVALIDSURFACETYPE:
        parse_says(0, "DDERR_INVALIDSURFACETYPE", -1, -1);
        break;
    case DDERR_CANTPAGELOCK:
        parse_says(0, "DDERR_CANTPAGELOCK", -1, -1);
        break;
    case DDERR_CANTPAGEUNLOCK:
        parse_says(0, "DDERR_CANTPAGEUNLOCK", -1, -1);
        break;
    case DDERR_NOTPAGELOCKED:
        parse_says(0, "DDERR_NOTPAGELOCKED", -1, -1);
        break;
    default:
        break;
        break;
    }
    if (pop_caption_title(const_cast<char *>("jackal"),
                          const_cast<char *>("DDRAWERROR"), 0x10040,
                          nullptr) == -1) {
        return 0;
    }
    return cd_check();
}



// ===== homed from src/unrecovered/006357d0.cpp =====
// The original calls CRT memset here; keep that target rather than a
// reconstruction-only wrapper or an intrinsic expansion.
#if defined(_MSC_VER)
#pragma function(memset)
#endif
// ORIGINAL: 0x006357D0 sub_6357d0 0x006357D0-0x00635835 FILE BYTE_EXACT
// symbol    ?lock_surface@DDInit@@QAEPAXXZ
// size      101 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x00635814
// NAMED from the body and its address: it sits directly after
// DDInit::teardown (0x00635750) and reads exactly DDInit's own
// `locked_surface_`/`locked_bits_` pair at 0xC/0x10, so it is DDInit's
// surface lock - the lock half of the same pair `teardown` unlocks.
//
// The descriptor is the SDK's own `DDSURFACEDESC` - 0x6C bytes, `lPitch` at
// 0x10, `lpSurface` at 0x24, which is where the image reads. 0x21 is
// DDLOCK_WAIT | DDLOCK_WRITEONLY, left as the literal: no other body here
// spells that pair, so there is nothing to name it against.
//
// LEVER: the pitch is a REAL MEMBER. The scaffold reached it as
// `*(int *)(self + 0x14)` - the one field the five-offset survey of
// WinDisplayInit in win.h had not accounted for - and declaring `pitch_`
// turned the raw store into the image's own `mov [esi+0x14], eax`.
LPVOID DDInit::lock_surface() {
    if (locked_surface_ == nullptr) {
        return nullptr;
    }
    if (locked_bits_ != nullptr) {
        return locked_bits_;
    }
    DDSURFACEDESC description;
    memset(&description, 0, sizeof(description));
    description.dwSize = sizeof(description);
    if (locked_surface_->Lock(nullptr, &description, 0x21, nullptr) != 0) {
        return nullptr;
    }
    pitch_ = description.lPitch;
    locked_bits_ = description.lpSurface;
    return locked_bits_;
}
#if defined(_MSC_VER)
#pragma intrinsic(memset)
#endif


// ===== homed from src/recovered/00635840.cpp =====
// ORIGINAL: 0x00635840 sub_635840 0x00635840-0x00635864 BYTE_EXACT
// symbol    ?flush@DDInit@@QAEXXZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind       game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00635855
// NAMED from its address and its fields: it sits between DDInit::teardown
// (0x00635750) and DDInit::lock_surface (0x006357D0) and reads exactly
// DDInit's locked_surface_ (0xC) / locked_bits_ (0x10) pair, calling the
// surface's slot-0x80 Unlock - the unlock half of the pair as its own entry
// point, which `teardown` repeats inline. The scaffold called the class
// `Handler` because the scaffold did not know DDInit existed.
void DDInit::flush() {
    if (locked_surface_ != nullptr && locked_bits_ != nullptr) {
        locked_surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
    }
}

