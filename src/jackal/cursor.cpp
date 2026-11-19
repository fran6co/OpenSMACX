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
#include "cursor.h"
#include "buffer.h"
#include "palette.h"
#include <new>
#include "sprite.h"   // the Sprite ~Cursor owns

// RECOVERY_IMPORT: v1 __imp__DestroyCursor@4 USER32.dll name:DestroyCursor

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0063B930 ?close_cursor_class@Cursor@@QAAXXZ 0x0063B930-0x0063B931 BYTE_EXACT
// symbol    ?close_cursor_class@Cursor@@SAXXZ
// size      1 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Cursor::close_cursor_class() {
}


// RECOVERY_OBJECT: v1 0x009BE6D4 ?CursorWidth@@3HA
int CursorWidth;   // 0x009BE6D4
// RECOVERY_OBJECT: v1 0x009BE6D8 ?CursorHeight@@3HA
int CursorHeight;  // 0x009BE6D8

/*
Purpose: Cache the system cursor's width and height.
// ORIGINAL: 0x0063B910 ?init_cursor_class@Cursor@@QAAHXZ 0x0063B910-0x0063B92D BYTE_EXACT
// symbol    ?init_cursor_class@Cursor@@SAHXZ
// size      29 bytes
// prototype int (__cdecl ?init_cursor_class@Cursor@@QAAHXZ)()
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int Cursor::init_cursor_class() {
    CursorWidth = GetSystemMetrics(SM_CXCURSOR);
    CursorHeight = GetSystemMetrics(SM_CYCURSOR);
    return 0;
}

/*
Purpose: Clear all four fields.

             mov eax,ecx / xor ecx,ecx
             mov [eax],ecx / mov [eax+8],ecx / mov [eax+0xC],ecx
             mov [eax+4],ecx / ret

         The genuine constructor retains the original store order 0,+8,+C,+4.
// ORIGINAL: 0x0063B2D0 ??0Cursor@@QAE@XZ 0x0063B2D0-0x0063B2E0 BYTE_EXACT
// symbol    ??0Cursor@@QAE@XZ
// size      16 bytes
// prototype Cursor* (__thiscall ??0Cursor@@QAE@XZ)(Cursor* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: this
Status: Complete
*/
Cursor::Cursor() {
    cursor_handle_ = 0;
    hotspot_x_ = 0;
    hotspot_y_ = 0;
    owned_sprite_ = 0;
}



// Owns the Windows cursor and copied Sprite; the definition follows its mapping.
// ORIGINAL: 0x0063B8D0 ??1Cursor@@QAE@XZ 0x0063B8D0-0x0063B90A BYTE_EXACT
Cursor::~Cursor() {
    // `cursor_handle_` is the HCURSOR and `owned_sprite_` a Sprite the object owns;
    // the image destroys both and then clears the two words after them.
    HCURSOR handle = cursor_handle_;
    if (handle) {
        DestroyCursor(handle);
        cursor_handle_ = 0;
    }
    Sprite *sprite = owned_sprite_;
    if (sprite) {
        delete sprite;
        owned_sprite_ = 0;
    }
    hotspot_x_ = 0;
    hotspot_y_ = 0;
}

