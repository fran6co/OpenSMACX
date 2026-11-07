/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// The VOX library's .cvr loader and engine setup, 0x006381F0..0x006393C0 in
// the image, grouped by address. The walk/IO/message/record helpers between
// these bodies keep their own files; this grouping does not assert the
// original translation-unit boundary.
#include "stdafx.h"
#include "caviar.h"
#include "caviar_io.h"
#include "caviar_cpu.h"
#include "vox_engine.h"
#include "vector.h"

#include <stddef.h>
#include <string.h>

// The library was built at /Ob1 (src/caviar/CMakeLists.txt): the image
// calls vox_read_dword and vox_read_length_field out of line from
// vox_load_record. strlen expands inline (repne scasb at 0x6383F7) where
// stdafx.h turns the intrinsic off for the game.
#pragma intrinsic(strlen)

// ---------------------------------------------------------------------------
// The voxel engine's own entry points, called by init_class above through
// caviar.h's declarations. None is a Caviar member; they are the engine
// (0x6392E0..0x63F9B0 in the image) operating on its own fixed globals.
// ---------------------------------------------------------------------------



// The engine's data slots, real storage now - each was a binding on the
// image address in its name, which the standalone build does not map.
typedef char (__cdecl *vox_user_chunk_fn)(int, CvrRecord *, CvrChunkHeader *);
// RECOVERY_OBJECT: v2 0x009C0834 0x009C0838 ?vox_user_chunk@@3P6ADHPAUCvrRecord@@PAUCvrChunkHeader@@@ZA 0x006381F0
vox_user_chunk_fn vox_user_chunk = 0; // 0x009C0834, zeroed on accepted CPU report
// Pinned 63934C writes one DWORD at9C0B78 on accepted CPU setup.
// This source scalar owns the corresponding storage; its complete original
// allocation extent and an original descriptive name remain unproved.
// RECOVERY_OBJECT: v1 0x009C0B78 ?vox_unk_9c0b78@@3IA
uint32_t vox_unk_9c0b78 = 0;    // 0x009C0B78, zeroed on accepted CPU report
// caviar_cpu.cpp owns the original0x009C0B70 vendor byte: 0 unknown,
// 1 GenuineIntel, 2 AuthenticAMD.
// Original 6395EC/6395F2 increments and 639680/63968D decrements this counter.
// Bind its observed start; an exclusive original allocation is not established.
// RECOVERY_OBJECT: v1 0x009BE69C ?vox_record_count@@3IA
uint32_t vox_record_count = 0;  // 0x009BE69C, records created, bumped per create

// The pixel-size dispatch tables the engine's two dispatchers (sub_63ad60
// for CPU ranks 1/2/4, sub_63ae20 for ranks 3/5) store into the record's
// five slots (+0x6c, +0x70, +0x74, +0x78, +0x7c), selected by pixel size
// and the record's alignment flag byte. Carried as address integers named by
// slot until the engine's behaviour names the tables themselves. The two
// dispatchers are byte-identical twins in the shipped image, so both bodies
// store the SAME five tables.

// The engine's message strings. Real literals now - the image keeps them in
// .rdata at the addresses each name carried, and vox_stage_error copies from the
// pointer it is handed, so a raw binding dereferenced unmapped memory the
// moment any error arm ran. "Unsupported pixel size" is different in the
// image: a data SLOT at 0x00698A9C holds the string's pointer and the body
// loads through it. Keep that writable pointer slot separate from its text.
// RECOVERY_OBJECT: v2 0x00698AFC 0x00698B0F _vox_msg_no_setup 0x006185A0
static const char vox_msg_no_setup[] = "No setup parameter";        // 0x00698AFC
// RECOVERY_OBJECT: v2 0x00698AD8 0x00698AE7 _vox_msg_setup_empty 0x006185A0
static const char vox_msg_setup_empty[] = "Setup is empty";         // 0x00698AD8
// RECOVERY_OBJECT: v2 0x00698AE8 0x00698AFA _vox_msg_cpu_unsupported 0x006185A0
static const char vox_msg_cpu_unsupported[] = "CPU not supported";  // 0x00698AE8
// RECOVERY_OBJECT: v2 0x00698B10 0x00698B29 _vox_msg_no_shadow 0x006393C0
static const char vox_msg_no_shadow[] = "No room for shadow table"; // 0x00698B10
// RECOVERY_OBJECT: v2 0x00698B2C 0x00698B44 _vox_msg_no_colour 0x006393C0
static const char vox_msg_no_colour[] = "No room for color table";  // 0x00698B2C
// RECOVERY_OBJECT: v2 0x00698B44 0x00698B58 _vox_msg_no_handler 0x006393C0
static const char vox_msg_no_handler[] = "No room for handler";     // 0x00698B44
// RECOVERY_OBJECT: v2 0x00698B58 0x00698B6E _vox_msg_no_colortab 0x006393C0
static const char vox_msg_no_colortab[] = "No ColorTab parameter";  // 0x00698B58
// RECOVERY_OBJECT: v2 0x00698AC0 0x00698AD7 _vox_msg_unsupported_pixel 0x00698A9C
static const char vox_msg_unsupported_pixel[] = "Unsupported pixel size"; // 0x00698AC0
// The image keeps a distinct writable pointer slot in .data at 0x00698A9C;
// this is not another literal and its initializer points to the message above.
// RECOVERY_OBJECT: v2 0x00698A9C 0x00698AA0 _vox_msg_unsupported_pixel_slot 0x006393C0
static const char *vox_msg_unsupported_pixel_slot = vox_msg_unsupported_pixel; // 0x00698A9C

