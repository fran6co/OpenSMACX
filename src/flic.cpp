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
// The Buffer mangling key this unit carries (2026-09-17, measured both
// ways). VC6 fixes the struct/class key at the first visible tag
// declaration a translation unit sees. A TU-wide `struct Buffer;` forward
// keyed this unit U and made init emit its catalogued
// `?init@Flic@@QAEHPAUBuffer@@PADHHH@Z` name - but the same key
// re-spelled every Buffer-typed back-reference PAU, above all the
// `?copy@Buffer@@QAEHPAU1@HHHHHH@Z` relocations UNK9, init and
// decode_black_trans's blitter calls emit, and no source mapping holds
// that spelling: the image catalogues copy as
// `?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z` and references resolve by exact
// emitted name, so UNK9's protected obligation sat permanently
// unresolvable with every non-reloc byte already matching. The forward is
// reverted: this unit takes the V key from buffer.h's `class Buffer` like
// sprite, texture and console, whose catalogued PAU names each record the
// tree's PAV emission as a `// symbol` fact (sprite.cpp's ?draw is the
// measured precedent). init's fact line is below; the copy relocations
// become PAV1@, the spelling buffer.cpp's own marker records and the
// checker resolves to 0x005DFF00.

#include "stdafx.h"
#include "flic.h"
#include "general.h"  // mem_get, env_open

// Every decoder whose image CALLS the no-argument `Buffer::get_data`
// (0x005E3373 in each call list: decode_frame, UNK1, UNK3 and the three
// _trans decoders reading the embedded buffer_) spells the call through a
// pointer-to-member. buffer.h defines get_data() in-class - implicitly
// inline - and a plain `buffer_.get_data()` folds the whole DDSURFACEDESC/
// Lock sequence into the caller (+96 bytes in decode_frame alone, no
// relocation where the image makes a call). The pointer spelling makes VC6
// devirtualize to exactly the image's bare thiscall (`mov ecx,[esi+5A8h]` /
// `lea ecx,[esi+4]` then `call 0x005E3373`). A `#pragma auto_inline(off)`
// same-file forwarder (probe.cpp's vector_dist_call form) was measured
// first and refused: its own cdecl push/add-esp frame sits between the
// caller and the target, its emitted symbol binds to nothing, and every
// function through it grew past its recorded extent (UNK1 240 bytes
// against 226). The pragma is gone; the pointer spelling serves all six
// sites, measured identical in shape to decode_frame's existing one.
//
// memset and memcpy stay real CRT calls in this file: the image's
// decode_byte_run and decode_byte_run_trans call 0x006465F0/0x00645930
// instead of expanding rep stos/rep movs loops, and stdafx.h's project-wide
// `#pragma function(...)` line deliberately leaves both names to each
// translation unit (textindex.cpp and buffer.cpp do the same locally).
#pragma function(memset, memcpy)

/*
Purpose: Construct the embedded Buffer and Palette (the class's only two
         non-POD members, in declaration order - no explicit statement
         reaches either), then zero every scalar/pointer field the image
         zeroes.
// ORIGINAL: 0x00629D60 ??0Flic@@QAE@XZ 0x00629D60-0x00629DE2;0x00663320-0x00663335 BYTE_EXACT
// size      130 bytes
// prototype void (__thiscall ??0Flic@@QAE@XZ)(Flic* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D7210 0x005FE2A0
Return Value: n/a
Status: Complete
*/
Flic::Flic() {
    field_5AC_ = 0;
    field_5B0_ = 0;
    field_5B4_ = 0;
    file_ = nullptr;
    frame_ = 0;
    buffer2_ = nullptr;
    field_0_ = 0;
    field_5A4_ = 0;
    field_59C_ = 0;
    field_5A0_ = 0;
}

/*
Purpose: Release the open file (if any) and zero the same fields close()
         zeroes.
// ORIGINAL: 0x00629DF0 ?close@Flic@@QAEXXZ 0x00629DF0-0x00629E44 BYTE_EXACT
// size      84 bytes
// prototype void (__thiscall ?close@Flic@@QAEXXZ)(Flic* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598
Return Value: n/a
Status: Complete
*/
void Flic::close() {
    FILE *file = file_;
    field_5AC_ = 0;
    field_5B0_ = 0;
    field_5B4_ = 0;
    frame_ = 0;
    buffer2_ = nullptr;
    field_0_ = 0;
    field_5A4_ = 0;
    field_59C_ = 0;
    field_5A0_ = 0;
    if (file) {
        fclose(file);
        file_ = nullptr;
    }
}

