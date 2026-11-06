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
#include <cstring>
#include <ddraw.h>
#include "basepop.h"
#include "buffer.h"
#include "general.h"
#include "jackal/palette.h"
#include "popup.h"
#include "win.h"

// Shared IntersectRect import identity is owned by graphicwin.cpp.
// RECOVERY_IMPORT: v1 __imp__EqualRect@8 USER32.dll name:EqualRect
// RECOVERY_IMPORT: v1 __imp__DirectDrawEnumerateA@8 DDRAW.dll name:DirectDrawEnumerateA
// RECOVERY_IMPORT: v1 __imp__DirectDrawCreate@12 DDRAW.dll name:DirectDrawCreate

// SDK dxguid-owned identities already consumed by UV2Player and MediaRuntime:
// IID_IDirectDrawSurface2 = 57805885-6eec-11cf-9441-a82303c10e27 (66FC28);
// IID_IDirectDraw2 = b3a6f3e0-2b43-11cf-a2de-00aa00b93356 (66FC38).
// No new GUID definitions or fixed-address aliases are introduced.

// Shared selected-display state, beside its lifecycle routines.
// All newly owned storage is native zero-fill; no explicit initialization fixes.
// RECOVERY_OBJECT: v2 0x009BC160 0x009BC2CC ?DirectDrawEmulationCaps@@3U_DDCAPS@@A 0x0062CD50
DDCAPS DirectDrawEmulationCaps;
// RECOVERY_OBJECT: v2 0x009BC2CC 0x009BC2D0 ?DirectDrawDeviceCount@@3HA 0x0062C860
int DirectDrawDeviceCount;
// A provisional ten-slot source capacity, covering the access window before
// the independently identified caps object. Adjacency does NOT prove an
// original allocation extent or bound. The callback has NO bounds check;
// overflow effects and the original capacity remain restoration obligations.
// RECOVERY_OBJECT: v1 0x009BC2E0 ?DirectDrawDeviceGuids@@3PAPAU_GUID@@A
GUID *DirectDrawDeviceGuids[10];
// RECOVERY_OBJECT: v2 0x009BC308 0x009BC474 ?DirectDrawDriverCaps@@3U_DDCAPS@@A 0x0062CD50
DDCAPS DirectDrawDriverCaps;
// RECOVERY_OBJECT: v2 0x009BC478 0x009BC47C ?DDActive@@3HA 0x0062CD50
int DDActive;
// RECOVERY_OBJECT: v2 0x009BC484 0x009BC488 ?DirectDrawDensityLimit@@3KA 0x0062CD10
DWORD DirectDrawDensityLimit;
// Separate from WinSharedHdc9B7B2C despite sharing WinHdcRefCount9B3AB0.
// RECOVERY_OBJECT: v2 0x009BC48C 0x009BC490 ?DirectDrawHdc@@3PAUHDC__@@A 0x0062D160
HDC DirectDrawHdc;
// RECOVERY_OBJECT: v2 0x009BC4A4 0x009BC4A8 ?DirectDrawClipper@@3PAUIDirectDrawClipper@@A 0x0062CD50
IDirectDrawClipper *DirectDrawClipper;
// RECOVERY_OBJECT: v2 0x009BC4A8 0x009BC4AC ?DirectDrawSelectionCancelled@@3HA 0x0062C8E0
int DirectDrawSelectionCancelled;

static_assert(sizeof(DDCAPS) == 0x16C,
              "shared display caps require the pinned SDK DirectDraw5 layout");
static_assert(sizeof(DDSURFACEDESC) == 0x6C,
              "shared display description must retain the native layout");

// Typed product bodies. Mapping records are generated separately by the native
// source writer from the complete existing seed/owner records.