// Three more of the same pool, named 2026-09-04 by reading the image's own
// pointer table: 0x006989CC..0x006989E0 is ONE run of six message slots -
// "File seeking", "Bad file format", "Reading file", "No room", "Invalid
// color buffer depth", "Error reading user data chunk" - and the CVR loader
// (0x00638620..0x00638DE0) hands the last five of them to vox_stage_error.
// The parser retains these message pointers; the exact-reader and allocator
// messages live with their shared callback operations in caviar_io.cpp.
// The eight engine messages above stay literals because their callers push
// the address straight out.
// RECOVERY_OBJECT: v1 0x006989D0 _vox_msg_bad_file_format
static const char *const vox_msg_bad_file_format = "Bad file format";  // 0x006989D0
// RECOVERY_OBJECT: v1 0x006989E0 _vox_msg_user_chunk
static const char *const vox_msg_user_chunk = "Error reading user data chunk"; // 0x006989E0


// The loader's two float constants, .rdata 0x00670FE8 and 0x00670FEC. The
// first is the chain walk's cut: vox_walk_scaled_chain keeps walking while a
// node's extent, scaled by the caller's factor, stays below it. 1.4142f is
// the exact float the image holds (0x3FB50481).
// RECOVERY_OBJECT: v2 0x00670FE8 0x00670FEC ?vox_sqrt_two@@3MB 0x006385D0
extern const float vox_sqrt_two = 1.4142f;       // 0x00670FE8
// The 7.0f vox_read_chunks tests the record's float at +0x8 against to
// choose between loading the 0x1020000 chunk and expanding it in place.
static const float vox_threshold_seven = 7.0f;   // 0x00670FEC




// Failures return without releasing partially built records, as in the image.
// ORIGINAL: 0x006381F0 ?vox_load_record@@YAPAUCvrRecord@@H@Z 0x006381F0-0x00638425 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 006381F0..00638425: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
CvrRecord *__cdecl vox_load_record(int fd) {
    CvrChunkHeader header;
    if (vox_read_exact(fd, &header, sizeof(header)) != 0) return 0;
    if (header.tag != 0x00525643) {
        vox_stage_error(vox_msg_bad_file_format);
        return 0;
    }
    int remaining = header.length - 8;
    CvrRecord *record;
    if (vox_alloc_typed(sizeof(CvrRecord), &record) != 0) return 0;
    memset(record, 0, sizeof(*record));
    while (remaining != 0) {
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return 0;
        remaining -= header.length;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return 0;
        }
        switch (header.tag) {
        case 0x01000000:
            if (vox_read_dword(fd, record) != 0) return 0;
            break;
        case 0x02000000:
            if (vox_read_length_field(fd, record, header.length) != 0) return 0;
            break;
        case 0x03000000:
            if (vox_read_chunks(fd, record, header.length) != 0) return 0;
            break;
        case 0x04000000:
            if (vox_read_parts(fd, record, header.length) != 0) return 0;
            break;
        case 0x0F000000: {
            vox_user_chunk_fn callback = vox_user_chunk;
            CaviarOutputMessageWindow.message_[0] = 0;
            if (callback == 0) {
                if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                    vox_stage_error(vox_msg_bad_file_format);
                    return 0;
                }
            } else {
                if (callback(fd, record, &header) != 0) {
                    if (strlen(CaviarOutputMessageWindow.message_) == 0) vox_stage_error(vox_msg_user_chunk);
                    return 0;
                }
                // The callback may rewrite the header; remaining was already
                // decremented, but this stored payload length is a fresh read.
                record->user_length_ = header.length - 8;
            }
            break;
        }
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return 0;
            }
            break;
        }
    }
    record->state_ = 3;
    return record;
}