/*
Purpose: Close the file, then tear down the embedded Palette and Buffer in
         reverse declaration order - both automatic, no explicit statement
         but the `close()` call.
// ORIGINAL: 0x004327A0 ??1Flic@@QAE@XZ 0x004327A0-0x004327F8;0x00653420-0x00653443
// size      88 bytes
// prototype void (__thiscall ??1Flic@@QAE@XZ)(Flic* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x00629DF0 0x005D7410 0x005FE2E0
// TRIED: best reached is 13/25 instructions, 0.980 similar (best of 10
//        flag sets, `/c /O2 /Ob0 /Gy /GR- /Oy- /GX`) - one single missing
//        instruction (`tools/listing_diff.py`): the image stores a SECOND
//        EH unwind-state byte (`mov byte ptr [ebp-4], 0`) right after
//        `close()` returns, transitioning from "everything still needs
//        teardown" to "only the two subobjects do". This tree's compiled
//        body never emits that second transition - compiler-generated EH
//        bookkeeping with no corresponding source statement to write, not
//        reachable by changing the body. Left as `close();`, the direct
//        transcription of the one real call in this function.
//        The strict reference check adds a divergence the masked listing
//        diff cannot see (a call is a call once its operand is masked):
//        the Palette teardown. /O2's /Ob1 expands the in-class
//        `~Palette() { close(); }` (palette.h) into the implicit member
//        destruction, so this unit emits `call ?close@Palette@@QAEXXZ`
//        (0x005FE500) where the image calls the out-of-line
//        `??1Palette@@QAE@XZ` thunk at 0x005FE2E0 (5 bytes, `jmp
//        0x5fe500`, 18 catalogued callers); under `/Oy-` every relocation
//        after the close() site mispairs from the shifted body. No
//        spelling in flic.cpp reaches it - the teardown call is
//        compiler-generated from palette.h's declaration. The lever is
//        declaring ~Palette out-of-line with its body beside the palette.cpp
//        marker, the same measured move buffer.h made for ~Buffer ("the
//        image makes `call ??1Buffer` at all 87 of its sites");
//        palette.h/palette.cpp sit outside this task's files, and
//        alphamovie.h and gamma.h embed Palette by value too. Reported for
//        Palette-owner scope.
//        APPLIED 2026-09-16: ~Palette moved out of line into palette.cpp
//        after the task's scope grew to the Palette files; see the LEVER on
//        the ??1Palette marker there.
//        ROOT CAUSE 2026-09-18 (round 3): the missing second state store
//        and the one-arm funclet reduce to one compiler fact, measured in
//        build/recovery/probes/eh-palette-dllexport (ehprobe2/ehprobe3,
//        /O2 /Gy /GR- /GX /Z7). VC6 does not need to inline close() to use
//        its body: with close()'s definition ABOVE this destructor, the
//        already-parsed body (member stores plus one extern-C `fclose`,
//        which /GX assumes cannot throw) lets the EH pass prove the whole
//        prefix no-throw, so VC6 collapses the "both members still need
//        teardown" entry state (state 0 = buffer_ only at entry, no
//        transition before `call ??1Palette`, 21-byte one-arm funclet).
//        The image enters at state 1 and steps 1 -> 0 -> -1, funclet
//        carrying both member arms (0x00653420, 35 bytes) - the shape the
//        original compile only produces when close()'s body is NOT yet
//        parsed at the destructor's call site (probe W_below), i.e. the
//        original flic.cpp spelled the call before the definition. The
//        declaration spelling of ~Palette is exonerated: a dllexport'd
//        out-of-line member destructor tracks identically to a plain one
//        (probe W_plain/W_dl).
//        APPLIED 2026-09-18: the close() call goes through a
//        pointer-to-member - the same opaque-call idiom this file uses for
//        Buffer::get_data and Buffer::copy. VC6 still devirtualizes it to
//        the direct `call ?close@Flic@@QAEXXZ` (no extra instruction) but
//        no longer consults the body's throw profile: entry state 1,
//        `mov byte ptr [ebp-4], 0` before `call ??1Palette`, two-arm
//        funclet. Scored in deck build/variants-dtor2 (dtor-close-pmf,
//        both campaign flag sets): under /Oy- both spans carry ZERO
//        differing bytes and the boundary passes - only the fs:0
//        __except_list relocations stay unknown, the same residue every
//        SEH frame in this task (the passing ??0Flic included) carries.
//        The pointer-to-member LITERAL spelling `(this->*&Flic::close)()`
//        folds back to the body-profiled direct call (fail, baseline
//        bytes); `#pragma auto_inline(off)` around close()'s definition
//        likewise changes nothing (probe W_qual/W_autoinline). The
//        typedef'd pointer form measures identical (dtor-close-pmf-
//        typedef); the plain local form keeps this file's established
//        spelling.
Return Value: n/a
Status: Complete
*/
Flic::~Flic() {
    // Through a pointer-to-member for a reason the file's other
    // pointer-to-member sites do not share: not inlining (VC6 does not
    // inline close() into this body either way) but EH bookkeeping. See
    // ROOT CAUSE 2026-09-18 above - a plain `close();` here lets VC6 read
    // close()'s already-parsed body, prove the call no-throw, and drop
    // palette_'s unwind state; the pointer spelling keeps the call opaque
    // and emits the image's full unwind map.
    void (Flic::*close_method)() = &Flic::close;
    (this->*close_method)();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062A710 ?UNK4@Flic@@QAEXXZ 0x0062A710-0x0062A711 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062AAF0 ?UNK5@Flic@@QAEXXZ 0x0062AAF0-0x0062AAF1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK5() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062AB00 ?UNK6@Flic@@QAEXXZ 0x0062AB00-0x0062AB01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062ACC0 ?UNK7@Flic@@QAEXXZ 0x0062ACC0-0x0062ACC1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK7() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062B5F0 ?UNK8@Flic@@QAEXXZ 0x0062B5F0-0x0062B5F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK8@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK8() {
}

/*
Purpose: Open a FLIC file and prepare the first frame: zero the per-file
         state, read and validate the 128-byte header, cache both ring
         frame headers, prime the embedded scratch buffer with the first
         frame and blit it to the destination at (x, y).
// ORIGINAL: 0x00629E50 ?init@Flic@@QAEHPAUBuffer@@PADHHH@Z 0x00629E50-0x0062A068
// symbol    ?init@Flic@@QAEHPAVBuffer@@PADHHH@Z
// size      536 bytes
// prototype int (__thiscall ?init@Flic@@QAEHPAUBuffer@@PADHHH@Z)(Flic* this, Buffer*, char*, int, int, int)
// callers   8   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7670 0x005DE8F0 0x005DFF00 0x005FE330 0x00634BB0 0x00645598 0x00646178 0x00647330
Return Value: 0 ready; 6 when the file cannot be opened; 0x10 without a
              file name; 0x18 when the magic is not 0xAF12
Status: Complete
*/
int Flic::init(Buffer *buffer, char *file_name, int x, int y,
               int transparent) {
    FILE *file = file_;
    field_5AC_ = 0;
    field_5B0_ = 0;
    field_5B4_ = 0;
    frame_ = 0;
    buffer2_ = nullptr;
    field_0_ = 0;
    field_5A4_ = 0;
    field_59C_ = 0;
    field_5A0_ = 0;
    if (file) {
        fclose(file);
        file_ = nullptr;
    }

    if (file_name == nullptr) {
        return 0x10;
    }

    palette_.init();

    field_5AC_ = x;
    field_5B0_ = y;
    buffer2_ = buffer;

    file_ = env_open(file_name, "rb");
    if (file_ == nullptr) {
        return 6;
    }

    fread(&header_, 0x80, 1, file_);
    if (header_.magic_ != 0xaf12) {
        fclose(file_);
        file_ = nullptr;
        return 0x18;
    }

    field_59C_ = header_.width_;
    field_5A0_ = header_.height_;
    field_5B4_ |= 1;

    fseek(file_, header_.first_frame_, SEEK_SET);
    fread(&first_frame_, 0x10, 1, file_);
    fseek(file_, header_.second_frame_, SEEK_SET);
    fread(&second_frame_, 0x10, 1, file_);

    chunk_ = first_frame_;

    fseek(file_, header_.first_frame_, SEEK_SET);

    buffer_.init(field_59C_, field_5A0_, 0, nullptr);
    buffer_.sync_to_palette(PaletteCurrent);

    // Through a pointer-to-member for the same measured reason as the
    // `get_data` sites: copy's body is in-class in buffer.h now, and a
    // plain call let VC6 fold the whole blitter into init (1344 bytes
    // against the image's 536) where the image makes the call.
    int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
        &Buffer::copy;
    (buffer->*copy)(&buffer_, field_5AC_, field_5B0_, 0, 0, field_59C_,
                    field_5A0_);

    if (transparent) {
        field_5B4_ |= 2;
    }
    return 0;
}

/*
Purpose: Decode the next frame of the open animation onto the destination
         buffer at (x, y), wrapping to the loop point once the final frame
         has played.
// ORIGINAL: 0x0062A070 ?decode_frame@Flic@@QAEHHH@Z 0x0062A070-0x0062A11D FILE BYTE_EXACT
// LEVER: fseek needs a real signature -> declared as fseek_impl (own name), since the scaffold's nullary fseek can't be redeclared (C2733)
// size      173 bytes
// prototype int (__thiscall ?decode_frame@Flic@@QAEHHH@Z)(Flic* this, int, int)
// callers   11   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x0062ACD0 0x00646178 0x00647330
Return Value: 0 frame decoded; 7 when no file is open; 0x10 when the
              destination buffer has no pixel bits
Status: Complete
*/
int Flic::decode_frame(int x, int y) {
    if (!(field_5B4_ & 1)) {
        return 7;
    }
    void *(Buffer::*get_data)() = &Buffer::get_data;
    if ((buffer2_->*get_data)() == 0) {
        return 0x10;
    }
    field_5AC_ = x;
    field_5B0_ = y;
    if (frame_ == header_.frames_) {
        fseek(file_, header_.second_frame_, SEEK_SET);
        frame_ = 0;
    }
    fread(&chunk_, 0x10, 1, file_);
    decode_chunk(&chunk_);
    frame_++;
    return 0;
}

/*
Purpose: Decode the next frame in transparent mode - identical to
         decode_frame except the transparent bit is set for the duration
         of the decode and cleared again on every exit.
// ORIGINAL: 0x0062A120 ?UNK1@Flic@@QAEHHH@Z 0x0062A120-0x0062A202
// size      226 bytes
// prototype int (__thiscall ?UNK1@Flic@@QAEHHH@Z)(Flic* this, int, int)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x0062ACD0 0x00646178 0x00647330
Return Value: 0 frame decoded; 7 when no file is open; 0x10 when the
              destination buffer has no pixel bits
Status: Complete
*/
int Flic::UNK1(int x, int y) {
    // Plain member read-modify-writes match the image's entry exactly
    // (0x0062A120): the cached `or edx,2` stays a dword op, the bit-1 test
    // goes through a copy (`mov eax,edx; test al,1`), and the
    // pointer-to-member get_data call invalidates the cache so both later
    // clears reload the member (`mov ecx,[esi+0x5b4]`) the way the image
    // does. A `flags` local round trip instead keeps the whole
    // computation in the byte domain (`or al,2` / `and al,0xfd`) from the
    // first instruction. The one feature left: every clear temp lands in
    // eax, where VC6 shortens `& ~2` to the 2-byte AL special form
    // (`and al,0xfd`), while the image's temps sit in ecx - where the
    // 3-byte `and ecx,0xfffffffd` sign-extended form is the same length
    // as the byte op and VC6 keeps the dword encoding. No spelling
    // measured (member, local round trip, result-local, explicit
    // self-read, full-width 0xfffffffd masks) steers those three temps
    // off eax; the retval constant sinks past the clear instead of
    // occupying eax first, which is what the image's interleaved
    // `mov eax,7` between copy and and would need.
    field_5B4_ |= 2;
    if (!(field_5B4_ & 1)) {
        field_5B4_ &= ~2;
        return 7;
    }
    void *(Buffer::*get_data)() = &Buffer::get_data;
    if ((buffer2_->*get_data)() == 0) {
        field_5B4_ &= ~2;
        return 0x10;
    }
    field_5AC_ = x;
    field_5B0_ = y;
    if (frame_ == header_.frames_) {
        fseek(file_, header_.second_frame_, SEEK_SET);
        frame_ = 0;
    }
    fread(&chunk_, 0x10, 1, file_);
    decode_chunk(&chunk_);
    frame_++;
    field_5B4_ &= ~2;
    return 0;
}

/*
Purpose: Step over the next frame without decoding it - seek past the
         current frame chunk, or jump back to the loop point after the
         final frame. The wrap branch deliberately leaves the frame
         counter alone: the original returns without resetting it.
// ORIGINAL: 0x0062A210 ?UNK2@Flic@@QAEHXZ 0x0062A210-0x0062A27B FILE BYTE_EXACT
// LEVER: fseek scaffolds nullary (not in declfix CRT_SIGNATURES); take &fseek and cast to the real function-pointer type instead of redeclaring
// size      107 bytes
// prototype int (__thiscall ?UNK2@Flic@@QAEHXZ)(Flic* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00647330
Return Value: 0 frame skipped; 7 when no file is open
Status: Complete
*/
int Flic::UNK2() {
    if (!(field_5B4_ & 1)) {
        return 7;
    }
    if (frame_ == header_.frames_) {
        fseek(file_, header_.second_frame_, SEEK_SET);
        return 0;
    }
    fseek(file_, chunk_.size_, SEEK_CUR);
    frame_++;
    return 0;
}

/*
Purpose: Seek back to the first frame of the animation and restart the
         frame counter.
// ORIGINAL: 0x0062A280 ?rewind@Flic@@QAEHXZ 0x0062A280-0x0062A2B9 FILE BYTE_EXACT
// size      57 bytes
// prototype int (__thiscall ?rewind@Flic@@QAEHXZ)(Flic* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00647330
Return Value: 0 rewound; 7 when no file is open
Status: Complete
*/
int Flic::rewind() {
    if (!(field_5B4_ & 1)) {
        return 7;
    }
    fseek(file_, header_.first_frame_, SEEK_SET);
    frame_ = 0;
    return 0;
}

/*
Purpose: Advance the animation to an arbitrary frame. When the target is
         behind the current position the file is rewound first; the file
         is then scanned frame by frame, remembering the most recent
         frame whose subchunks are all still decodable (types other than
         7 and 0xC), and finally the remembered position is re-seeked and
         frames are decoded forward until the target is reached.
// ORIGINAL: 0x0062A2C0 ?UNK3@Flic@@QAEHH@Z 0x0062A2C0-0x0062A4FA
// size      570 bytes
// prototype int (__thiscall ?UNK3@Flic@@QAEHH@Z)(Flic* this, int)
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x0062ACD0 0x00646178 0x00647330 0x00647920
// TRIED: the two image-motivated spellings for this body measured
//        2026-09-17 (deck unk3-pmf-goto, four candidates, both flag
//        sets): declaring the pointer-to-member at the call site inside
//        the flag guard instead of at the function top, and replacing
//        the `decodable` flag with a flagless `goto` past the pending
//        update - the image's own shapes, its frame being exactly 0x18
//        with no pointer slot and its scan jumping `je` straight past
//        the update (0x0062A3A2/0x0062A3A8 -> 0x0062A3C9). Both change
//        nothing: every spelling emits one identical 576-byte section,
//        because VC6 already folds this flag spelling into those two
//        `je`s and devirtualizes the member pointer without a stack
//        slot wherever it is declared. The 411 differing /O2 bytes are
//        register and slot allocation: the image holds &chunk_ in ebx
//        across both loops, spills frame_pos to its own slot (re-seeked
//        at 0x0062A4D8) and homes target in the recycled argument slot
//        while i takes a local; this tree rematerializes &chunk_ in edi
//        at each loop head, keeps frame_pos live in ebx, homes target
//        in a local and recycles the argument slot for i - so the
//        forward loop pins target in ebp (x in ebx) where the image
//        reloads target from the argument slot every iteration (x in
//        ebp), and the skip-scan exit re-seeks live ebx where the image
//        re-seeks the spill. Same compiler-side allocation class fill()
//        and decode_byte_run_trans document; under /Oy- every reference
//        still binds but the 560-byte body no longer matches the
//        recorded 570-byte extent, so /O2 is the only operative trial.
Return Value: 0
Status: Complete
*/
int Flic::UNK3(int target_frame) {
    void *(Buffer::*get_data)() = &Buffer::get_data;
    int target = target_frame % header_.frames_;

    if (target < static_cast<int>(frame_) && (field_5B4_ & 1)) {
        fseek(file_, header_.first_frame_, SEEK_SET);
        frame_ = 0;
    }

    int pending_frame = frame_;
    long pending_pos = ftell(file_);
    // frame_pos rides across the whole scan: every iteration reassigns it
    // at the head, a frame with no decodable subchunk copies it into
    // pending_pos, and the already-past-target exit re-seeks to the last
    // value it holds (0x0062A2C0). The original never writes its slot
    // before the scan - the spill at 0x0062A35F is the first store - so
    // that exit seeks whatever the stack held; initializing it from
    // pending_pos keeps the path defined without touching the scanned
    // cases.
    long frame_pos;
    // The scan head, the forward loop's fread and its decode_chunk call
    // all take &chunk_: the image materializes that address once above
    // the scan guard (lea at 0x0062A379) and reuses the register.
    DataChunk *chunks = &chunk_;

    if (static_cast<int>(frame_) <= target) {
        do {
            frame_pos = ftell(file_);
            fread(chunks, 0x10, 1, file_);
            long next_pos = chunk_.size_ + frame_pos;
            int subchunks = chunk_.count_;

            int decodable = 0;
            for (int i = 0; i < subchunks; i++) {
                DataChunk sub;
                fread(&sub, 6, 1, file_);
                if (sub.type_ == 7 || sub.type_ == 0xc) {
                    decodable = 1;
                    break;
                }
            }
            if (!decodable) {
                pending_frame = frame_;
                pending_pos = frame_pos;
            }

            fseek(file_, next_pos, SEEK_SET);
            frame_++;
        } while (static_cast<int>(frame_) <= target);
    }

    if (static_cast<int>(pending_frame) >= static_cast<int>(frame_)) {
        fseek(file_, frame_pos, SEEK_SET);
        return 0;
    }

    frame_ = pending_frame;
    fseek(file_, pending_pos, SEEK_SET);

    if (static_cast<int>(frame_) < target) {
        do {
            // x/y captured before the guards and stored back once both
            // pass: the call may mutate *this, so the image carries the
            // members through registers around it (0x0062A432-0x0062A474).
            int y = field_5B0_;
            int x = field_5AC_;
            if (field_5B4_ & 1) {
                if ((buffer2_->*get_data)() != 0) {
                    field_5AC_ = x;
                    field_5B0_ = y;
                    if (frame_ == header_.frames_) {
                        fseek(file_, header_.second_frame_, SEEK_SET);
                        frame_ = 0;
                    }
                    fread(chunks, 0x10, 1, file_);
                    decode_chunk(chunks);
                    frame_++;
                }
            }
        } while (static_cast<int>(frame_) < target);
    }

    return 0;
}

/*
Purpose: Decode a type-0xF (FLI BYTE RUN) chunk: per row, a packet count
         then packets - a positive control byte repeats the following
         value byte, a negative one copies that many literal bytes from
         the file. When a colour offset is set the literal path is
         staged through a scratch row so the offset can be added per
         byte.
// ORIGINAL: 0x0062A500 ?decode_byte_run@Flic@@QAEXXZ 0x0062A500-0x0062A70F
// size      527 bytes
// prototype void (__thiscall ?decode_byte_run@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E33F3 0x00644EF2 0x00644F3A 0x00646178 0x006465F0
// TRIED: extent holds under /O2 but the body carries one extra
//        instruction (158 against 157, 40 masked bytes, similarity
//        0.952), decomposed exactly against the image listing
//        (2026-09-17): the image homes `value` in its stack slot and
//        folds the colour-offset add into ONE memory-destination byte op
//        (`add BYTE PTR [esp+0x14],al`, the field_5A4_ load's low byte
//        as addend) where the run path here routes through registers
//        (`mov cl,al` / `mov al,[slot]` / `add al,cl` / `mov [slot],al`,
//        +3), and in exchange both literal-path run lengths materialize
//        as `movsx` from al (-1 each) where the image reloads the count
//        dword and masks (`and 0xff`). The dword-slot family - unsigned
//        count/value with `& 0xff` at every length use, the shape
//        decode_byte_run_trans proves for its own body - reproduces the
//        image's reload+and forms and shortens the add to
//        `add al,[slot]` + `and eax,0xff` + `mov [slot],eax`, still not
//        the one-op RMW, and the count reloads grow the body to 544
//        bytes against the recorded 527 - extent fail (four spellings,
//        deck build/variants-dbr1). The byte-typed value family -
//        assignment, compound `+=`, hoisted declarations (deck
//        build/variants-dbr2; the committed spelling is the assignment
//        form, which reproduces the image's dword-reload+and VALUE uses
//        at memset) - holds the extent but leaves the add unfolded and
//        the lengths as movsx: 158 again. Seven deck spellings plus both
//        committed spellings, both campaign flag sets; nothing emits the
//        memory-destination add, and the rect block carries the hoisted
//        add placement fill() documents for the family (count-neutral).
//        The residual is VC6 register/stack allocation, the same class
//        fill(), decode_byte_run_trans and UNK3 record.
Return Value: n/a
Status: Complete
*/
void Flic::decode_byte_run() {
    void *scratch = nullptr;
    uint8_t *dst = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));
    if (field_5A4_ != 0) {
        scratch = mem_get(buffer2_->stride_);
        if (scratch == nullptr) {
            return;
        }
    }

    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;

    for (int row = 0; row < static_cast<int>(field_5A0_); row++) {
        uint8_t *row_dst = dst;
        int remaining = field_59C_;
        int8_t count;
        fread(&count, 1, 1, file_);
        while (remaining > 0) {
            fread(&count, 1, 1, file_);
            if (count > 0) {
                uint8_t value;
                fread(&value, 1, 1, file_);
                if (field_5A4_ != 0) {
                    value = static_cast<uint8_t>(
                        value + static_cast<uint8_t>(field_5A4_));
                }
                memset(row_dst, value, static_cast<uint8_t>(count));
            } else {
                count = static_cast<int8_t>(abs(count));
                if (field_5A4_ == 0) {
                    fread(row_dst, 1, count, file_);
                } else {
                    fread(scratch, 1, count, file_);
                    uint8_t *src = static_cast<uint8_t *>(scratch);
                    for (int i = 0; i < static_cast<uint8_t>(count); i++) {
                        row_dst[i] = static_cast<uint8_t>(src[i] + field_5A4_);
                    }
                }
            }
            row_dst += static_cast<uint8_t>(count);
            remaining -= static_cast<uint8_t>(count);
        }
        dst += buffer2_->stride_;
    }

    if (scratch != nullptr) {
        free(scratch);
    }
}

