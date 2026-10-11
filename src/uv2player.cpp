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
#include "uv2player.h"
#include <mmsystem.h>
#include "alpha.h"
#include "palette.h"

#include "win.h"
#include "text.h"
#include "stringstruct.h"
#include "wave_device.h"

// Shared help-file pointer; its storage is owned by datalink.cpp.
extern char *DatalinkHelpFile;

/*
Purpose: Construct the Buffer subobject, then clear one field.

             mov esi,ecx / lea ecx,[esi+0x8dc] / call ??0Buffer
             mov dword [esi+0x10c],0 / mov eax,esi / ret

         The Buffer lives at 0x8dc and the cleared field at 0x10c - which is
         BELOW the Buffer, so the store is in this object's own space and not
         inside the subobject. It happens after the constructor, which is the
         order that would matter if the constructor ever reached that far back.

// ORIGINAL: 0x004BEA30 ??0UV2Player@@QAE@XZ 0x004BEA30-0x004BEA4C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ??0UV2Player@@QAE@XZ)(UV2Player* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210
Return Value: `this`
Status: Complete
*/
UV2Player::UV2Player() {
    // buffer_ has completed construction before the native ready pointer clears.
    direct_draw2_ = 0;
}

// The movie input loop and timed writer share these original four-byte slots.
// RECOVERY_OBJECT: v2 0x008CC250 0x008CC254 ?UV2PlayerQuitRequested@@3HA 0x004BF390
int UV2PlayerQuitRequested;

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BF390 ?on_key@UV2Player@@QAAHXZ 0x004BF390-0x004BF3FE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004bf390.cpp", "image:terranx_original.exe:0x004BF390", "4BF3AA/BE reject only zero message results; Return/Escape/Space and either mouse-down set quit8CC250. Timed text disarms Buffer+8, writes individual bytes, reloads delayE64 after each timeGetTime and re-arms the cdecl callback even for nonpositive length.", "recovery/tasks/uv2player-complete-owner-recovery.json; bounded complete-owner scope, preserved historical baseline/claims, no matching or runtime acceptance."], "reason": "Recover the movie input/timed text owner using SDK messages and real Buffer callback, active instance and unsigned delay storage, preserving native input and callback order."}
// symbol    ?on_key@UV2Player@@QAAHXZ
int __cdecl UV2Player::on_key() {
    MSG message;
    if (!PeekMessageA(&message, 0, 0, 0, 0) ||
        !GetMessageA(&message, 0, 0, 0)) {
        return 0;
    }
    if (message.message == WM_KEYDOWN) {
        if (message.wParam != VK_RETURN && message.wParam != VK_ESCAPE &&
            message.wParam != VK_SPACE) {
            return 0;
        }
    } else if (message.message != WM_LBUTTONDOWN &&
               message.message != WM_RBUTTONDOWN) {
        return 0;
    }
    UV2PlayerQuitRequested = 1;
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// LEVER: literal thiscall member with a guarded do/while (a4>0) around the cached timeGetTime slot; nested guarded do/while for the busy-wait.
// ORIGINAL: 0x004BEDC0 ?UNK1@UV2Player@@QAEHHHHH@Z 0x004BEDC0-0x004BEE5A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004bedc0.cpp", "image:terranx_original.exe:0x004BEDC0", "4BF3AA/BE reject only zero message results; Return/Escape/Space and either mouse-down set quit8CC250. Timed text disarms Buffer+8, writes individual bytes, reloads delayE64 after each timeGetTime and re-arms the cdecl callback even for nonpositive length.", "recovery/tasks/uv2player-complete-owner-recovery.json; bounded complete-owner scope, preserved historical baseline/claims, no matching or runtime acceptance."], "reason": "Recover the movie input/timed text owner using SDK messages and real Buffer callback, active instance and unsigned delay storage, preserving native input and callback order."}
// symbol    ?write_timed@UV2Player@@QAEHPADHHH@Z
int UV2Player::write_timed(LPSTR text, int x, int y, int length) {
    buffer_.set_write_raw_callback(0);
    for (int index = 0; index < length; ++index) {
        const DWORD started = timeGetTime();
        x = buffer_.write_raw_l(text + index, x, y, 1);
        while (timeGetTime() - started < write_delay_) {
        }
    }
    buffer_.set_write_raw_callback(&uv2_player_writer_redirect);
    return x;
}

// The one player instance the movie writer serves: 0x008CC254 holds the
// UV2Player whose embedded Buffer the __cdecl writer below walks when it is
// invoked as the buffer's output hook - the writer member's QAA convention
// passes no receiver, so the image routes the object through this slot.
// Modelled as real storage beside its only recovered reader, the
// WorldClimateWorldWin idiom.
// RECOVERY_OBJECT: v2 0x008CC254 0x008CC258 ?UV2PlayerActive@@3PAVUV2Player@@A 0x004BF400
UV2Player *UV2PlayerActive;  // 0x008CC254

/*
Purpose: The buffered writer's re-entry half, run as the embedded Buffer's
         output hook: take the player instance from the active-player slot,
         disarm the hook (so the writes below land raw), then rewrite the
         caller's rows one character at a time through Buffer::write_raw_l
         with the same busy-wait the timed writer uses, and re-arm the hook
         with this function before returning.

// ORIGINAL: 0x004BEE60 ?UNK2@UV2Player@@QAAHHHHH@Z 0x004BEE60-0x004BEEFD FILE
// symbol   _uv2_player_writer_redirect
// size      157 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DBD00
// indirect  0x004BEE93 0x004BEEB8 0x004BEEC6
Return Value: the x coordinate the last write left off at
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl uv2_player_writer_redirect(LPSTR text, int x_coord,
                                                  int y_coord, int row_count) {
    UV2Player *const instance = UV2PlayerActive;
    if (instance == 0) {
        return 0;
    }
    Buffer &buffer = instance->buffer_;
    buffer.set_write_raw_callback(0);
    int x = x_coord;
    int index = 0;
    if (row_count > 0) {
        DWORD(WINAPI *const get_time)() = timeGetTime;
        do {
            const unsigned int started = get_time();
            x = buffer.write_raw_l(text + index, x,
                                    y_coord, 1);
            while (get_time() - started < instance->write_delay_) {
            }
            ++index;
        } while (index < row_count);
    }
    buffer.set_write_raw_callback(&uv2_player_writer_redirect);
    return x;
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004BF380 ?UNK4@UV2Player@@QAEHXZ 0x004BF380-0x004BF383 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK4@UV2Player@@QAEHXZ)(UV2Player* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int UV2Player::UNK4() {
    return 0;
}


/*
Purpose: Tear down a UV2Player: close it, then destroy its Buffer member.
         buffer_ is a real member, so the compiler installs its own SEH
         unwind protection around close() automatically - matching the
         image, which protects the same call so the Buffer subobject is
         still destroyed if close() throws.
// ORIGINAL: 0x004043D0 ??1UV2Player@@QAE@XZ 0x004043D0-0x0040441C;0x00650740-0x00650758 BYTE_EXACT
// LEVER: MEASURED 22/22 agreeing. `Buffer buffer_` being a REAL typed member (not raw storage) is what does it - the compiler installs the exact SEH-wrapped-close()-then-destroy-Buffer shape on its own once the member is real, so the body is just `close();` with no explicit try/catch and no explicit `~Buffer()` call at all.
// size      100 bytes
// prototype void (__thiscall ??1UV2Player@@QAE@XZ)(UV2Player* this)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF400 0x005D7410
Return Value: n/a
Status: Complete
*/
UV2Player::~UV2Player() {
    close();
}

// ORIGINAL: 0x004BF400 ?close@UV2Player@@QAEXXZ 0x004BF400-0x004BF4A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned4BF400..4BF4A2 releases primary Surface2 and Draw2 only; it does not release back buffer or clipper. Clears active8CC254 before COM calls, then applies DirectDraw preference and 800x600/1024x768 restoration.", "Play4BF4B0 creates DirectDraw then queries IID_IDirectDraw2=b3a6f3e0-2b43-11cf-a2de-00aa00b93356 at66FC38; surfaces query IID_IDirectDrawSurface2=57805885-6eec-11cf-9441-a82303c10e27 at66FC28. Pinned SDK DDRAW.H establishes method ABI.", "recovery/tasks/uv2player-complete-owner-recovery.json; retain exact constructor/destructor/timed-writer obligations and source-owned original play mapping. No full structure or runtime acceptance is asserted."], "reason": "Recover movie display teardown with genuine SDK interfaces, source-owned active player, shared preferences/display helpers and native live pointer reloads."}
// TRIED: COM-style vtable (ecx=vtbl, self pushed explicit, callee-pop) via a raw VFn* vtable array rather than the scaffold's thiscall VCall; set_display_mode called through a differently-named free extern since it is QAA/cdecl-static, not the scaffold's non-static member. MISMATCH #29, well past the vtable teardown into the display-mode branch.
void UV2Player::close() {
    UV2PlayerActive = nullptr;
    if (direct_draw2_ != nullptr) {
        direct_draw2_->RestoreDisplayMode();
        primary_surface_->Release();
        primary_surface_ = nullptr;
        direct_draw2_->Release();
        direct_draw2_ = nullptr;

        if (prefs_get("DirectDraw", 1, 0) != 0) {
            if (WinScreenWidth >= 0x400) {
                Win::set_display_mode(0x400, 0x300, 8, 0);
            } else {
                Win::set_display_mode(0x320, 0x258, 8, 0);
            }
        }
    }
    direct_draw_ = nullptr;
    back_buffer_surface_ = nullptr;
    field_11C_ = 0;
    field_120_ = 0;
    media_session_ = 0;
}

/*
Purpose: Bring up the movie player's DirectDraw chain: clear and flip the
         screen buffer, create the DirectDraw object, take the cooperative
         level on the main window, try the display mode at four bit depths,
         create the primary flipping surface with its back buffer and
         clipper, and clear both surfaces through two Blt calls.

// ORIGINAL: 0x004BF4B0 ?play@UV2Player@@QAEHHH@Z 0x004BF4B0-0x004BF6D1 FILE
// TRIED: plain member access vs a `UV2Player *self = this;` local before the first field_10C_ read - both leave the prologue's `this` spill (`mov [ebp-4],edi`) placed differently than the original (MISMATCH #4 'push' vs 'mov', 84% mnemonic similarity); not chased further. The 21-slot COM vtable dispatch (QueryInterface/ Release/SetCooperativeLevel/SetDisplayMode retry chain/ CreateSurface/GetAttachedSurface/CreateClipper/Blt) and the DDSURFACEDESC/DDBLTFX field offsets are a faithful transcription off the raw disassembly.
// size      545 bytes
// prototype int (__thiscall ?play@UV2Player@@QAEHHH@Z)(UV2Player* this, int, int)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF400 0x005D44FC 0x005DFB50 0x005EFD20 0x006465F0
// notes     Seed provenance (TRIED): the 21-slot COM vtable dispatch
//           (QueryInterface/Release/SetCooperativeLevel/SetDisplayMode
//           retry chain/CreateSurface/GetAttachedSurface/CreateClipper/Blt)
//           and the DDSURFACEDESC/DDBLTFX field offsets are a faithful
//           transcription off the raw disassembly; the prologue's this-spill
//           placement differs (MISMATCH #4 push vs mov). Not measured exact.
Return Value: 0 after surface clearing; 1 for early/attached-surface failure;
              18 for failed display modes or clipper creation.
Status: Coverage transcription - unclaimed body, seed
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004bf4b0.cpp retained.
*/
int UV2Player::play(int width, int height) {
    // Native ESI points to the member slot (+0x10C), not a cached interface.
    // Reload across mode retries and surface callbacks: COM calls may rebind it.
    if (direct_draw2_ != nullptr) {
        return 1;
    }

    ScreenBufferOwner.buffer().fill(0);
    Win::flip(nullptr);

    if (DirectDrawCreate(nullptr, &direct_draw_, nullptr) != DD_OK) {
        return 1;
    }
    if (direct_draw_->SetCooperativeLevel(HandleMain,
            DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE) != DD_OK) {
        return 1;
    }

    // The native code does not branch on QueryInterface's result, or on the
    // Release result. Keep that partial-initialization path unchanged.
    direct_draw_->QueryInterface(IID_IDirectDraw2,
        reinterpret_cast<void **>(&direct_draw2_));
    direct_draw_->Release();

    if (direct_draw2_->SetDisplayMode(width, height, 0x10, 0, 0) != DD_OK &&
        direct_draw2_->SetDisplayMode(width, height, 0x18, 0, 0) != DD_OK &&
        direct_draw2_->SetDisplayMode(width, height, 0x20, 0, 0) != DD_OK &&
        direct_draw2_->SetDisplayMode(width, height, 8, 0, 0) != DD_OK) {
        close();
        return 0x12;
    }

    DDSURFACEDESC description;
    memset(&description, 0, 0x6C);
    description.dwSize = 0x6C;
    description.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    description.dwBackBufferCount = 1;
    description.ddsCaps.dwCaps =
        DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE;

    // These are output slots in the native frame. Do not initialize them or
    // add a CreateSurface failure check absent from the original path.
    LPDIRECTDRAWSURFACE primary_surface;
    direct_draw2_->CreateSurface(&description, &primary_surface, nullptr);

    DDSCAPS attached_caps;
    attached_caps.dwCaps = DDSCAPS_BACKBUFFER;
    LPDIRECTDRAWSURFACE back_buffer_surface;
    if (primary_surface->GetAttachedSurface(&attached_caps,
                                             &back_buffer_surface) != DD_OK) {
        close();
        return 1;
    }

    primary_surface->QueryInterface(IID_IDirectDrawSurface2,
        reinterpret_cast<void **>(&primary_surface_));
    back_buffer_surface->QueryInterface(IID_IDirectDrawSurface2,
        reinterpret_cast<void **>(&back_buffer_surface_));

    if (direct_draw2_->CreateClipper(0, &clipper_, nullptr) != DD_OK) {
        return 0x12;
    }

    DDBLTFX effects;
    effects.dwSize = 0x64;
    effects.dwFillColor = 0;
    back_buffer_surface_->Blt(nullptr, nullptr, nullptr,
                              DDBLT_WAIT | DDBLT_COLORFILL, &effects);
    primary_surface_->Blt(nullptr, nullptr, nullptr,
                          DDBLT_WAIT | DDBLT_COLORFILL, &effects);
    return 0;
}

// ORIGINAL: 0x004BEF00 ?UNK3@UV2Player@@QAEXXZ 0x004BEF00-0x004BF380;0x00659D00-0x00659D55
// RECOVERY_BINDING: v1 {"evidence": ["task:uv2player-complete-owner-recovery", "Pinned4BEF00-4BF380 and659D00-659D55; exec4BED47 calls this only when quit8CC250 is clear", "4BEF42-4BEF71 captures allocation owner before StringList construction; native cleanup restores it after derived/base list teardown", "4BF07B-4BF0AE parses left,top,width,height;4BF1C8-4BF20F centers text vertically with signed division", "4BF11D-4BF147 borrows primary surface and clipper via stack ExtDirectDraw;4BF1F0/4BF21A install/clear cdecl writer callback", "Literal687AC0 is movlist; help pointer691B00 is source-owned DatalinkHelpFile", "historical seed:src/recovered/units/004bef00.cpp"], "reason": "Recover the post-play MOVLIST presentation as a real UV2Player member, using Font array lifetime, genuine StringList and outer allocation-base lifetime, source-owned help-file storage and Buffer callback. Preserve original main and cleanup spans; no matching or game acceptance claim is granted."}
// symbol    ?show_post_movie_text@UV2Player@@QAEXXZ
void UV2Player::show_post_movie_text() {
    Font fonts[3];
    char font_names[3][0x80];
    int font_styles[3];
    int font_heights[3];
    int color_first[3];
    int color_second[3];
    int color_third[3];
    int color_fourth[3];

    StringAllocationBase allocator_scope;
    StringList help_lines;

    if (text_open("movlist", movie_name_) == 0) {
        for (int index = 0; index < 3; ++index) {
            font_names[index][0] = '\0';
            text_get();
            strcat(font_names[index], text_item());

            font_styles[index] = text_item_number();
            font_heights[index] = text_item_number();

            text_get();
            color_first[index] = text_item_number() + 0xA;
            color_second[index] = text_item_number() + 0xA;
            color_third[index] = text_item_number();
            color_fourth[index] = text_item_number();

            // The original pushes style, then height, then the font-name
            // pointer. Font::init takes (name, height, style).
            fonts[index].init(font_names[index], font_heights[index],
                              font_styles[index]);
        }

        text_get();
        const int left = text_item_number();
        const int top = text_item_number();
        const int right = text_item_number() + left;
        const int bottom = text_item_number() + top;

        text_get();
        write_delay_ = text_item_number();
        const int timeout = text_item_number();

        text_get();
        LPSTR help_line = text_get();

        help_lines.remove_all();
        while (help_line != 0 && help_line[0] != '\0') {
            if (help_line[0] == '@') {
                help_lines.load(DatalinkHelpFile, help_line + 1, 0, 0);
            } else {
                help_lines.add_special(help_line);
            }
            help_line = text_get();
        }
        text_close();

        ExtDirectDraw display;
        // DirectDrawSurface2 extends DirectDrawSurface in the SDK; Buffer's
        // borrowed surface API consumes the shared interface prefix.
        display.surface = reinterpret_cast<IDirectDrawSurface *>(primary_surface_);
        display.clipper = clipper_;
        buffer_.init(0x280, 0x1E0, 4, &display);
        buffer_.set_font(&fonts[0], &fonts[1], &fonts[2], &fonts[0]);
        buffer_.set_text_color(color_first[0], color_second[0],
                               color_third[0], color_fourth[0]);
        buffer_.set_text_color2(color_first[1], color_second[1],
                                color_third[1], color_fourth[1]);
        buffer_.set_text_color3(color_first[2], color_second[2],
                                color_third[2], color_fourth[2]);
        buffer_.set_text_color_hyper(color_first[0], color_second[0],
                                     color_third[0], color_fourth[0]);

        const int width = right - left;
        const int drawn_height = buffer_.write_strings_height(&help_lines,
                                                               width, 0);
        const int y = top + (bottom - drawn_height - top) / 2;

        buffer_.set_write_raw_callback(&uv2_player_writer_redirect);
        buffer_.write_strings(&help_lines, left, y, width, 0);
        buffer_.set_write_raw_callback(0);

        const DWORD start = timeGetTime();
        DWORD now = timeGetTime();
        while (now - start < static_cast<DWORD>(timeout)) {
            now = timeGetTime();
        }
    }
}

// ORIGINAL: 0x004BEA50 ?exec@UV2Player@@QAEHPADHHH@Z 0x004BEA50-0x004BEDB5
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x004BEA50-0x004BEDB5", "image:terranx_original.exe:0x00403BE0 caller passes a found .wve path, 640x480, flags 0xC", "image:terranx_original.exe:0x004BF390 is ?on_key@UV2Player@@QAAHXZ", "image:terranx_original.exe:0x005CB220 and 0x005CAAB0 consume MediaDescriptor+0xAC only under flags&0x40000", "image:terranx_original.exe:0x005CB6C0 reads and 0x005CD750 writes MediaDescriptor palette bytes at +0xB0", "source:src/uv2player.h:UV2Player exact 0xE68 layout, media_ and Buffer ownership", "source:src/movie_media.h:MediaDescriptor exact 0x7B4 storage view", "source:src/wave_device.h:WaveDeviceGlobal", "source:src/alphamovie.cpp:amovie_project .wve caller"], "reason": "Recover the .wve controller on UV2Player using its real descriptor, sound device, movie lifecycle and caller, preserving the pinned message/audio/cursor order and early-return behavior."}
// TRIED: direct transcription off Ghidra + raw disasm, `this` used as the char* name buffer at offset 0, indirect calls through the fixed import-table slots (ShowCursor/PeekMessage/GetMessage/ TranslateMessage/DispatchMessage/Sleep/CharUpperA); matches through instr #26, diverges in the strrchr/inc-vs-lea shape
// Proposed source-owner replacement for the PENDING_BODY at 0x004BEA50.
// Drop this definition into src/uv2player.cpp after reviewing the companion
// evidence report and the MediaDescriptor +0xB0 union proposal.

int UV2Player::exec(char *movie_file, int width, int height, int flags) {
    if (movie_file == 0) {
        return 1;
    }

    close();
    UV2PlayerActive = this;
    movie_name_[0] = '\0';
    char *slash = strrchr(movie_file, '\\');
    strcat(movie_name_, slash != 0 ? slash + 1 : movie_file);
    char *extension = strrchr(movie_name_, '.');
    if (extension != 0) {
        *extension = '\0';
    }
    CharUpperA(movie_name_);

    field_104_ = flags;
    if (prefs_get("8BitMovieSound", 0, 0) == 0) {
        WaveDeviceGlobal.suspend();
    } else {
        WaveDeviceGlobal.disable();
    }

    // The native body clears exactly the complete embedded descriptor after
    // suspending/disabling sound and before attempting DirectDraw playback.
    memset(&media_, 0, 0x7B4);
    if (play(width, height) != 0) {
        // The image returns here without restoring sound or undoing the cursor
        // state. Keep that failure-path behavior intact.
        return 1;
    }

    media_.draw_query_interface_0c = direct_draw_;
    media_.draw_device_10 = direct_draw2_;
    media_.surface_14 = primary_surface_;
    media_.surface_18 = back_buffer_surface_;
    media_.flags |= 0x0BA11600;
    media_.capability_options_08 = 5;

    // This address is stored at descriptor+0xB0 (player+0x1D8), not in the
    // distinct C callback slot at +0xAC. The member-pointer view is a union
    // with the stream RGB triplets because the media parser later writes those
    // same bytes. Its 32-bit VC6 representation must be checked by the owner
    // build before this proposal is integrated.
    media_.key_poll_member_b0 = &UV2Player::on_key;

    media_.sound_device = WaveDeviceGlobal.get_ds();
    media_.flags |= 0x00600000;
    media_.source_buffer_58 = 0;
    if ((flags & 0x0001) != 0) media_.flags |= 0x0001;
    if ((flags & 0x0002) != 0) media_.flags |= 0x0002;
    if ((flags & 0x0004) != 0) media_.flags |= 0x0004;
    if ((flags & 0x0008) != 0) media_.flags |= 0x0008;
    if ((flags & 0x0010) != 0) media_.flags |= 0x0010;
    if ((flags & 0x0020) != 0) media_.flags |= 0x0020;
    if ((flags & 0x0040) != 0) media_.flags |= 0x0040;
    if ((flags & 0x8000) != 0) media_.flags |= 0x8000;

    while (ShowCursor(FALSE) >= 0) {
    }

    UV2PlayerQuitRequested = 0;
    media_session_ = sub_5cae80(movie_file, HandleMain, &media_);
    if (media_session_ != 0) {
        MSG message;
        do {
            if (PeekMessageA(&message, 0, 0, 0, 0) != 0) {
                if (GetMessageA(&message, 0, 0, 0) == 0) {
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                } else if (message.message == WM_KEYDOWN) {
                    if (message.wParam == VK_RETURN ||
                        message.wParam == VK_ESCAPE ||
                        message.wParam == VK_SPACE) {
                        UV2PlayerQuitRequested = 1;
                        sub_5cb660(media_session_);
                    }
                } else if (message.message == WM_LBUTTONDOWN ||
                           message.message == WM_RBUTTONDOWN) {
                    UV2PlayerQuitRequested = 1;
                    sub_5cb660(media_session_);
                }
            }
            Sleep(30);
        } while (sub_5cb610(media_session_) != 0);
    }

    if (UV2PlayerQuitRequested == 0) {
        show_post_movie_text();
    }
    close();
    do_all_non_input();
    Win::update_screen(0, 0);
    Win::flip(0);

    while (ShowCursor(TRUE) <= 0) {
    }

    if (prefs_get("8BitMovieSound", 0, 0) == 0) {
        WaveDeviceGlobal.restart();
    } else {
        WaveDeviceGlobal.enable();
    }
    return 0;
}