/*
Purpose: Free a CVR record: five plain pointer fields (0xc, 0x10, 0x14, 0x18,
         0x1c), then four arrays sized by the shared count at +0x24 - a
         pointer array at +0x28, a 12-byte-element array at +0x30 and an
         8-byte-element array at +0x2c, and a second 12-byte array at +0x34,
         each element's leading dwords being pointers - then the record
         itself. Every release goes through the engine's free slot.
// ORIGINAL: 0x00638430 sub_638430 0x00638430-0x006385CD BYTE_EXACT
// TRIED: teardown - frees five plain pointer fields (0xc/0x10/0x14/ 0x18/0x1c), three arrays of small structs at 0x28/0x2c/0x30/ 0x34 sized by a shared count field at 0x24, then the object itself through the free callback at g_009c0b84. Diverges at #6 - the original re-reads each array-base field inside the loop rather than caching it once; landed at 0.64 similarity.
// symbol    ?caviar_free_record@@YAXPAUCvrRecord@@@Z
// size      413 bytes
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638447 0x00638458 0x00638469 0x0063847A 0x0063848B 0x006384AB 0x006384C0 0x006384E6 0x006384FB 0x00638510 0x00638528 0x0063854D 0x00638562 0x00638589 0x0063859E 0x006385B6 0x006385C0
//
// NAMED 2026-09-04: caviar.h already reached this body by this name through a
// pending_bodies forwarder, and CaviarData::close is its one caller. The
// binding it freed through (g_009c0b84) is the engine's free slot, which this
// file already models as vox_free_slot.
//
// HOMED from src/unrecovered/00638430.cpp.
// LEVER: the array bases (+0x28, +0x30, +0x34) and the count (+0x24) are
//        RE-READ inside every loop - the image loads [esi+0x30] at 0x6384DB,
//        0x6384EF and 0x638504 within one iteration - so the loops below
//        spell those field reads out in full. Caching either in a local is
//        what the artifact did, and what diverged at instruction 6.
// BUG IN THE ORIGINAL: the +0x28 pointer array's elements are freed WITHOUT a
//        null test (image 0x6384A4..0x6384AB pushes whatever it finds) while
//        the other three arrays' elements are tested first. Reproduced as
//        written - a NULL element there would fault inside the allocator.
// DEFECT FIXED 2026-09-04: the scaffold this came from had NO +0x2c array
//        walk at all - it freed +0x28, +0x30 and +0x34 and silently dropped
//        the 8-byte-element array between them, so a third of every record's
//        allocations leaked. The image walks it at 0x638531..0x638568; the
//        loop is restored here. 55 of 154 -> 102 of 154 with the same
//        instruction count on both sides.
// TRIED: flipping the SIB base - `offset + *(int *)(base + 0x30)` against the
//        image's [edi+eax], where this tree emits [eax+edi] - measured no
//        change at all; VC6 normalises the sum before register allocation, so
//        the base/index split is codegen's, not the source's. 6 of the 19
//        remaining differing runs.
// TRIED: the image pushes edi at instruction 5 and ebx at 7, inside the first
//        field test's guard, and pops them in that order at the single exit;
//        every spelling here saves both at the first array loop instead (and
//        pops ebx first). The eager-save placement is upstream of the body -
//        same wall the Palette receivers hit. 8 of the 19 remaining runs.
//        102 of 154, /c /O2 /Gy /GR- /GX.
// LEVER: one function-scope index and `for` loops put the image's index
//        zeroing ahead of each count test: 114 of 154. Wrapping the body in
//        `if (record != 0)` instead of the early return measured the same.
Return Value: n/a
*/
void __cdecl caviar_free_record(CvrRecord *record) {
    if (record == 0) {
        return;
    }
    int index;
    char *first = record->description_;
    if (first != 0) {
        vox_free_slot(first);
    }
    char *second = record->palette_name_;
    if (second != 0) {
        vox_free_slot(second);
    }
    uint8_t *third = record->palette_;
    if (third != 0) {
        vox_free_slot(third);
    }
    uint8_t *fourth = record->field_18_;
    if (fourth != 0) {
        vox_free_slot(fourth);
    }
    char *fifth = record->parts_name_;
    if (fifth != 0) {
        vox_free_slot(fifth);
    }

    if (record->slot_table_ != 0) {
        for (index = 0; index < record->part_count_; ++index) {
            // No null test - the image frees the slot's answer whatever
            // it holds.
            char *element = record->slot_table_[index];
            vox_free_slot(element);
        }
        vox_free_slot(record->slot_table_);
    }

    if (record->parts_ != 0) {
        for (index = 0; index < record->part_count_; ++index) {
            uint8_t *lead = record->parts_[index].colours_;
            if (lead != 0) {
                vox_free_slot(lead);
            }
            VOX_Vect *mid = record->parts_[index].vectors_;
            if (mid != 0) {
                vox_free_slot(mid);
            }
            VOX_Matrix *last = record->parts_[index].matrices_;
            if (last != 0) {
                vox_free_slot(last);
            }
        }
        vox_free_slot(record->parts_);
    }

    if (record->steps_ != 0) {
        for (index = 0; index < record->part_count_; ++index) {
            CvrShape *lead = record->steps_[index].shape_;
            if (lead != 0) {
                vox_free_slot(lead);
            }
        }
        vox_free_slot(record->steps_);
    }

    if (record->shadows_ != 0) {
        for (index = 0; index < record->part_count_; ++index) {
            uint8_t *lead = record->shadows_[index].field_4_;
            if (lead != 0) {
                vox_free_slot(lead);
            }
            uint8_t *mid = record->shadows_[index].field_8_;
            if (mid != 0) {
                vox_free_slot(mid);
            }
        }
        vox_free_slot(record->shadows_);
    }

    vox_free_slot(record);
}


/*
Purpose: Read a single 4-byte field at offset +8 of the caller's block: the
         CVR loader's fixed-size header reader, vox_read_exact wrapped for one
         dword.
// ORIGINAL: 0x006386B0 sub_6386b0 0x006386B0-0x006386CC BYTE_EXACT
// symbol    _vox_read_dword
// size      28 bytes
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638650
//
// CLAIMED BYTE_EXACT, homed from src/recovered/006386b0.cpp 2026-09-04 and
// re-measured here. NAMED from the body: a four-byte read at +8 through
// vox_read_exact. Its one callee is homed in this file, so the call stays
// `call rel32`.
Return Value: 0 when the four bytes were read, -1 otherwise.
*/
extern "C" int __cdecl vox_read_dword(int fd, CvrRecord *record) {
    return vox_read_exact(fd, &record->version_, sizeof(record->version_)) ? -1 : 0;
}

/*
Purpose: Allocate (length - 8) + 1 bytes into the record's slot at +0xc, read
         that many bytes into it and NUL-terminate: the CVR loader's reader
         for the chunks it keeps as text (vox_read_chunks' 0x1010000 case
         is the same code against +0x10 instead of +0xc).
// ORIGINAL: 0x006386D0 sub_6386d0 0x006386D0-0x0063871D FILE BYTE_EXACT
// symbol    _vox_read_length_field
// size      77 bytes
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
//
// HOMED from src/recovered/units/006386d0.cpp.
Return Value: 0 when the field is read and terminated, -1 on either failure.
*/
extern "C" int __cdecl vox_read_length_field(int fd, CvrRecord *record,
                                             int length) {
    CvrRecord *base = record;
    length -= 8;
    // The slot lives at +0xc of the record the caller hands over - the same
    // field caviar_free_record releases first.
    char **slot = &base->description_;
    if (vox_alloc_typed(length + 1, slot) != 0) {
        return -1;
    }
    if (vox_read_exact(fd, (void *)*slot, length) != 0) {
        return -1;
    }
    // Through the field, not the slot: `(*slot)[length]` gives [ecx+esi]
    // where the image has [esi+ecx].
    base->description_[length] = 0;
    return 0;
}