/*
Purpose: Decode a type-7 (FLC DELTA) chunk: word-run deltas. Per line a
         signed line count, optional 0xC000 skip words, an optional
         last-pixel opcode (0x8000), then packets - a skip byte followed
         by a run byte; a negative run repeats the following word, a
         positive one copies that many words, staged through a scratch
         row when a colour offset is set. The dirty rectangle tracks the
         first and last line touched and the least and greatest column.
// ORIGINAL: 0x0062A720 ?decode_delta_flc@Flic@@QAEXXZ 0x0062A720-0x0062AAE7
// size      967 bytes
// prototype void (__thiscall ?decode_delta_flc@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E33F3 0x00644EF2 0x00644F3A 0x00646178
// TRIED: the +41 extent gap is exactly one duplicated fread call site.
//        The image reads the line header at ONE site (0x0062A7C7) with
//        BOTH loop back edges entering it - the skip path's unconditional
//        `jmp 0x0062A7B7` (0x0062A804) and the line tail's
//        `jl 0x0062A7B7` (0x0062AA8C) - eight fread sites in the whole
//        function (+109 +167 +396 +443 +502 +553 +599 +702), row and
//        line_index memory-homed at the tail (`mov [esp+0x2c],ebp`,
//        `mov [esp+0x1c],eax`). This tree spells the read twice (line
//        head + skip re-read) and VC6 emits both - nine sites, 1008
//        bytes under /O2, 960 under /Oy-. Measured 2026-09-17 across
//        seven spellings and both campaign flag sets (decks
//        flic-delta-loopmerge and flic-delta-continue, plus local
//        compiles): the nested-cycle family - current while form,
//        for(;;)+break, goto-to-head, comma-condition while - all
//        compile BYTE-IDENTICAL at 1008/960 (VC6 canonicalizes every
//        spelling of the cycle into the rotated two-site loop, the
//        cloned read taking a second stack home); a Buffer local for
//        buffer2_ reaches 992/944; the continue-merge family (skip as
//        `continue` in the line loop, plain and with the Buffer local)
//        does collapse to a single read site but at 928/912-944 with a
//        divergent register allocation (kdb 1633 against the baseline's
//        864). No spelling reaches the unrotated single-site loop; the
//        residual is VC6-side loop rotation, the same compiler-side
//        class fill() and decode_byte_run_trans document. The current
//        while spelling stays as the closest measured form.
Return Value: n/a
Status: Complete
*/
void Flic::decode_delta_flc() {
    uint8_t run_count;
    uint16_t line_hdr;
    void *work_buf = nullptr;
    int line_index;
    uint32_t max_touched;
    uint16_t word;
    int packet_index;
    uint8_t *row;
    uint16_t num_lines;
    uint32_t min_touched;
    int last_row;
    int first_row;
    uint8_t skip_count;

    if (field_5A4_ != 0) {
        work_buf = mem_get(buffer2_->stride_);
        if (work_buf == nullptr) {
            return;
        }
    }
    row = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));
    fread(&num_lines, 2, 1, file_);
    min_touched = 0xffffffff;
    first_row = -1;
    max_touched = 0;
    last_row = 0;
    line_index = 0;
    if (num_lines > 0) {
        do {
            fread(&line_hdr, 2, 1, file_);
            while ((line_hdr & 0xc000) == 0xc000) {
                int skip_lines = abs(static_cast<int16_t>(line_hdr));
                row += buffer2_->stride_ * skip_lines;
                fread(&line_hdr, 2, 1, file_);
            }
            if (line_index == 0) {
                uint8_t *base = reinterpret_cast<uint8_t *>(
                    buffer2_->get_data(field_5AC_, field_5B0_));
                first_row = static_cast<int>(row - base) /
                            static_cast<int>(buffer2_->stride_);
            }
            if (line_index == static_cast<int>(num_lines) - 1) {
                uint8_t *base = reinterpret_cast<uint8_t *>(
                    buffer2_->get_data(field_5AC_, field_5B0_));
                last_row = static_cast<int>(row - base) /
                           static_cast<int>(buffer2_->stride_);
            }
            if (line_hdr & 0x8000) {
                max_touched = field_59C_;
                row[max_touched - 1] =
                    static_cast<uint8_t>(static_cast<int8_t>(line_hdr) +
                                         static_cast<int8_t>(field_5A4_));
                fread(&line_hdr, 2, 1, file_);
            }
            packet_index = 0;
            uint8_t *dst = row;
            if (line_hdr > 0) {
                do {
                    fread(&skip_count, 1, 1, file_);
                    dst += skip_count;
                    if (packet_index == 0 &&
                        static_cast<uint32_t>(dst - row) < min_touched) {
                        min_touched = dst - row;
                    }
                    fread(&run_count, 1, 1, file_);
                    if (static_cast<int8_t>(run_count) > 0) {
                        if (field_5A4_ == 0) {
                            fread(dst, 2, run_count, file_);
                            dst += run_count * 2;
                        } else {
                            fread(work_buf, 2, run_count, file_);
                            uint8_t *src = static_cast<uint8_t *>(work_buf);
                            for (int i = 0; i < run_count * 2; i++) {
                                dst[i] = static_cast<uint8_t>(
                                    src[i] + field_5A4_);
                            }
                            dst += run_count * 2;
                        }
                    } else if (static_cast<int8_t>(run_count) < 0) {
                        fread(&word, 2, 1, file_);
                        run_count = static_cast<uint8_t>(
                            abs(static_cast<int8_t>(run_count)));
                        word = static_cast<uint16_t>(
                            (static_cast<uint8_t>(
                                 static_cast<uint8_t>(word >> 8) +
                                 field_5A4_)
                             << 8) +
                            static_cast<uint8_t>(word + field_5A4_));
                        for (int i = 0; i < run_count; i++) {
                            *reinterpret_cast<uint16_t *>(dst) = word;
                            dst += 2;
                        }
                    }
                    packet_index++;
                } while (packet_index < line_hdr);
            }
            if (max_touched < static_cast<uint32_t>(dst - row)) {
                max_touched = dst - row;
            }
            row += buffer2_->stride_;
            line_index++;
        } while (line_index < static_cast<int>(num_lines));
    }
    if (work_buf != nullptr) {
        free(work_buf);
    }
    update_rect_.left = field_5AC_ + min_touched;
    update_rect_.top = field_5B0_ + first_row;
    update_rect_.right = field_5AC_ + 1 + max_touched;
    update_rect_.bottom = field_5B0_ + 1 + last_row;
}