// ORIGINAL: 0x0063B2E0 ?init@Cursor@@QAEXPAUSprite@@HH@Z 0x0063B2E0-0x0063B8CA;0x00663910-0x0066393E
// RECOVERY_BINDING: v1 {"evidence": ["task:cursor-init-owner", "pinned:63B42F..63B443/63B4A2..63B4BA signed truncating half and independent centering reloads", "pinned:63B6EF..63B728 color DIB dimensions snapshotted before get_hdc;63B734..63B7A8 live hotspot rereads", "source:Buffer DIB palette, Sprite pixels/dimensions and Cursor handle/owned Sprite/hotspot members", "historical seed:src/unrecovered/0063b2e0.cpp"], "reason": "Recover complete cursor initialization through real automatic Buffers, Sprite ownership and SDK cursor/GDI APIs; preserve native status and failure effects, live callback reads, original primary/cleanup spans and useful seed findings without a new byte claim."}
// symbol    ?init@Cursor@@QAEHPAVSprite@@HH@Z
// TRIED: two real `Buffer` locals (RAII ctor/dtor) + GDI calls through IAT-slot function pointers (CreateCompatibleDC/CreateDIBSection/ SelectObject/SetDIBColorTable/SetDIBits/CreateDCA/GetDeviceCaps/ DeleteDC/GdiFlush/BitBlt/CreateIconIndirect/DeleteObject/ DestroyCursor, per the Ghidra hint) + `new Sprite()` for the icon sprite. Compiles and the compiler DOES emit a matching SEH prologue shape (push -1 first); diverges at instruction #1 (mov fs:[0] vs push), so the frame-setup byte sequence itself isn't reproduced exactly. The BitBlt width/height and the second (256-color) DIB's palette bytes are best-effort guesses (screen width/height, uninitialized buffer) - the deep esp-relative stack juggling around the icon-creation block was not fully re-derived.
int Cursor::init(Sprite *sprite, int hotspot_x, int hotspot_y) {
    Buffer mono;
    Buffer color;

    if (sprite == nullptr) {
        return 3;
    }
    if (sprite->pcBits_ == nullptr) {
        return 7;
    }
    if (BufferDirectDraw != nullptr) {
        return 0;
    }

    if (cursor_handle_ != 0) {
        DestroyCursor(cursor_handle_);
        cursor_handle_ = 0;
    }
    if (owned_sprite_ != 0) {
        Sprite *old_sprite = owned_sprite_;
        delete old_sprite;
        owned_sprite_ = 0;
    }
    hotspot_x_ = 0;
    hotspot_y_ = 0;

    mono.init(CursorWidth, CursorHeight, 0, 0);
    mono.sync_to_palette(PaletteCurrent);
    mono.fill(0xff);
    {
        int y = (CursorHeight - sprite->height()) / 2;
        int x = (CursorWidth - sprite->iWidth_) / 2;
        sprite->draw_mono(&mono, sprite->cTransparentIndex_, x, y, 0);
    }

    color.init(CursorWidth, CursorHeight, 0, 0);
    color.sync_to_palette(PaletteCurrent);
    color.fill(0);
    {
        int y = (CursorHeight - sprite->height()) / 2;
        int x = (CursorWidth - sprite->iWidth_) / 2;
        sprite->draw(&color, sprite->cTransparentIndex_, x, y);
    }

    Dib mono_info;
    mono_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    mono_info.bmiHeader.biWidth = CursorWidth;
    mono_info.bmiHeader.biHeight = -CursorHeight;
    mono_info.bmiHeader.biPlanes = 1;
    mono_info.bmiHeader.biBitCount = 1;
    mono_info.bmiHeader.biCompression = BI_RGB;
    mono_info.bmiHeader.biSizeImage = 0;
    mono_info.bmiHeader.biXPelsPerMeter = 0;
    mono_info.bmiHeader.biYPelsPerMeter = 0;
    mono_info.bmiHeader.biClrUsed = 0;
    mono_info.bmiHeader.biClrImportant = 0;
    mono_info.bmiColors[0].rgbRed = 0;
    mono_info.bmiColors[0].rgbGreen = 0;
    mono_info.bmiColors[0].rgbBlue = 0;
    mono_info.bmiColorsRest[0].rgbRed = 0xff;
    mono_info.bmiColorsRest[0].rgbGreen = 0xff;
    mono_info.bmiColorsRest[0].rgbBlue = 0xff;

    HDC mono_dc = CreateCompatibleDC(nullptr);
    if (mono_dc == nullptr) {
        return 1;
    }
    void *bits;
    HBITMAP mono_bitmap = CreateDIBSection(mono_dc, &mono_info, DIB_RGB_COLORS,
                                           &bits, nullptr, 0);
    if (mono_bitmap == nullptr) {
        DeleteDC(mono_dc);
        return 1;
    }

    SelectObject(mono_dc, mono_bitmap);
    SetDIBColorTable(mono_dc, 0, 2, mono_info.bmiColors);
    SetDIBits(mono_dc, mono_bitmap, 0, CursorHeight,
        mono.get_data(), &mono.dib_, DIB_RGB_COLORS);

    // This is a separate automatic Dib from color.dib_. The pinned stack
    // layout gives it the complete Dib extent; preserve the uninitialized
    // palette and reserved bytes beyond the header fields written here.
    Dib color_info;
    HDC display_dc = CreateDCA("DISPLAY", nullptr, nullptr, nullptr);
    color_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    color_info.bmiHeader.biWidth = CursorWidth;
    color_info.bmiHeader.biHeight = -CursorHeight;
    color_info.bmiHeader.biPlanes = static_cast<WORD>(GetDeviceCaps(display_dc, PLANES));
    color_info.bmiHeader.biBitCount = static_cast<WORD>(GetDeviceCaps(display_dc, BITSPIXEL));
    DeleteDC(display_dc);
    color_info.bmiHeader.biCompression = BI_RGB;
    color_info.bmiHeader.biSizeImage = 0;
    color_info.bmiHeader.biXPelsPerMeter = 0;
    color_info.bmiHeader.biYPelsPerMeter = 0;
    color_info.bmiHeader.biClrUsed = 0;
    color_info.bmiHeader.biClrImportant = 0;

    HDC color_dc = CreateCompatibleDC(nullptr);
    if (color_dc == nullptr) {
        // The original exits here without releasing the mono bitmap/DC.
        return 1;
    }
    HBITMAP color_bitmap = CreateDIBSection(color_dc, &color_info, DIB_RGB_COLORS,
                                            &bits, nullptr, 0);
    if (color_bitmap == nullptr) {
        DeleteDC(color_dc);
        // The original exits here without releasing mono_bitmap/mono_dc.
        return 1;
    }

    SelectObject(color_dc, color_bitmap);
    SetDIBColorTable(color_dc, 0, 256, color.dib_.bmiColors);
    SetDIBColorTable(mono_dc, 0, 2, mono_info.bmiColors);
    GdiFlush();

    const int blit_height = color.pixel_height();
    const int blit_width = color.pixel_width();
    BitBlt(color_dc, 0, 0, blit_width, blit_height, color.get_hdc(), 0, 0,
           SRCCOPY);
    GdiFlush();

    ICONINFO icon_info;
    icon_info.fIcon = FALSE;
    icon_info.xHotspot = (CursorWidth - sprite->iWidth_) / 2 + hotspot_x;
    icon_info.xHotspot = max(0, min(CursorWidth - 1, (int)icon_info.xHotspot));
    icon_info.yHotspot = (CursorHeight - sprite->iHeight_) / 2 + hotspot_y;
    icon_info.yHotspot = max(0, min(CursorHeight - 1, (int)icon_info.yHotspot));
    icon_info.hbmMask = mono_bitmap;
    icon_info.hbmColor = color_bitmap;
    cursor_handle_ = CreateIconIndirect(&icon_info);

    DeleteObject(color_bitmap);
    DeleteDC(color_dc);
    DeleteObject(mono_bitmap);
    DeleteDC(mono_dc);

    if (cursor_handle_ == 0) {
        return 2;
    }
    if (owned_sprite_ == 0) {
        owned_sprite_ = new Sprite;
    }
    if (owned_sprite_ != nullptr) {
        sprite->copy(owned_sprite_);
        hotspot_x_ = hotspot_x;
        hotspot_y_ = hotspot_y;
        return 0;
    } else {
        return 0;
    }
}