/*
Purpose: Walk a CVR record's chunk list. Each chunk is an 8-byte {tag,
         length} header followed by length-8 payload bytes, and `size` is the
         whole block's extent; the walk stops when the remaining count runs
         out. Three tags are read into the record - 0x1010000 (a NUL-
         terminated text field, into +0x10), 0x1020000 (the palette, into
         +0x14, expanded in place when the record's float at +8 is under 7),
         0x1030000 (into +0x18) - and anything else is seeked past.
// ORIGINAL: 0x00638720 sub_638720 0x00638720-0x006388B2 BYTE_EXACT
// TRIED: chunked-record reader (tag/length header, three named chunk tags 0x1010000/0x1020000/0x1030000 plus a default skip through the indirect callback at g_009c0b90) - diverges at #3 with the loop-entry `lea` vs `push`, register/stack layout differs from here on; landed at 0.73 similarity.
// symbol    _vox_read_chunks
// size      402 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0063878D
//
// HOMED from src/unrecovered/00638720.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (slot 4 of the six vx_* callbacks, this
// file's vox_seek_slot - the payload size and SEEK_CUR's 1 are its two
// arguments), g_006989d0 the "Bad file format" message slot and g_00670fec
// the 7.0f at 0x00670FEC.
// LEVER: the tag dispatch is a switch. The image compares 0x1010000/
//        0x1020000/0x1030000 with three cmp/je pairs because the values are
//        not sequential; an if/else-if chain spells the same thing here and
//        a switch spells it too - the two were measured identical on
//        0x00638DE0, whose tags ARE sequential and whose chain is sub/dec/je.
// The historical seed used < 7.0 for the old-palette arm. The original
//        0x006387D3 TEST AH,0x41 selects that arm for <= 7.0 and unordered
//        values; only > 7.0 takes the newer payload path. No overflow is
//        checked. Payload lengths survive the allocator/reader callbacks.
Return Value: 0 when the walk consumed the block, -1 on any failure.
*/
extern "C" int __cdecl vox_read_chunks(int fd, CvrRecord *record, int size) {
    int remaining = size - 8;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x1010000: {
            const int payload = header.length - 8;
            if (vox_alloc_typed(payload + 1, &record->palette_name_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, record->palette_name_, payload) != 0) {
                return -1;
            }
            record->palette_name_[payload] = 0;
            break;
        }
        case 0x1020000: {
            const int payload = header.length - 8;
            if (!(record->version_ > vox_threshold_seven)) {
                if (vox_alloc_typed(0x1b02, &record->palette_) != 0) {
                    return -1;
                }
                if (vox_expand_palette(fd, record->palette_) != 0) {
                    return -1;
                }
            } else {
                if (vox_alloc_typed(payload, &record->palette_) != 0) {
                    return -1;
                }
                if (vox_read_exact(fd, record->palette_, payload) != 0) {
                    return -1;
                }
            }
            break;
        }
        case 0x1030000: {
            // VC5 cross-jumps this arm's read call into the palette arm's
            // (0x6387BF) only with the cases in ascending order.
            const int payload = header.length - 8;
            if (vox_alloc_typed(payload, &record->field_18_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, record->field_18_, payload) != 0) {
                return -1;
            }
            break;
        }
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}


// ORIGINAL: 0x006388C0 ?vox_read_parts@@YAHHPAUCvrRecord@@H@Z 0x006388C0-0x00638AC2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 006388C0..00638AC2: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_parts(int fd, CvrRecord *record, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    // The format supplies the count chunk before entries (or the final count
    // check). The original has no initialized fallback for a missing count.
    int expected_count;
    int index;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((int32_t)remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x04010000: {
            int payload = header.length - 8;
            if (vox_alloc_typed((uint32_t)payload + 1u, &record->parts_name_) != 0) return -1;
            if (vox_read_exact(fd, record->parts_name_, payload) != 0) return -1;
            record->parts_name_[payload] = 0;
            break;
        }
        case 0x04020000: {
            if (vox_read_exact(fd, &expected_count, sizeof(expected_count)) != 0) return -1;
            uint32_t names_bytes = (uint32_t)expected_count * sizeof(*record->slot_table_);
            if (vox_alloc_typed(names_bytes, &record->slot_table_) != 0) return -1;
            memset(record->slot_table_, 0, names_bytes);
            uint32_t parts_bytes = (uint32_t)expected_count * sizeof(*record->parts_);
            if (vox_alloc_typed(parts_bytes, &record->parts_) != 0) return -1;
            memset(record->parts_, 0, parts_bytes);
            record->part_count_ = expected_count;
            index = 0;
            break;
        }
        case 0x04030000:
            if (vox_read_exact(fd, &record->step_count_, sizeof(record->step_count_)) != 0) return -1;
            break;
        case 0x04040000:
            if (vox_read_part(fd, record, index, header.length) != 0) return -1;
            ++index;
            break;
        case 0x04050000:
            if (vox_read_shape_patches(fd, record, header.length) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    if (expected_count != index) {
        vox_stage_error("Number of objects mismatch");
        return -1;
    }
    return 0;
}


// ORIGINAL: 0x00638AD0 ?vox_read_part@@YAHHPAUCvrRecord@@HH@Z 0x00638AD0-0x00638CB9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 00638AD0..00638CB9: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_part(int fd, CvrRecord *record, int index, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((int32_t)remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x04040100: {
            int payload = header.length - 8;
            if (vox_alloc_typed((uint32_t)payload + 1u, &record->slot_table_[index]) != 0) return -1;
            if (vox_read_exact(fd, record->slot_table_[index], payload) != 0) return -1;
            record->slot_table_[index][payload] = 0;
            break;
        }
        case 0x04040200:
            if (record->steps_ == 0) {
                uint32_t bytes = (uint32_t)record->part_count_ * sizeof(*record->steps_);
                if (vox_alloc_typed(bytes, &record->steps_) != 0) return -1;
                memset(record->steps_, 0, bytes);
            }
            if (vox_read_colour_chunks(fd, &record->steps_[index], header.length) != 0) return -1;
            if (record->steps_[index].lead_ != 0) {
                record->flags_ |= 1u;
            } else {
                record->flags_ &= ~1u;
                record->steps_[index].lead_ = record->steps_[index].shape_->byte_size_;
            }
            break;
        case 0x04040300:
            if (vox_read_geometry_chunks(fd, &record->parts_[index], header.length, record->step_count_) != 0) return -1;
            break;
        case 0x04040400:
            if (vox_read_shadow(fd, record, index, header.length) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}