/*
Purpose: Fill the rectangle the animation occupies on the destination
         buffer with color zero and mark it dirty.
// ORIGINAL: 0x0062AB10 ?fill@Flic@@QAEXXZ 0x0062AB10-0x0062AB72
// size      98 bytes
// prototype void (__thiscall ?fill@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240
Return Value: n/a
Status: Complete
*/
void Flic::fill() {
    // Stores go through a `uint32_t *` to the four dirty-rectangle
    // members: VC6's optimizer caches member loads across direct
    // same-base stores but invalidates that cache for stores through a
    // pointer local, which is what makes the image reload every operand
    // for the call (0x0062AB10) - this spelling alone moves the function
    // 80 -> 98 bytes, the recorded size. The one feature no spelling
    // reaches is the image's add placement: it interleaves the two sums
    // between the store pairs in statement order (`add edx,edi`,
    // `add eax,esi` after both `mov [ecx+0x58c/0x590]`), while every
    // plain-sum spelling - including `x += width`, `width + x` and named
    // sum locals - hoists both adds above the stores, and every read-back
    // spelling (`rect->right = rect->left + width`) anchors its adds after the
    // FIRST store only: VC6 store-forwards rect->left back to the x register,
    // both adds become free-floating register ops again and the scheduler
    // lifts them above the rect1 store, and whichever local rect->left
    // stores loads last instead of second. Declaration-order permutations
    // and swapping the sum operands do not restore the load order.
    // CORRECTION 2026-09-18 (deck dchunk-arm-readback, five arm
    // spellings, both flag sets): the same block inline inside
    // decode_chunk's two 0xD cases does NOT interleave either - plain
    // sums hoist the same 18-byte cluster in both arms, and the
    // read-back spelling that pins both adds between the store pairs
    // flips the loads x-first and leaves the add destinations wrong
    // (19 differing bytes per arm against plain sums' 18; the
    // operand-swap and x-sum-only read-back spellings measure
    // identical to it, and a read-back y-sum alone compiles
    // byte-identical to plain sums). Placement and load order trade
    // against each other in every context measured; the round-1
    // "context-dependent scheduling" reading (arms interleave, esi
    // holds `this`, no pending pushes) was placement-only and is
    // disproven by bytes.
    // Nineteen spellings measured under both flag sets (decks
    // fill-rect-v1/v2/v3 and local compiles); plain sums stay as the
    // idiomatic form that matches the image's loads, registers, stores
    // and reloads. Extended 2026-09-17 with fifteen disassembled local
    // /O2 compiles: two further anchor mechanisms exist, neither
    // combines the image's prologue loads with its x/y destinations. A
    // fresh member read inside the sum (`rect->right = x + field_59C_`)
    // keeps its load at the sum and anchors the add after both stores -
    // the y-sum's destination lands on eax, the image's register - but
    // the x-sum still accumulates into the fresh width register and
    // neither load reaches the prologue (96 bytes). A read-back left
    // operand (`rect->right = rect->left + width`) steers the FIRST add's
    // destination onto x's register through store forwarding, but the
    // second always lands on height's, the loads reorder to
    // y,height,width,x, and every operand-swap spelling of it emits one
    // identical body (112 bytes). Direct-member sum stores after the
    // pointer stores (`update_rect_.right = x + width`) keep both plain stores
    // first but sink both sum stores below the call's reload sequence.
    // All-member spelling (no locals) disables the load cache
    // entirely - every operand reloads at every use. Thirty-four
    // spellings in total; the destination pair (edx,eax) with four
    // prologue loads is VC6-internal allocation no spelling reaches.
    // Closed 2026-09-17 (round 2, three fresh /O2 compiles): the
    // statement-order form - destructive updates interleaved between
    // the store pairs (`rect->top=y; x+=width; y+=height; rect->right=x;`)
    // - compiles BYTE-IDENTICAL to the plain-sum baseline, compound
    // and assignment spellings both: x and y are dead after their
    // sums, so VC6 canonicalizes the interleaving away and nothing
    // anchors the adds. The read-back spelling disassembles to its
    // own 98-byte shape: loads permuted y,height,width,x, both adds
    // pinned after the FIRST store only (between the rect0 and rect1
    // stores), x-sum store-forwarded onto x's register, y-sum onto
    // height's - the image pins both after the SECOND store onto
    // x/y's registers. Statement order cannot express the image's
    // placement; the anchor question is closed. The same residual
    // add placement carries to
    // decode_black_trans, decode_fli_copy and decode_fli_copy_trans,
    // which share this block.
    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;
    buffer2_->fill(field_5AC_, field_5B0_, field_59C_, field_5A0_, 0);
}