// ORIGINAL: 0x0062C860 Callback 0x0062C860-0x0062C8D7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:62C860..62D38E", "62CE7F incoming ESP+90 after net ESP-8C proves bit_depth argument, not uninitialized EDI", "62CD70/62D118 and62D0D6 write ScreenBuffer+58/+5C; these are members, not DDCached globals", "SDK DDRAW.H Draw2/Surface2 IIDs and genuine slot signatures;62D297/62D2A9 are Clipper::SetClipList/Surface2::SetClipper", "Original selector cold lifetime span663340..663494 retained through real automatic BasePop", "62D160/62D1A0 use separate HDC9BC48C but shared WinHdcRefCount9B3AB0;62D2C0 retains final HRESULT"], "reason": "Recover shared DirectDraw selection/init/DC/clip lifetime as a complete typed owner in general.cpp, reuse actual Buffer/Win/Palette storage, and retain all original claims, spans and historical seed findings. Gamma adjacency routing is not ownership proof. No byte, linked-preservation or runtime acceptance is claimed."}
// symbol    ?direct_draw_device_callback@@YGHPAU_GUID@@PAD1PAX@Z
// kind game
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
BOOL CALLBACK direct_draw_device_callback(GUID *guid, LPSTR description,
                                         LPSTR /*driver_name*/, LPVOID context) {
    BasePop *popup = static_cast<BasePop *>(context);
    popup->dialogs().item(description, DirectDrawDeviceCount);
    if (guid == 0) {
        DirectDrawDeviceGuids[DirectDrawDeviceCount] = 0;
        ++DirectDrawDeviceCount;
        return 1;
    }
    GUID *allocated = static_cast<GUID *>(mem_get(16));
    DirectDrawDeviceGuids[DirectDrawDeviceCount] = allocated;
    if (allocated != 0) {
        ++DirectDrawDeviceCount;
        memcpy(allocated, guid, 16);
    }
    return 1;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

// ORIGINAL: 0x0062C8E0 sub_62c8e0 0x0062C8E0-0x0062CD02;0x00663340-0x00663494
// symbol    ?select_direct_draw_device@@YAPAU_GUID@@XZ
// TRIED: a giant (0x3234-byte) BasePop+Spot+Dialogs(embeds Dialog)+two list entries+Sprite+two FlatButtons+Heap local, driven by direct calls at disassembly offsets into a raw `unsigned char buf[]` rather than as real RAII C++ locals - the three tails' explicit vtable-patch-then-close()/dtor sequences (e.g. writing g_006698d4/g_006698cc into the BasePop object right before close()) are themselves that literal, so the raw-offset transcription matches the disassembly's own shape. Did NOT attempt real nested locals of type BasePop/Dialogs/etc: the emitted `Dialogs` class embeds `GraphicWin virtual_base_` at 0x188, but the disassembly calls `??1Dialogs@@QAE@XZ` and `??1GraphicWin@@QAE@XZ` on the IDENTICAL address twice running, which only matches non-virtual `Dialogs : public GraphicWin` (base at offset 0) - the two class shapes disagree, so a real local risked wrong offsets on top of not getting the SEH frame either. MISMATCH #0 (`mov` vs `push`, no SEH frame at all is generated without a real object of non-trivial type as a local) - the whole-function control flow (three-way branch on BasePop::exec()'s result, matching Ghidra's own decompilation) and every call's argument order (BasePop::init/exec, DirectDrawEnumerateA, the two sub_4066c0 calls whose result feeds g_009b3374) is re-derived from the push order, not chased to byte-exactness.
// kind game
GUID *__cdecl select_direct_draw_device() {
    BasePop popup;
    popup.init(0x42, 0);
    DirectDrawDeviceCount = 0;
    DirectDrawEnumerateA(direct_draw_device_callback, &popup);
    const int selected = popup.exec(0, 0);
    if (selected == -1) {
        DirectDrawSelectionCancelled = 1;
        return 0;
    }
    if (selected >= 0 && selected < DirectDrawDeviceCount) {
        return DirectDrawDeviceGuids[selected];
    }
    return 0;
}

// ORIGINAL: 0x0062CD10 sub_62cd10 0x0062CD10-0x0062CD48 BYTE_EXACT
// symbol    ?sub_62cd10@@YGJPAU_DDSURFACEDESC@@PAX@Z
// kind game
HRESULT CALLBACK sub_62cd10(LPDDSURFACEDESC description, LPVOID context) {
    const DWORD density = (description->dwBackBufferCount + 1u) *
        static_cast<DWORD>(description->lPitch) * description->dwHeight * 2u;
    if (density < DirectDrawDensityLimit) {
        const DWORD width = description->dwWidth;
        if (width <= 0x400u) {
            DWORD *selected_width = static_cast<DWORD *>(context);
            if (width > *selected_width) {
                *selected_width = width;
            }
        }
    }
    return 1;
}

// ORIGINAL: 0x0062CD50 sub_62cd50 0x0062CD50-0x0062D0ED
// symbol    ?sub_62cd50@@YAHH@Z
// TRIED: nullary/int-arg per the contract/IDA guess - no stack param reads exist, so kept nullary; DirectDraw init sequence with slots {0,2,4,5,6,8,11,12,20,21,31} matches the real IDirectDraw2/IDirectDrawSurface vtable orders exactly (QueryInterface/Release/CreateClipper/CreatePalette/ CreateSurface/EnumDisplayModes/GetCaps/GetDisplayMode/ SetCooperativeLevel/SetDisplayMode/SetPalette), confirming the derived arg counts per push-site. One value ([esp+0x90], Ghidra's unaff_EDI) reads stack memory past this function's own frame in the original and is not reproducible in C; replaced with a zero-initialized local. Frame layout differs from instruction #0 given the size/local count; not pursued further.
// kind game
int __cdecl sub_62cd50(int bit_depth) {
    if (BufferDirectDraw != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
    }
    Win::screen_buffer_.surface_ = 0;
    Win::screen_buffer_.clipper_ = 0;
    DirectDrawSurface = 0;
    DirectDrawBackBuffer = 0;
    DDActive = 0;
    DirectDrawSelectionCancelled = 0;
    GUID *selected = select_direct_draw_device();
    if (DirectDrawSelectionCancelled != 0) {
        return 0;
    }

    IDirectDraw *created;
    if (DirectDrawCreate(selected, &created, 0) != 0) {
        return 0x12;
    }
    if (created->QueryInterface(IID_IDirectDraw2,
                               reinterpret_cast<void **>(&BufferDirectDraw)) != 0) {
        created->Release();
        return 0x12;
    }
    created->Release();

    // One reused native DDSURFACEDESC, not three zero-filled descriptors.
    DDSURFACEDESC description;
    description.dwSize = 0x6C;
    if (BufferDirectDraw->GetDisplayMode(&description) != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
        return 0x12;
    }
    DirectDrawDriverCaps.dwSize = 0x16C;
    DirectDrawEmulationCaps.dwSize = 0x16C;
    if (BufferDirectDraw->GetCaps(&DirectDrawDriverCaps,
                                 &DirectDrawEmulationCaps) != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
        return 0x12;
    }
    DirectDrawDensityLimit = (description.dwBackBufferCount + 1u) *
        description.dwHeight * static_cast<DWORD>(description.lPitch) +
        DirectDrawDriverCaps.dwVidMemFree;

    {
        DWORD selected_width;
        description.dwSize = 0x6C;
        description.dwFlags = 0x1000;
        description.ddpfPixelFormat.dwSize = 0x20;
        description.ddpfPixelFormat.dwFlags = 0x40;
        description.ddpfPixelFormat.dwRGBBitCount = bit_depth;
        selected_width = 0;
        if (BufferDirectDraw->EnumDisplayModes(0, &description, &selected_width,
                                              sub_62cd10) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        if (selected_width == 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        if (BufferDirectDraw->SetCooperativeLevel(HandleMain, 0x13) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        // VC6's signed (width * 3) / 4, even if a callback supplied a
        // width outside the ordinary 0..1024 callback range.
        const int product = static_cast<int>(selected_width * 3u);
        if (BufferDirectDraw->SetDisplayMode(selected_width, product / 4,
                                            bit_depth, 0, 0) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        // Reload selected_width after SetDisplayMode: callbacks may change it.
        DirectDrawClipRect.right = selected_width;
        DirectDrawClipRect.left = 0;
        DirectDrawClipRect.top = 0;
        DirectDrawClipRect.bottom =
            static_cast<int>(selected_width * 3u) / 4;

        IDirectDrawSurface *primary;
        IDirectDrawSurface *backbuffer;
        description.dwSize = 0x6C;
        description.dwFlags = 0x21;
        description.ddsCaps.dwCaps = 0x4218;
        description.dwBackBufferCount = 1;
        if (BufferDirectDraw->CreateSurface(&description, &primary, 0) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        DDSCAPS backbuffer_caps;
        backbuffer_caps.dwCaps = 4;
        if (primary->GetAttachedSurface(&backbuffer_caps, &backbuffer) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        primary->QueryInterface(IID_IDirectDrawSurface2,
                               reinterpret_cast<void **>(&DirectDrawSurface));
        backbuffer->QueryInterface(IID_IDirectDrawSurface2,
                                  reinterpret_cast<void **>(&DirectDrawBackBuffer));
        if (bit_depth == 8 &&
            BufferDirectDraw->CreatePalette(
                // Native forwards the Palette's proven entries_ prefix (+0),
                // including a null PaletteActive; no null-this accessor call.
                4, reinterpret_cast<PALETTEENTRY *>(PaletteActive),
                &DirectDrawPalette, 0) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        DirectDrawSurface->SetPalette(DirectDrawPalette);
        DirectDrawBackBuffer->SetPalette(DirectDrawPalette);
        if (BufferDirectDraw->CreateClipper(0, &DirectDrawClipper, 0) != 0) {
            BufferDirectDraw->Release();
            BufferDirectDraw = 0;
            return 0x12;
        }
        DDActive = 1;
        Win::screen_buffer_.close();
        Win::screen_buffer_.locked_bits_ = 0;
        Win::screen_buffer_.hdc2_ = 0;
        Win::screen_buffer_.surface_ = DirectDrawBackBuffer;
        Win::screen_buffer_.clipper_ = DirectDrawClipper;
    }
    return 0;
}

// ORIGINAL: 0x0062D0F0 sub_62d0f0 0x0062D0F0-0x0062D0F8 BYTE_EXACT
// symbol    ?sub_62d0f0@@YAHXZ
// TRIED: homed into general.cpp 2026-09-03 - does not link: the body's only call
// kind game
int __cdecl sub_62d0f0() {
    select_direct_draw_device();
    return 0;
}

// ORIGINAL: 0x0062D100 sub_62d100 0x0062D100-0x0062D13A BYTE_EXACT
// symbol    _sub_62d100
// kind game
extern "C" int __cdecl sub_62d100() {
    if (BufferDirectDraw != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
    }
    Win::screen_buffer_.surface_ = 0;
    Win::screen_buffer_.clipper_ = 0;
    DirectDrawSurface = 0;
    DirectDrawBackBuffer = 0;
    DDActive = 0;
    return 0;
}

// ORIGINAL: 0x0062D160 sub_62d160 0x0062D160-0x0062D191 BYTE_EXACT
// symbol    ?sub_62d160@@YAPAUHDC__@@XZ
// kind game
HDC __cdecl sub_62d160() {
    HDC handle = DirectDrawHdc;
    WinHdcRefCount = static_cast<int>(static_cast<DWORD>(WinHdcRefCount) + 1u);
    IDirectDrawSurface2 *surface = DirectDrawSurface;
    if (surface != 0 && handle == 0) {
        surface->GetDC(&DirectDrawHdc);
        handle = DirectDrawHdc;
    }
    return handle;
}

// ORIGINAL: 0x0062D1A0 sub_62d1a0 0x0062D1A0-0x0062D1D2 BYTE_EXACT
// symbol    ?sub_62d1a0@@YAXXZ
// TRIED: homed into general.cpp 2026-09-03 - does not link: the body reads and
// kind game
void __cdecl sub_62d1a0() {
    const int count = WinHdcRefCount;
    IDirectDrawSurface2 *surface = DirectDrawSurface;
    WinHdcRefCount = static_cast<int>(static_cast<DWORD>(count) - 1u);
    if (surface != 0) {
        HDC handle = DirectDrawHdc;
        if (handle != 0) {
            surface->ReleaseDC(handle);
            DirectDrawHdc = 0;
        }
    }
}

// RGNDATA has a variable-sized trailing byte buffer. This owns the exact
// native header and its one RECT without pretending Buffer[1] is large enough.
struct DirectDrawClipRegion {
    RGNDATAHEADER header;
    RECT rectangle;
};

// ORIGINAL: 0x0062D1E0 sub_62d1e0 0x0062D1E0-0x0062D2B1 BYTE_EXACT
// symbol    ?sub_62d1e0@@YAXPAUtagRECT@@@Z
// TRIED: 209 vs 204 bytes; struct-literal field order for the 0x30-byte DrawRegion local drifts the compiler's address-of scheduling (lea moves relative to the field stores) versus the original's register reuse. Tried plain field assigns and an early `DrawRegion *preg = &region;` alias; neither matched.
// kind game
void __cdecl sub_62d1e0(RECT *clip_rect) {
    if (DirectDrawBackBuffer == 0 || clip_rect == 0) {
        return;
    }
    RECT intersection;
    if (!IntersectRect(&intersection, &DirectDrawClipRect, clip_rect)) {
        return;
    }
    if (EqualRect(&DirectDrawClipRect, &intersection)) {
        DirectDrawBackBuffer->SetClipper(0);
        return;
    }
    DirectDrawClipRegion region;
    region.header.dwSize = 0x20;
    region.header.iType = 1;
    region.header.nCount = 1;
    region.header.nRgnSize = 0x10;
    region.header.rcBound = intersection;
    region.rectangle = intersection;
    DirectDrawClipper->SetClipList(reinterpret_cast<RGNDATA *>(&region), 0);
    // Both globals are reloaded after SetClipList, not cached before it.
    DirectDrawBackBuffer->SetClipper(DirectDrawClipper);
}

// ORIGINAL: 0x0062D2C0 sub_62d2c0 0x0062D2C0-0x0062D38E
// symbol    ?sub_62d2c0@@YAHXZ
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: IntersectRect/EqualRect are called through the fixed IAT-style slots (0x669338/0x6692bc) with `this` pushed as an explicit stdcall arg, not via C++ virtual dispatch - modelled with raw `PFN_*` casts. Every early-out in the original shares ONE epilogue (add esp,0x40; ret); separate `if (x) return 0;` statements each get their own inline return instead of jumping to a shared tail, which is most of the 0.82 mnemonic-similarity gap.
// kind game
int __cdecl sub_62d2c0() {
    if (DirectDrawBackBuffer == 0) {
        return 0;
    }
    RECT intersection;
    if (!IntersectRect(&intersection, &DirectDrawClipRect, &DirectDrawClipRect)) {
        return 0;
    }
    if (EqualRect(&DirectDrawClipRect, &intersection)) {
        return DirectDrawBackBuffer->SetClipper(0);
    }
    DirectDrawClipRegion region;
    region.header.dwSize = 0x20;
    region.header.iType = 1;
    region.header.nCount = 1;
    region.header.nRgnSize = 0x10;
    region.header.rcBound = intersection;
    region.rectangle = intersection;
    DirectDrawClipper->SetClipList(reinterpret_cast<RGNDATA *>(&region), 0);
    return DirectDrawBackBuffer->SetClipper(DirectDrawClipper);
}