/*
Purpose: Read a CVR record's colour chunks. Two tags: 0x4040201 is a colour
         table whose expected size arrives in *out (the caller's slot is
         {requested size, filled pointer} - an entry of zero means take the
         chunk's own size), and 0x4040202 is a table read straight into the
         caller's array. Anything else is seeked past.
// ORIGINAL: 0x00638CC0 sub_638cc0 0x00638CC0-0x00638DD4 BYTE_EXACT
// TRIED: only one source-form tried; the callee-save register allocation (ebx/ebp/esi/edi across the whole loop) at the prologue diverges early. Landing the closest structural translation (mnemonic_similarity 0.635).
// symbol    _vox_read_colour_chunks
// size      276 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638D1B
//
// HOMED from src/unrecovered/00638cc0.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (vox_seek_slot here) and g_006989d0 the
// "Bad file format" message slot.
Return Value: 0 when the walk consumed the block, -1 on any failure.
// TRIED: the image dedicates ebx to the caller's {size, pointer} array for
//        the whole walk, loaded in the same breath as the fd (0x638CD6 then
//        0x638CDA); this tree re-loads it inside the 0x4040201 case, and
//        every spelling tried (a local alias, a subscripted read, a read at
//        the top of the loop) left it deferred. 53 of 116 at /O2, first
//        divergence at instruction 8 - the missing ebx load. /c /O2 /Gy
//        /GR- /GX.
// LEVER: the image tests lead_ and falls through to the lead_ allocation,
//        so the nonzero arm is spelled first (82 of 115). A top `if` plus
//        do-while for the two return-0 epilogues measured worse (56).
*/
extern "C" int __cdecl vox_read_colour_chunks(int fd, CvrStep *out, int size) {
    struct ChunkHeader {
        int tag;
        int length;
    };
    int remaining = size - 8;
    while (remaining != 0) {
        ChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        const int payload = header.length - 8;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x4040201:
            if (out->lead_ != 0) {
                if (vox_alloc_typed(out->lead_, &out->shape_) != 0) {
                    return -1;
                }
            } else {
                if (vox_alloc_typed(payload, &out->shape_) != 0) {
                    return -1;
                }
            }
            if (vox_read_exact(fd, out->shape_, payload) != 0) {
                return -1;
            }
            break;
        case 0x4040202:
            if (vox_read_exact(fd, out, payload) != 0) {
                return -1;
            }
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}


/*
Purpose: Read a CVR record's geometry chunks. Three tags name the three
         arrays the record keeps - 0x4040301 into slots[0] (count bytes),
         0x4040302 into slots[1] (count*12 bytes, 3 dwords per element) and
         0x4040303 into slots[2] (count*36 bytes, 9 dwords per element) -
         and anything else is seeked past. The whole block spans `size`
         bytes and `count` is the element count the chunk headers repeat.
// ORIGINAL: 0x00638DE0 sub_638de0 0x00638DE0-0x00638F2A BYTE_EXACT
// TRIED: calling sub_638620/sub_638650/sub_639390 by name - all three are catalogued unmangled with no CRT_SIGNATURES entry, so declfix leaves them nullary (C2660 direct, C2733 if redeclared); routed through function-pointer casts. The fallback branch's indirect call goes through `*(int*)0x9c0b90`, a stored function pointer value (not a vtable), called the same way. Sharing ONE `sub_638650` call site between the 0x4040302/0x4040303 cases via goto (matching the disassembly's single shared call at 0x638e99) was not attempted - the two branches' locals would cross the jump into each other's scope, which C++ refuses; each case is expanded with its own call instead. Best reached: MISMATCH, edit_count 17, 315 bytes vs 330, mnemonic similarity ~0.67.
// symbol    _vox_read_geometry_chunks
// size      330 bytes
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638E3E
//
// HOMED from src/unrecovered/00638de0.cpp. Its bindings are named from the
// image: g_009c0b90 is the seek slot (vox_seek_slot here) and g_006989d0 the
// "Bad file format" message slot.
// LEVER: the tag dispatch is a switch spelled in ascending order - the
//        image's 0x4040301/0x4040302/0x4040303 chain is sub eax,0x4040301 /
//        je / dec eax / je / dec eax / je, which only sequential case values
//        produce (0x00638720's non-sequential tags compare with cmp/je).
Return Value: 0 when the walk consumed the block, -1 on any failure.
// LEVER: under VC5 the cases in ascending order give the image's shared
//        read call (the matrices arm jumps into the vectors arm's).
*/
extern "C" int __cdecl vox_read_geometry_chunks(int fd, CvrPart *slots, int size,
                                                int count) {
    struct ChunkHeader {
        int tag;
        int length;
    };
    int remaining = size - 8;
    while (remaining != 0) {
        ChunkHeader header;
        if (vox_read_exact(fd, &header, 8) != 0) {
            return -1;
        }
        remaining -= header.length;
        if (remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        switch (header.tag) {
        case 0x4040301:
            if (vox_alloc_typed(count, &slots->colours_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->colours_, count) != 0) {
                return -1;
            }
            break;
        case 0x4040302:
            if (vox_alloc_typed(count * sizeof(VOX_Vect), &slots->vectors_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->vectors_, count * sizeof(VOX_Vect)) != 0) {
                return -1;
            }
            break;
        case 0x4040303:
            if (vox_alloc_typed(count * sizeof(VOX_Matrix), &slots->matrices_) != 0) {
                return -1;
            }
            if (vox_read_exact(fd, slots->matrices_, count * sizeof(VOX_Matrix)) != 0) {
                return -1;
            }
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, header.length - 8, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}

// ORIGINAL: 0x00638F30 ?vox_read_shadow@@YAHHPAUCvrRecord@@HH@Z 0x00638F30-0x00639086 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 00638F30..00639086: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_shadow(int fd, CvrRecord *record, int index, int size) {
    if (record->shadows_ == 0) {
        if (vox_alloc_typed((uint32_t)record->part_count_ * sizeof(*record->shadows_), &record->shadows_) != 0) return -1;
        // Unlike the name/part/step arrays, the original does not clear this.
    }
    uint32_t remaining = (uint32_t)size - 8u;
    CvrShadow *entry = &record->shadows_[index];
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((int32_t)remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        int payload = header.length - 8;
        switch (header.tag) {
        case 0x04040401:
            if (vox_read_exact(fd, entry, payload) != 0) return -1;
            break;
        case 0x04040402:
            if (vox_alloc_typed(payload, &entry->field_4_) != 0) return -1;
            if (vox_read_exact(fd, entry->field_4_, payload) != 0) return -1;
            break;
        case 0x04040403:
            if (vox_alloc_typed(payload, &entry->field_8_) != 0) return -1;
            if (vox_read_exact(fd, entry->field_8_, payload) != 0) return -1;
            break;
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}


// ORIGINAL: 0x00639090 ?vox_read_shape_patches@@YAHHPAUCvrRecord@@H@Z 0x00639090-0x006391E6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 00639090..006391E6: preserve existing seed extent; pinned decode independently checks terminal return, dispatch callees and relevant field/callback behavior. Fresh VC6 output establishes current emitted symbol.", "Original638230 allocates44 bytes;638353 writes user_length at40. Palette storage is a pointer, parts/steps/shadows are typed allocations with callback-observable failure and release ordering.", "Patch639090 uses prefix+4 displacement,42-byte prefix and overlap-capable original6474E0 copy; standard memmove preserves the observed backward-copy behavior without undefined overlapping memcpy. Host controls are bounded and not full-game or malformed-file acceptance."], "reason": "Recover typed CVR record loading and nested chunk ownership in the genuine Caviar loader closure."}
int __cdecl vox_read_shape_patches(int fd, CvrRecord *record, int size) {
    uint32_t remaining = (uint32_t)size - 8u;
    // The format must select an index before its first patch. The original
    // leaves the index unspecified for a patch lacking that predecessor.
    int index;
    while (remaining != 0) {
        CvrChunkHeader header;
        if (vox_read_exact(fd, &header, sizeof(header)) != 0) return -1;
        remaining -= (uint32_t)header.length;
        if ((int32_t)remaining < 0) {
            vox_stage_error(vox_msg_bad_file_format);
            return -1;
        }
        const int payload = header.length - 8;
        switch (header.tag) {
        case 0x04050001:
            if (vox_read_exact(fd, &index, payload) != 0) return -1;
            break;
        case 0x04050002: {
            CvrShape *shape = record->steps_[index].shape_;
            // A 42-byte serialized prefix of a variable-size shape allocation,
            // not a declaration of a 42-byte complete object or array stride.
            // Only the serialized prefix (offsets 0..0x29) is read into it.
            CvrShape prefix;
            const uint32_t prefix_bytes = offsetof(CvrShape, item_stream_);
            if (vox_read_exact(fd, &prefix, prefix_bytes) != 0) return -1;
            // The new size first, read from the field (image: lea edx,[ecx+eax]).
            uint32_t new_size = shape->byte_size_ + prefix.byte_size_;
            uint32_t old_size = shape->byte_size_;
            unsigned char *bytes = reinterpret_cast<unsigned char *>(shape);
            // The original CRT target copies backward when these ranges
            // overlap; the patch prefix need not exceed the old shape size.
            memmove(bytes + prefix.byte_size_, bytes, old_size);
            memcpy(bytes, &prefix, prefix_bytes);
            if (vox_read_exact(fd, bytes + prefix_bytes, (uint32_t)payload - prefix_bytes) != 0) return -1;
            shape->byte_size_ = new_size;
            break;
        }
        default:
            if (vox_seek_slot((unsigned long)fd, payload, 1) < 0) {
                vox_stage_error(vox_msg_bad_file_format);
                return -1;
            }
            break;
        }
    }
    return 0;
}

// Old CVR palettes contain sixteen 256-byte shade rows after a two-byte
// prefix and RGB table. Expand them to twenty-four rows, in column-first
// order; failed reads leave the already-read prefix intact and no new rows.
// ORIGINAL: 0x006391F0 ?vox_expand_palette@@YAHHPAE@Z 0x006391F0-0x006392DC
// RECOVERY_BINDING: v1 {"evidence": ["Original6391F0..6392DC reads 1,1,768,4096 bytes in order, returns -1 on each read failure and expands 24 shade rows from source row floor(2*row/3) in column-first order. Existing seed provides compatible bounds; fresh VC6 emits the declared symbol."], "reason": "Recover old-format CVR palette expansion as genuine C++ in its loader owner, replacing the original-address forwarder."}
int __cdecl vox_expand_palette(int fd, uint8_t *palette) {
    uint8_t shades[0x1000];
    if (vox_read_exact(fd, palette, 1) != 0) return -1;
    if (vox_read_exact(fd, palette + 1, 1) != 0) return -1;
    if (vox_read_exact(fd, palette + 2, 0x300) != 0) return -1;
    if (vox_read_exact(fd, shades, sizeof(shades)) != 0) return -1;

    uint8_t *column_out = palette + 0x302;
    for (int column = 0; column < 0x100; ++column) {
        uint8_t *out = column_out++;
        for (int shade = 0; shade < 0x30; shade += 2) {
            *out = shades[(shade / 3) * 0x100 + column];
            out += 0x100;
        }
    }
    return 0;
}

/*
Purpose: Install the six vx_* file-IO callbacks and vet the machine's CPU rank.
         0 = accepted, 1 = rejected (init_class then shows CAVIAR_INVALIDCPU
         when the stored preference is also out of range).
// ORIGINAL: 0x006392E0 sub_6392e0 0x006392E0-0x0063937F FILE BYTE_EXACT
// symbol    ?vox_init_callbacks@@YADPBUVoxIoCallbacks@@H@Z
// The LEVER/TRIED measurements below are historical notes from the earlier
// integer-array/void-slot source model; this typed table supersedes that model.
// LEVER: the callback-table address must reach the memcpy intrinsic as a
//        LITERAL - a file-scope `void *const` variable compiles
//        `mov edi, dword ptr [var]` where the image folds `mov edi, 0x9c0b80`
//        (listing_diff, /O2 /GX). Macros, not const variables, for the slots.
// TRIED: `algo_flag == 0 || 5 < algo_flag` compiles `test al,al; je` against
//        the image's `test al,al; jbe` - SOLVED 2026-09-03: the test is the
//        RANGE form `CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag` on
//        the global directly (init_class's copy of the same check reads
//        `cmp al,bl; jbe`, also an unsigned `<= 0`). With the old
//        `const uint8_t algo_flag` local the front end folded `<= 0` back to
//        `== 0`; testing the global does not. Remaining after that: ONLY the
//        edi save placement (6 differing runs, all edi).
// TRIED: the image pushes edi in the save block (instruction 3, hoisted above
//        the first `jne` by scheduling) and pops it on all four return paths,
//        while this tree's memcpy intrinsic push/pops edi around its own
//        rep movsd - image 58 instructions, this tree 55. No spelling tried
//        made VC6 dedicate edi as a callee-saved register here: caching the
//        table pointer in a local (`void *table = vox_callback_table;`)
//        folds away and changes nothing.
// TRIED: at the frameless sets this body is 8 of 58 with only those edi runs
//        differing, against 1 of 58 at the /Oy- sets - but the /Oy- variant
//        scores 0.941 similar against 0.920 and `measure` picks it, so the
//        banked tier reads worse than the body is. Same pattern on 0x63AF60
//        (7 of 96 frameless, 0 of 96 /Oy-).
*/
char __cdecl vox_init_callbacks(const VoxIoCallbacks *callbacks, int flag) {
    if (callbacks == 0) {
        vox_stage_error(vox_msg_no_setup);
        return 1;
    }
    if (callbacks->allocate != 0 && callbacks->release != 0 &&
        callbacks->read != 0 && callbacks->write != 0 &&
        callbacks->seek != 0 && callbacks->tell != 0) {
        memcpy(&CaviarOutputMessageWindow, callbacks, sizeof(*callbacks));
        vox_detect_cpu();
        // The RANGE form, not `== 0`: the image branches `test al,al; jbe`
        // here and `cmp al,bl; jbe` in init_class - an unsigned `<= 0`
        // comparison, not an equality test. Spelled `== 0`, VC6 emits `je`
        // and the byte differs.
        if (CaviarVoxelAlgoFlag <= 0 || 5 < CaviarVoxelAlgoFlag) {
            vox_stage_error(vox_msg_cpu_unsupported);
            return 1;
        }
        vox_user_chunk = 0;
        vox_unk_9c0b78 = 0;
        return 0;
    }
    vox_stage_error(vox_msg_setup_empty);
    return 1;
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x006393C0 0x00639614 0x00639628 0x006395B5
/*
Purpose: Create the scene render record: allocate it, copy the colour and
         shadow tables for the descriptor's pixel size, fill the clip bounds
         and viewport extents, then run the per-CPU-rank init handler. The
         record pointer, or 0 on any failure.
// ORIGINAL: 0x006393C0 sub_6393c0 0x006393C0-0x00639612 FILE
// symbol    ?vox_create_record@@YAPAUVoxRenderRecord@@HPAX0PAUCaviarSceneSetupDescriptorCandidate@@H@Z
// NAMED RETURN 2026-08-30: the catalogue's K (unsigned long) decorates a
// pointer - the body allocates a VoxRenderRecord and every caller stores it
// as one; the return register carries the same dword either way.
// LEVER: the alloc/free slots must be MACROS on the raw addresses: a const
//        pointer variable compiles `mov eax,[var]; call dword ptr [eax]`
//        where the image calls `call dword ptr [0x9c0b80]` straight through
//        the slot (listing_diff, /O2 /GX).
// LEVER: the pixel-size dispatch is a switch with grouped cases - case 1/2
//        and 3/5 reproduce the image's dec/je/dec/je chain (an if/else-if
//        chain compiles cmp/jne; 0x0063AE20's marker says the same), and the
//        algo switch's five-entry jump table with one range check falls out
//        of the same spelling on *CaviarVoxelAlgoFlag.
// TRIED: the image keeps `xor ebp, ebp` live for EVERY null test and zero
//        store (cmp reg,ebp / mov [ebx+N],ebp); every spelling tried emits
//        test/imm forms instead - the persistent zero register is the main
//        remaining divergence and no source shape reached it.
// TRIED: per-case constant memcpys duplicate the shadow-table rep movsd the
//        image merges across the two pixel sizes (`mov ecx,0x8000; jmp L;
//        mov ecx,0x40; L: mov edi,eax; rep movsd`); a merged variable-size
//        memcpy would call the CRT instead - neither matches.
// TRIED: pixel_size as uint8_t and as unsigned int; the uint8_t spelling
//        reads the byte straight into al, the image widens through edx
//        (xor edx,edx / mov dl,[eax+0x20] / mov eax,edx).
*/
VoxRenderRecord *__cdecl vox_create_record(
    int setup_id, void *setup_data, void *shadow_data,
    CaviarSceneSetupDescriptorCandidate *setup_block, int size_code) {
    if (setup_data == 0) {
        vox_stage_error(vox_msg_no_colortab);
        return 0;
    }
    VoxRenderRecord *record = (VoxRenderRecord *)vox_alloc_slot(0x80);
    if (record == 0) {
        vox_stage_error(vox_msg_no_handler);
        return 0;
    }
    memset(record, 0, sizeof(VoxRenderRecord));
    record->setup_id_ = setup_id;
    record->setup_block_ = setup_block;
    record->setup_size_code_ = (uint8_t)size_code;

    // unsigned int, not uint8_t: the image zeroes a register and reads the
    // byte into its low half (xor edx,edx / mov dl,[eax+0x20]) before the
    // dispatch, which is the widening of a 32-bit local.
    const unsigned int pixel_size = setup_block->pixel_size_;
    switch (pixel_size) {
    case 1:
        record->colour_table_ = (void *)vox_alloc_slot(0x1800);
        if (record->colour_table_ == 0) {
            vox_stage_error(vox_msg_no_colour);
            return 0;
        }
        memcpy(record->colour_table_, setup_data, 0x1800);
        if (shadow_data != 0) {
            record->shadow_table_ = (void *)vox_alloc_slot(0x100);
            if (record->shadow_table_ == 0) {
                vox_stage_error(vox_msg_no_shadow);
                vox_free_slot(record->colour_table_);
                return 0;
            }
            memcpy(record->shadow_table_, shadow_data, 0x100);
        }
        break;
    case 2:
        record->colour_table_ = (void *)vox_alloc_slot(0x3000);
        if (record->colour_table_ == 0) {
            vox_stage_error(vox_msg_no_colour);
            return 0;
        }
        memcpy(record->colour_table_, setup_data, 0x3000);
        if (shadow_data != 0) {
            record->shadow_table_ = (void *)vox_alloc_slot(0x20000);
            if (record->shadow_table_ == 0) {
                vox_stage_error(vox_msg_no_shadow);
                vox_free_slot(record->colour_table_);
                return 0;
            }
            memcpy(record->shadow_table_, shadow_data, 0x20000);
        }
        break;
    default:
        vox_stage_error(vox_msg_unsupported_pixel_slot);
        return 0;
    }

    // Original bound setup sign-extends each low word. The descriptor's
    // full dwords still control the later row-table allocation path.
    record->draw_right_ =
        static_cast<int>(static_cast<int16_t>(static_cast<uint16_t>(
            setup_block->colour_width_))) - 1;
    const int height_minus_1 =
        static_cast<int>(static_cast<int16_t>(static_cast<uint16_t>(
            setup_block->height_))) - 1;
    record->draw_bottom_ = height_minus_1;
    if (record->draw_right_ >= 0 && height_minus_1 >= 0) {
        record->draw_width_ = record->draw_right_ + 1;
        record->draw_height_ = height_minus_1 + 1;
    } else {
        record->draw_width_ = 0;
        record->draw_height_ = 0;
    }

    switch (CaviarVoxelAlgoFlag) {
    case 1:
    case 2:
    case 4:
        sub_63ad60(record);
        break;
    case 3:
    case 5:
        sub_63ae20(record);
        break;
    default:
        vox_stage_error(vox_msg_cpu_unsupported);
        return 0;
    }
    record->ramp_a_ptr_ = 0;
    record->ramp_b_ptr_ = 0;
    if (setup_block->colour_width_ != 0 && setup_block->height_ != 0 &&
        sub_63f9b0(record) != 0) {
        return 0;
    }
    vox_record_count += 1;
    return record;
}