/*
Purpose: Decode a type-0x10 (FLI COPY) chunk: the rows follow verbatim in
         the file. When a colour offset is set each row is staged through
         a scratch row so the offset can be added per byte.
// ORIGINAL: 0x0062AB80 ?decode_fli_copy@Flic@@QAEXXZ 0x0062AB80-0x0062ACB6
// size      310 bytes
// prototype void (__thiscall ?decode_fli_copy@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E33F3 0x00644EF2 0x00646178
// TRIED: plain sums here (`rect->right = x + width`) are the image's
//        spelling - loads y-first into eax/ecx, adds pinned between the
//        store pairs, 310 code bytes + NOP pad, extent boundary passes -
//        while the read-back spelling flips the loads x-first. The
//        residual bytes are the family-wide add-placement plateau
//        fill() documents plus size-neutral register permutations in
//        the per-row fread setup (image loads stride/width through
//        ecx/edx, every spelling measured here picks eax).
Return Value: n/a
Status: Complete
*/
void Flic::decode_fli_copy() {
    void *scratch = nullptr;
    if (field_5A4_ != 0) {
        scratch = mem_get(buffer2_->stride_);
        if (scratch == nullptr) {
            return;
        }
    }

    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;

    uint8_t *dst = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));

    for (int row = 0; row < static_cast<int>(field_5A0_); row++) {
        if (field_5A4_ == 0) {
            fread(dst, 1, field_59C_, file_);
        } else {
            fread(scratch, 1, field_59C_, file_);
            uint8_t *src = static_cast<uint8_t *>(scratch);
            for (int i = 0; i < static_cast<int>(field_59C_); i++) {
                dst[i] = static_cast<uint8_t>(src[i] + field_5A4_);
            }
        }
        dst += buffer2_->stride_;
    }

    if (scratch != nullptr) {
        free(scratch);
    }
}

/*
Purpose: Dispatch one chunk: a type-0xF1FA frame ring recurses over its
         subchunks; anything else is decoded by type (7 delta, 0xD black,
         0xF byte run, 0x10 copy, unknown types are skipped over), the
         transparent variants when the transparent bit is set. Afterwards
         the file position is corrected by whatever the decoder did not
         consume.
// ORIGINAL: 0x0062ACD0 ?decode_chunk@Flic@@QAEXPAUDataChunk@@@Z 0x0062ACD0-0x0062AF40
// size      624 bytes  (extent corrected 2026-09-17: 541 code-to-ret bytes plus the
//             two switch jump-table pairs the original image carries at 0x0062AEED-
//             0x0062AF38 and VC6 emits inside this COMDAT under both campaign flag
//             sets; 80 table bytes and 3 pad. The dispatch shape, the wide [4,0x12]
//             six-slot table and the index table [0,5,5,1,5,5,5,0,0,2,5,3,4,5,0]
//             reproduce byte-for-byte; the code-to-ret extent alone was the record's
//             incomplete unit, as with the constructor's funclet span.)
// prototype void (__thiscall ?decode_chunk@Flic@@QAEXPAUDataChunk@@@Z)(Flic* this, DataChunk*)
// callers   4   call targets   13
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8240 0x005DFF00 0x0062A500 0x0062A720 0x0062AB80 0x0062ACD0 0x0062AF40 0x0062B190 0x0062B670 0x00644F3A 0x00646178 0x00647330 0x00647920
Return Value: n/a
Status: Complete
*/
void Flic::decode_chunk(DataChunk *chunk) {
    // The frame ring recurses over 6-byte subchunk headers; the local
    // struct spells exactly those six bytes (an 8-byte stack object) and
    // the two guards - `== 0` then `<= 0` - lower to the image's
    // `test/je` + `jle` pair at the ring head (0x0062ACD0). Splitting
    // the ring counter into a guard variable plus a `remaining` copy
    // reproduces the image's `mov ax,[edi+6]` guard on the caller-saved
    // side and `mov edi,eax` counter handoff, which is what tips the
    // whole register allocation to chunk->edi / size->ebx with both
    // `push ebx`/`push ebp` sunk under their branches.
    // The end/size pair below is deliberately TWO separate member
    // reads: any spelling that lets both `size - 6` uses share one read
    // (a `size` local first, destructively subtracting, or member
    // spelling at the fseek) extracts and hoists `lea ebp,[eax-6]`
    // above the ftell call, where the image keeps raw size in ebx and
    // re-derives `add ebx,-6` inside the skip arm. Reading the member
    // inside `end` and again into the local lets the allocator merge
    // the loads (one `mov ebx,[edi]`) without giving CSE a shared
    // subexpression to hoist.
    // The two switches spell each skip cluster as `goto` INTO a
    // labeled default arm. That is the only shape measured that keeps
    // the image's WIDE dispatch span [4,0x12] with SIX handler slots -
    // slot0 = slot5 = the shared skip body, zero stubs - and the index
    // table [0,5,5,1,5,5,5,0,0,2,5,3,4,5,0] byte-for-byte; every
    // break/duplicate-default spelling collapses to the narrow
    // [7,0x10] four-slot fold. Arm order 7, 0xF, 0xD, 0x10 is the
    // image's body order. The 0xD blocks spell plain sums with y
    // loaded first - the image's load order. Measured 2026-09-18 in
    // deck dchunk-arm-readback (five arm spellings, both flag sets):
    // the read-back spelling pins both adds between the store pairs
    // but flips the loads x-first and leaves the add destinations
    // wrong (19 differing bytes per arm against plain sums' 18; the
    // operand-swap and x-sum-only read-back spellings measure
    // identical to it, a read-back y-sum alone byte-identical to
    // plain sums, and fresh member reads inside the sums miss the
    // recorded extent). The two adds still schedule above the store
    // pair instead of between it and the second - the same
    // add-placement residual fill() documents for the family,
    // size-neutral at 52 bytes.
    // Extent note: VC6 always emits these jump tables inside the
    // function's own COMDAT (624 total, code-to-ret 541), while the
    // recorded 0x0062ACD0-0x0062AEED extent stops at the ret - the
    // linked image carries its tables past that boundary - so the
    // extent check cannot admit any table-bearing spelling of this
    // function. The structure above still matches the image's dispatch
    // exactly; measured 541 code bytes + 80 table bytes + 3 pad.
    if (chunk->type_ == 0xf1fa) {
        int count = chunk->count_;
        if (count == 0) {
            return;
        }
        if (count <= 0) {
            return;
        }
        int remaining = count;
        do {
            struct {
                uint32_t size_;
                uint16_t type_;
                uint16_t count_;
            } sub;
            fread(&sub, 6, 1, file_);
            decode_chunk(reinterpret_cast<DataChunk *>(&sub));
        } while (--remaining != 0);
        return;
    }

    int end = ftell(file_) + chunk->size_ - 6;
    int size = chunk->size_;

    if (field_5B4_ & 2) {
        switch (chunk->type_) {
        case 7:
            decode_delta_flc_trans();
            break;
        case 0xf:
            decode_byte_run_trans();
            break;
        case 0xd: {
            int y = field_5B0_;
            int x = field_5AC_;
            int height = field_5A0_;
            int width = field_59C_;
            RECT *rect = &update_rect_;
            rect->left = x;
            rect->top = y;
            rect->right = x + width;
            rect->bottom = y + height;
            if (field_0_ != 0) {
                int (Buffer::*copy)(Buffer *, int, int, int, int, int,
                                    int) = &Buffer::copy;
                (buffer_.*copy)(buffer2_, 0, 0, field_5AC_, field_5B0_,
                                field_59C_, field_5A0_);
            }
            break;
        }
        case 0x10:
            decode_fli_copy_trans();
            break;
        case 4:
        case 0xb:
        case 0xc:
        case 0x12:
            goto skip;
        default:
skip:
            fseek(file_, size - 6, SEEK_CUR);
            break;
        }
    } else {
        switch (chunk->type_) {
        case 7:
            decode_delta_flc();
            break;
        case 0xf:
            decode_byte_run();
            break;
        case 0xd: {
            int y = field_5B0_;
            int x = field_5AC_;
            int height = field_5A0_;
            int width = field_59C_;
            RECT *rect = &update_rect_;
            rect->left = x;
            rect->top = y;
            rect->right = x + width;
            rect->bottom = y + height;
            buffer2_->fill(field_5AC_, field_5B0_, field_59C_,
                           field_5A0_, 0);
            break;
        }
        case 0x10:
            decode_fli_copy();
            break;
        case 4:
        case 0xb:
        case 0xc:
        case 0x12:
            goto skip2;
        default:
skip2:
            fseek(file_, size - 6, SEEK_CUR);
            break;
        }
    }

    int delta = abs(end - ftell(file_));
    if (delta != 0) {
        fseek(file_, delta, SEEK_CUR);
    }
}

/*
Purpose: Decode a type-0xF (FLI BYTE RUN) chunk in transparent mode: as
         decode_byte_run, but every pixel whose offset-adjusted value
         equals the transparent color is taken from the embedded scratch
         buffer instead of the file.
// ORIGINAL: 0x0062AF40 ?decode_byte_run_trans@Flic@@QAEXXZ 0x0062AF40-0x0062B186
// size      582 bytes
// prototype void (__thiscall ?decode_byte_run_trans@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E3373 0x005E33F3 0x00644EF2 0x00644F3A 0x00645930 0x00646178 0x006465F0
// TRIED: the image proves a 32-bit count local reloaded as a dword and
//        masked `& 0xff` at EVERY length use (memset arg, memcpy arg, both
//        cursor advances, the remaining decrement), the count slot reused
//        for the literal run length via `(uint8_t)abs((int8_t)count)` with
//        a byte store-back and an unsigned `count > 0` guard (`test al,al;
//        jbe`), and ONE byte-typed value slot shared by run and literal
//        paths (byte compare `cmp al,cl`, byte store-back; the run-path
//        spelling above compiles that path instruction-for-instruction as
//        measured in deck build/variants-round2 candidate count-reuse-
//        runval8). 14 spellings scored across decks variants-round1/2/3
//        (256-592 bytes): every byte-value spelling caps at 576 bytes
//        (extent-fail against the recorded 582), the 582-byte spellings
//        all widen the literal value to 32 bits - a dword compare the
//        image contradicts. The 6-byte gap and the differing bytes sit in
//        register allocation: all measured spellings open `push ebx` early
//        with `xor ebx,ebx` testing field_5A4_, cache remaining in ebx
//        with a write-back before each fread, reload both cursors after
//        calls, and lay out a 0x20 frame; the image defers `push ebx`/
//        `push ebp` under the row head, memory-homes remaining (round-trip
//        through [esp+0x18]), keeps both cursors in edi/ebp across the
//        packet loop, and uses a 0x1c frame with scratch immediate-zeroed
//        into it. No spelling in this family reaches that allocation; the
//        residual is compiler-side, consistent with the pending
//        discoveries on this entity. The literal loop's signed `jl` bound
//        against the unsigned guard also has no single-count-type
//        spelling (unsigned count gives `jbe` + `jb`, int count gives
//        `jle` + `jl`); unsigned kept for the guard and every masked
//        length, `int` measured identical (deck variants-round3
//        hoist-u8-icount, 576/864).
Return Value: n/a
Status: Complete
*/
void Flic::decode_byte_run_trans() {
    void *scratch = nullptr;
    if (field_5A4_ != 0) {
        scratch = mem_get(buffer2_->stride_);
        if (scratch == nullptr) {
            return;
        }
    }

    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;

    void *(Buffer::*get_data)() = &Buffer::get_data;
    uint8_t *row_dst = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));
    uint8_t *row_src = static_cast<uint8_t *>((buffer_.*get_data)());

    uint8_t value;
    int row = 0;
    if (static_cast<int>(field_5A0_) > 0) {
        do {
            int remaining = field_59C_;
            unsigned count;
            fread(&count, 1, 1, file_);
            uint8_t *dst = row_dst;
            uint8_t *src = row_src;
            for (; remaining > 0; remaining -= count & 0xff) {
                fread(&count, 1, 1, file_);
                if (static_cast<int8_t>(count) > 0) {
                    fread(&value, 1, 1, file_);
                    value = static_cast<uint8_t>(
                        value + static_cast<uint8_t>(field_5A4_));
                    if (value != field_0_) {
                        memset(dst, value, count & 0xff);
                    } else {
                        memcpy(dst, src, count & 0xff);
                    }
                    dst += count & 0xff;
                    src += count & 0xff;
                } else {
                    count = static_cast<uint8_t>(
                        abs(static_cast<int8_t>(count)));
                    int i = 0;
                    if (count > 0) {
                        do {
                            fread(&value, 1, 1, file_);
                            value = static_cast<uint8_t>(
                                value +
                                static_cast<uint8_t>(field_5A4_));
                            if (value != field_0_) {
                                *dst = value;
                            } else {
                                *dst = *src;
                            }
                            dst++;
                            src++;
                            i++;
                        } while (i < (count & 0xff));
                    }
                }
            }
            row_dst += buffer2_->stride_;
            row_src += buffer_.stride_;
            row++;
        } while (row < static_cast<int>(field_5A0_));
    }

    if (scratch != nullptr) {
        free(scratch);
    }
}

/*
Purpose: Decode a type-7 (FLC DELTA) chunk in transparent mode: as
         decode_delta_flc, but every pixel whose offset-adjusted value
         equals the transparent color is taken from the embedded scratch
         buffer instead of the file, per pixel or per RLE-word half.
// ORIGINAL: 0x0062B190 ?decode_delta_flc_trans@Flic@@QAEXXZ 0x0062B190-0x0062B5ED
// size      1117 bytes
// prototype void (__thiscall ?decode_delta_flc_trans@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E33F3 0x00644F3A 0x00646178
// TRIED: the opcode fread is single-site in the image (+136, the line
//        loop head; the line tail's `jl 0x62b204` at 0x0062B46B enters
//        the read's setup) - seven fread sites, thirteen calls total.
//        This tree spells one textual site through `do { fread; if
//        (...) break; ... } while (true)` and VC6 ROTATES that loop,
//        emitting two copies (entry +0x84, back edge +0xDE) - eight
//        sites, 1104 bytes under /O2, 1072 under /Oy-. Yet the image is
//        LARGER, not smaller: it carries ~168 more bytes between the
//        per-byte value read (+576) and the word read (+831) than any
//        measured spelling, offset by ~100 fewer after the word abs
//        (+842 to end) - the transparent byte loop and the mixed-word
//        loops trade extent between regions the way register allocation
//        schedules them, not the way any statement spelling moves.
//        Measured 2026-09-17 (decks flic-delta-loopmerge and
//        flic-trans-continue, both flag sets): the rotation duplicate is
//        spelling-invariant (every nested-cycle spelling is
//        byte-identical to the baseline), the continue-merge family
//        (skip as `continue` in the line loop, plain and with a dest
//        Buffer local) collapses the site but lands farther under
//        extent (1072/1056/1040 against 1117) with the same differing
//        byte count. No spelling reaches 1117; the residual is
//        VC6-side, the same class decode_delta_flc documents for this
//        loop pair.
Return Value: n/a
Status: Complete
*/
void Flic::decode_delta_flc_trans() {
    uint8_t value;
    uint8_t run;
    uint16_t opcode;
    uint8_t *dst = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));
    void *(Buffer::*get_data)() = &Buffer::get_data;
    uint8_t *src = static_cast<uint8_t *>((buffer_.*get_data)());
    int row;
    uint16_t word;
    int packet;
    uint16_t line_count;
    uint32_t max_x = 0;
    uint32_t min_x = 0xffffffff;
    int max_y = 0;
    int min_y = -1;
    uint8_t skip_count;

    fread(&line_count, 2, 1, file_);
    row = 0;
    if (line_count > 0) {
        do {
            do {
                fread(&opcode, 2, 1, file_);
                if ((opcode & 0xc000) != 0xc000) {
                    break;
                }
                int skip = abs(static_cast<int16_t>(opcode));
                opcode = static_cast<uint16_t>(skip);
                dst += buffer2_->stride_ * (skip & 0xffff);
                src += buffer_.stride_ * (skip & 0xffff);
            } while (true);
            if (row == 0) {
                uint8_t *base = reinterpret_cast<uint8_t *>(
                    buffer2_->get_data(field_5AC_, field_5B0_));
                min_y = static_cast<int>(dst - base) /
                        static_cast<int>(buffer2_->stride_);
            }
            if (row == static_cast<int>(line_count) - 1) {
                uint8_t *base = reinterpret_cast<uint8_t *>(
                    buffer2_->get_data(field_5AC_, field_5B0_));
                max_y = static_cast<int>(dst - base) /
                        static_cast<int>(buffer2_->stride_);
            }
            if (opcode & 0x8000) {
                uint16_t last = static_cast<uint16_t>(
                    static_cast<uint8_t>(field_5A4_ +
                                         static_cast<int8_t>(opcode)));
                if (last != static_cast<uint16_t>(field_0_)) {
                    dst[field_59C_ - 1] = static_cast<uint8_t>(last);
                } else {
                    dst[field_59C_ - 1] = src[field_59C_ - 1];
                }
                fread(&opcode, 2, 1, file_);
            }
            packet = 0;
            uint8_t *out = dst;
            if (opcode > 0) {
                do {
                    fread(&skip_count, 1, 1, file_);
                    out += skip_count;
                    src += skip_count;
                    if (packet == 0 &&
                        static_cast<uint32_t>(out - dst) < min_x) {
                        min_x = out - dst;
                    }
                    fread(&run, 1, 1, file_);
                    if (static_cast<int8_t>(run) > 0) {
                        run = static_cast<uint8_t>(run * 2);
                        int i = 0;
                        if (run != 0) {
                            do {
                                fread(&value, 1, 1, file_);
                                value = static_cast<uint8_t>(
                                    value + field_5A4_);
                                if (field_0_ != value) {
                                    *out = value;
                                } else {
                                    *out = *src;
                                }
                                out++;
                                src++;
                                i++;
                            } while (i < run);
                        }
                    } else if (static_cast<int8_t>(run) < 0) {
                        fread(&word, 2, 1, file_);
                        run = static_cast<uint8_t>(
                            abs(static_cast<int8_t>(run)));
                        word = static_cast<uint16_t>(
                            (static_cast<uint8_t>(
                                 static_cast<uint8_t>(word >> 8) +
                                 field_5A4_)
                             << 8) +
                            static_cast<uint8_t>(word + field_5A4_));
                        if (static_cast<uint8_t>(word) == field_0_) {
                            if (static_cast<uint8_t>(word >> 8) ==
                                field_0_) {
                                for (int i = 0; i < run; i++) {
                                    *reinterpret_cast<uint16_t *>(out) =
                                        *reinterpret_cast<uint16_t *>(src);
                                    src += 2;
                                    out += 2;
                                }
                            } else {
                                for (int i = 0; i < run; i++) {
                                    out[0] =
                                        static_cast<uint8_t>(word >> 8);
                                    out[1] = src[1];
                                    out += 2;
                                    src += 2;
                                }
                            }
                        } else if (static_cast<uint8_t>(word >> 8) ==
                                   field_0_) {
                            for (int i = 0; i < run; i++) {
                                out[0] = src[0];
                                out[1] = static_cast<uint8_t>(word);
                                out += 2;
                                src += 2;
                            }
                        } else {
                            for (int i = 0; i < run; i++) {
                                *reinterpret_cast<uint16_t *>(out) = word;
                                out += 2;
                            }
                        }
                    }
                    packet++;
                } while (packet < opcode);
            }
            if (max_x < static_cast<uint32_t>(out - dst)) {
                max_x = out - dst;
            }
            dst += buffer2_->stride_;
            src += buffer_.stride_;
            row++;
        } while (row < static_cast<int>(line_count));
    }
    update_rect_.left = field_5AC_ + min_x;
    update_rect_.top = field_5B0_ + min_y;
    update_rect_.right = field_5AC_ + 1 + max_x;
    update_rect_.bottom = field_5B0_ + 1 + max_y;
}

/*
Purpose: Handle a type-0xD (FLI BLACK) chunk in transparent mode: restore
         the embedded scratch buffer over the animation's rectangle.
// ORIGINAL: 0x0062B600 ?decode_black_trans@Flic@@QAEXXZ 0x0062B600-0x0062B66E
// size      110 bytes
// prototype void (__thiscall ?decode_black_trans@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFF00
Return Value: n/a
Status: Complete
*/
void Flic::decode_black_trans() {
    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;
    if (field_0_ != 0) {
        int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
            &Buffer::copy;
        (buffer_.*copy)(buffer2_, 0, 0, field_5AC_, field_5B0_, field_59C_,
                        field_5A0_);
    }
}

/*
Purpose: Decode a type-0x10 (FLI COPY) chunk in transparent mode: the
         rows follow verbatim in the file, and every pixel whose
         offset-adjusted value equals the transparent color is taken from
         the embedded scratch buffer instead.
// ORIGINAL: 0x0062B670 ?decode_fli_copy_trans@Flic@@QAEXXZ 0x0062B670-0x0062B776
// size      262 bytes
// prototype void (__thiscall ?decode_fli_copy_trans@Flic@@QAEXXZ)(Flic* this)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E33F3 0x00646178
Return Value: n/a
Status: Complete
*/
void Flic::decode_fli_copy_trans() {
    int y = field_5B0_;
    int x = field_5AC_;
    int height = field_5A0_;
    int width = field_59C_;
    RECT *rect = &update_rect_;
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;

    void *(Buffer::*get_data)() = &Buffer::get_data;
    uint8_t *dst = reinterpret_cast<uint8_t *>(
        buffer2_->get_data(field_5AC_, field_5B0_));
    uint8_t *src = static_cast<uint8_t *>((buffer_.*get_data)());

    for (int row = 0; row < static_cast<int>(field_5A0_); row++) {
        for (int i = 0; i < static_cast<int>(field_59C_); i++) {
            uint8_t value;
            fread(&value, 1, 1, file_);
            value = static_cast<uint8_t>(value + field_5A4_);
            if (value != field_0_) {
                *dst = value;
            } else {
                *dst = *src;
            }
            dst++;
            src++;
        }
        dst += buffer2_->stride_ - field_59C_;
        src += buffer_.stride_ - field_59C_;
    }
}

/*
Purpose: Copy the rectangle the animation occupies on the destination
         buffer back into the embedded scratch buffer - the counterpart
         of the transparent-mode restore the black-chunk decoder performs.
// ORIGINAL: 0x0062B780 ?UNK9@Flic@@QAEXXZ 0x0062B780-0x0062B7BF FILE BYTE_EXACT
// size      63 bytes
// prototype void (__thiscall ?UNK9@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFF00
Return Value: n/a
Status: Complete
*/
void Flic::UNK9() {
    if (buffer2_ && (field_5B4_ & 1)) {
        int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
            &Buffer::copy;
        (buffer2_->*copy)(&buffer_, field_5AC_, field_5B0_, 0, 0, field_59C_,
                          field_5A0_);
    }
}





