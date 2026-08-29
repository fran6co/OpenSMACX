/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "caviar.h"
#include "basepop.h"
#include "buffer.h"
#include "general.h"    // mem_get
#include "stringstruct.h"
#include "strings.h"    // StringTemp

#include <new>

namespace {

OPENSMACX_NOINLINE
uint32_t load_scaling_bits(const void *source) {
    uint32_t value;
    memcpy(&value, source, sizeof(value));
    return value;
}

} // namespace

/*
Purpose: Initialize an empty Caviar object-data record.
// ORIGINAL: 0x00616BC0 ??0CaviarData@@QAE@XZ 0x00616BC0-0x00616BCD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ??0CaviarData@@QAE@XZ)(CaviarData* this)
// callers   82   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
// body src/caviar.h

/*
Purpose: Release the record this data slot owns. A slot with no record is left
         entirely untouched; otherwise the helper walks the record and frees
         its members, and the slot forgets it. The two leading fields keep
         whatever they held either way.
// ORIGINAL: 0x00616C60 ?close@CaviarData@@QAEXXZ 0x00616C60-0x00616C7C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?close@CaviarData@@QAEXXZ)(CaviarData* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638430
Return Value: n/a
Status: Complete
*/
void CaviarData::close() {
    if (record_) {
        caviar_free_record(record_);
        record_ = nullptr;
    }
}

/*
Purpose: Initialize the Caviar renderer's object records and default scalar fields.
// ORIGINAL: 0x00616DA0 ??0Caviar@@QAE@XZ 0x00616DA0-0x00616DD6 SEMANTIC
// LEVER: 0 of 16 -> SHAPE_EXACT 12 of 16, on two changes that are one idea. The image never memsets `slots_`: it runs a 200-turn loop writing three zero dwords per element, which is `CaviarData::CaviarData()` FOLDED into the implicit array construction. This body was doing both - the compiler emitted a 200-turn loop of `call ?CaviarData::CaviarData` because that constructor was out-of-line in caviar.cpp, AND the source then memset the same storage with `rep stosd`, 25 instructions against the image's 16. Moving the constructor in-class in caviar.h (`MEASURED`, so 0x00616BC0 keeps its own COMDAT and stays BYTE_EXACT) lets VC6 inline it, and deleting the memset leaves exactly the image's loop.
// TRIED: the residual 4 instructions are a pointer bias inside VC6's array-construction loop and no source form here reaches it. The image walks `[eax + 0x750]` and stores at `[edx - 4]`, `[edx]`, `[edx + 4]`; this tree walks `[eax + 0x74c]` and stores at `[edx]`, `[edx + 4]`, `[edx + 8]`. Same element, same order, same encoded length - only the base is biased by 4. listing_diff reports 0 differing runs, which is why this is SHAPE_EXACT rather than a structural mismatch.
// size      54 bytes
// prototype void (__thiscall ??0Caviar@@QAE@XZ)(Caviar* this)
// callers   7   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
Caviar::Caviar() {
    uint8_t *bytes = reinterpret_cast<uint8_t *>(this);
    const uint32_t distance_limit = 0x461C4000;
    const uint32_t default_scene_scale = 0x3F2AAAAB;
    const uint32_t zero = 0;
    memcpy(bytes + 0x108, &distance_limit, sizeof(distance_limit));
    memcpy(bytes + 0x13CC, &zero, sizeof(zero));
    memcpy(&scene_scale_, &default_scene_scale, sizeof(scene_scale_));
}

/*
Purpose: Copy a camera position and orientation directly into the renderer state.
// ORIGINAL: 0x006182A0 ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z 0x006182A0-0x0061831F BYTE_EXACT
// LEVER: TWELVE ELEMENT COPIES, no struct assignment anywhere. Every store in the image is `mov [ecx + disp32], reg` off the incoming `this` - 0xA5, 0xA9, 0xAD for the vector and 0xB1 through 0xD1 for the matrix - so nothing ever takes the address of a subobject. `camera_.position = *camera` (12 bytes) forms `lea edx, [ecx + 0xa5]` and indexes off it, and `camera_.rotation = *matrix` (36 bytes) is over VC6's inline-copy threshold and becomes `rep movsd`, which also costs the EDI/ESI pair and moves the two null tests behind the pushes: 21 compiled instructions against 33, agreeing 0. Assigning the twelve floats one at a time gives 33 of 33, BYTE_EXACT.
// TRIED: MEASURED, all with the same 5 of 33 - three 12-byte `memcpy`s for the matrix rows (42 compiled), a `for (row)` loop over three row memcpys (34), splitting `camera && matrix` into two guard clauses (42), and keeping the struct assignment for `position` while spelling the nine matrix floats out (36). It is the struct assignment that costs it, not the copy width and not the chained condition - the image really does test both and share one `ret 8`.
// TRIED: `record` DROPPED THIS LINE the first time. Re-recording a body whose tier changes rewrites its annotation from the parsed state and keeps only the LEVER; the TRIED text is discarded silently. Re-added after the claim was banked, and verified with `osmx show` rather than by reading the diff.
// symbol    ?set_camera_direct@Caviar@@QAEXPBUVOX_Vect@@PBUVOX_Matrix@@@Z
// size      127 bytes
// prototype void (__thiscall ?set_camera_direct@Caviar@@QAEXPAUVOX_Vect@@PAUVOX_Matrix@@@Z)(Caviar* this, VOX_Vect*, VOX_Matrix*)
// callers   10   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Caviar::set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix) {
    if (camera && matrix) {
        camera_.position.values[0] = camera->values[0];
        camera_.position.values[1] = camera->values[1];
        camera_.position.values[2] = camera->values[2];
        camera_.rotation.values[0][0] = matrix->values[0][0];
        camera_.rotation.values[0][1] = matrix->values[0][1];
        camera_.rotation.values[0][2] = matrix->values[0][2];
        camera_.rotation.values[1][0] = matrix->values[1][0];
        camera_.rotation.values[1][1] = matrix->values[1][1];
        camera_.rotation.values[1][2] = matrix->values[1][2];
        camera_.rotation.values[2][0] = matrix->values[2][0];
        camera_.rotation.values[2][1] = matrix->values[2][1];
        camera_.rotation.values[2][2] = matrix->values[2][2];
    }
}

/*
Purpose: Set the Caviar renderer scaling factor.
// ORIGINAL: 0x006183B0 ?set_scaling@Caviar@@QAEXM@Z 0x006183B0-0x006183BD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?set_scaling@Caviar@@QAEXM@Z)(Caviar* this, float)
// callers   24   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Caviar::set_scaling(float scaling) {
    memcpy(&camera_.scaling, &scaling, sizeof(scaling));
}

void Caviar::set_scaling_bits(uint32_t scaling_bits) {
    memcpy(&camera_.scaling, &scaling_bits, sizeof(scaling_bits));
}

/*
Purpose: Read the Caviar renderer scaling factor.
// ORIGINAL: 0x006183C0 ?get_scaling@Caviar@@QAEMXZ 0x006183C0-0x006183C7 BYTE_EXACT
// size      7 bytes
// prototype float (__thiscall ?get_scaling@Caviar@@QAEMXZ)(Caviar* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Current scaling factor
Status: Complete
*/
float Caviar::get_scaling() {
    return camera_.scaling;
}







/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x006176C0 ?UNK3@Caviar@@QAEHHHH@Z 0x006176C0-0x006176C5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK3@Caviar@@QAEHHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Caviar::UNK3(int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x006176D0 ?UNK4@Caviar@@QAEHHHHH@Z 0x006176D0-0x006176D5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@Caviar@@QAEHHHHH@Z)(Caviar* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Caviar::UNK4(int, int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618250 ?UNK5@Caviar@@QAEXHH@Z 0x00618250-0x00618253 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK5@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK5(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618260 ?UNK6@Caviar@@QAEXHH@Z 0x00618260-0x00618263 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK6(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618270 ?UNK7@Caviar@@QAEXHH@Z 0x00618270-0x00618273 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK7@Caviar@@QAEXHH@Z)(Caviar* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK7(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00618290 ?UNK9@Caviar@@QAEXHHH@Z 0x00618290-0x00618293 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK9@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK9(int, int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x006183A0 ?UNK12@Caviar@@QAEXHHH@Z 0x006183A0-0x006183A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK12@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK12(int, int, int) {
}


/*
Purpose: Store a single value at 0x108.
// ORIGINAL: 0x00618280 ?UNK8@Caviar@@QAEXH@Z 0x00618280-0x0061828D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?UNK8@Caviar@@QAEXH@Z)(Caviar* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK8(int menu_id) {
    field_108_ = menu_id;
}

/*
Purpose: Store three values into the triplet at 0x2C.
// ORIGINAL: 0x00618320 ?UNK10@Caviar@@QAEXHHH@Z 0x00618320-0x00618338 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?UNK10@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK10(int a1, int a2, int a3) {
    field_2C_ = a1;
    field_30_ = a2;
    field_34_ = a3;
}



/*
Purpose: Set the scene's rotation, handing the three angles and the matrix at
         0x38 to the helper that applies them.
// ORIGINAL: 0x00618370 ?set_scene_rotation@Caviar@@QAEXMMM@Z 0x00618370-0x0061839F BYTE_EXACT
// size      47 bytes
// prototype void (__thiscall ?set_scene_rotation@Caviar@@QAEXMMM@Z)(Caviar* this, float, float, float)
// callers   13   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00627D00
Return Value: n/a
Status: Complete
*/
void Caviar::set_scene_rotation(float x, float y, float z) {
    float angles[3] = {x, y, z};
    caviar_apply_rotation(angles, reinterpret_cast<uint8_t *>(this) + 0x38);
}


/*
Purpose: Read back the three values at 0x2C, 0x30 and 0x34, skipping any output
         whose pointer is null.

             test eax,eax / je next / mov edx,[ecx+0x2C] / mov [eax],edx

         repeated per output. Each null check guards only its OWN store, so a
         caller passing the middle pointer null still receives the other two -
         which is what the three independent branches encode and what a single
         guard around all three would get wrong.

         The mangled name declares three ints; they are used as pointers, and
         the parameters are typed that way here because that is what the body
         does with them.
// ORIGINAL: 0x00618340 ?UNK11@Caviar@@QAEXHHH@Z 0x00618340-0x0061836A BYTE_EXACT
// symbol    ?UNK11@Caviar@@QAEXPAH00@Z
// size      42 bytes
// prototype void (__thiscall ?UNK11@Caviar@@QAEXHHH@Z)(Caviar* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Caviar::UNK11(int *out1, int *out2, int *out3) {
    if (out1) {
        *out1 = field_2C_;
    }
    if (out2) {
        *out2 = field_30_;
    }
    if (out3) {
        *out3 = field_34_;
    }
}


/*
Purpose: Release every allocation the renderer holds and reset the scene.
// ORIGINAL: 0x00617020 ?close@Caviar@@QAEXXZ 0x00617020-0x0061707F BYTE_EXACT
// size      95 bytes
// prototype void (__thiscall ?close@Caviar@@QAEXXZ)(Caviar* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
//
// PROMOTED from src/recovered/00617020.cpp, which was byte-exact but not
// compiled - so it proved the artifact and not this tree, and `class Caviar`
// did not even declare `close`. Its body reached the slots as
// `reinterpret_cast<char *>(this) + 0x750` walked by a raw `record += 0xc`;
// `slots_` says the same thing in the type, and see the note on it in the
// header for why the arithmetic reconciles.
//
// A DOWN-COUNTING do/while OVER A WALKING POINTER, because that is what the
// image emits: it never materialises an index, and the loop cannot run zero
// times.
Return Value: n/a
Status: Complete
*/
void Caviar::close() {
    // THE CURSOR AIMS AT THE MIDDLE MEMBER. The image computes
    // `lea esi, [ebp + 0x750]` - `fileDescriptor_` of slot zero, not
    // `field_0_` at 0x74C - and reaches the other two as `[esi - 4]` and
    // `[esi + 4]`. Walking from `slots_` bases at 0x74C instead and reads
    // `[esi]`, `[esi+4]`, `[esi+8]`: the same three words, a different
    // instruction at every site.
    void **cursor = &slots_->fileDescriptor_;
    int count = 0xc8;
    do {
        if (cursor[-1] != nullptr) {
            free(cursor[-1]);
            cursor[-1] = nullptr;
        }
        if (cursor[0] != nullptr) {
            free(cursor[0]);
            cursor[0] = nullptr;
        }
        if (cursor[1] != nullptr) {
            free(cursor[1]);
            cursor[1] = nullptr;
        }
        cursor += 3;
    } while (--count);

    field_4_ = 0;
    scene_scale_ = 2.0f / 3.0f;
}

// ====================
Caviar g_VEHDRAW_CAVIAR;
Buffer g_CAVIAR_BUFFER_1;
Buffer g_CAVIAR_BUFFER_2;

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
CaviarData g_UNUSED_CAVIARDATA_VAR1;  // 0x00791D68
CaviarData g_SSF_CAVIARDATA;  // 0x007ACCA0
CaviarData g_SDP_CAVIARDATA;  // 0x0079A6A0
CaviarData g_SAS_CAVIARDATA;  // 0x00787E48
CaviarData g_SCD_CAVIARDATA;  // 0x00799228
CaviarData g_SCJ_CAVIARDATA;  // 0x007ACC90
CaviarData g_SAGS_CAVIARDATA;  // 0x0077A578
CaviarData g_SFT_CAVIARDATA;  // 0x0079A6B0
CaviarData g_VHR_CAVIARDATA1;  // 0x007765B0
CaviarData g_SHT_CAVIARDATA;  // 0x0078A5E0
CaviarData g_SRB_CAVIARDATA;  // 0x0078A300
CaviarData g_ASAS_CAVIARDATA;  // 0x007AC030
CaviarData g_RESLASER_CAVIARDATA;  // 0x007ACCC0
CaviarData g_RESBOLT_CAVIARDATA;  // 0x007A7900
CaviarData g_FUNLOAD_CAVIARDATA;  // 0x007ACC10
CaviarData g_TECLOAD_CAVIARDATA;  // 0x00788028
CaviarData g_SP_DISSWAVE_CAVIARDATA;  // 0x00799E38
CaviarData g_SP_MARINED_CAVIARDATA;  // 0x007792A0
CaviarData g_SP_NANOO_CAVIARDATA;  // 0x0075AEF0
CaviarData g_SP_SOPORIFIC_CAVIARDATA;  // 0x007AD130
CaviarData g_AA01_CAVIARDATA;  // 0x007AD040
CaviarData g_AA_ROVER_CAVIARDATA;  // 0x0075AFC0
CaviarData g_AA_WING_CAVIARDATA[2];  // 0x00779288, 0xc stride
CaviarData g_ACP_CAVIARDATA[4];  // 0x0078A610, 0xc stride
CaviarData g_AX_CAVIARDATA;  // 0x0079A6F0
CaviarData g_AA_CAVIARDATA;  // 0x00789A50
CaviarData g_ACOLPOD_CAVIARDATA;  // 0x0078A600
CaviarData g_AT_CAVIARDATA;  // 0x00787E60
CaviarData g_VTA_CAVIARDATA;  // 0x007880C0
CaviarData g_ATP_CAVIARDATA;  // 0x007ABFC0
CaviarData g_SSFA_CAVIARDATA;  // 0x0078A510
CaviarData g_SFTA_CAVIARDATA;  // 0x0079A640
CaviarData g_VW00_CAVIARDATA;  // 0x007871C0
CaviarData g_VIPTAWL_CAVIARDATA;  // 0x0079A700
CaviarData g_VIPTASGN_CAVIARDATA;  // 0x007AB3F0
CaviarData g_VIPTAPSI_CAVIARDATA;  // 0x007795A0
CaviarData g_VIPTR_CAVIARDATA[4];  // 0x00799628, 0xc stride
CaviarData g_VIPTA_CAVIARDATA[2];  // 0x007791D0, 0xc stride
CaviarData g_PTMOD_CAVIARDATA;  // 0x0078A500
CaviarData g_VB_CAVIARDATA;  // 0x00779230
CaviarData g_VBP_CAVIARDATA;  // 0x0078E968
CaviarData g_VGMC_CAVIARDATA;  // 0x0075AED0
CaviarData g_VGMCP_CAVIARDATA;  // 0x0076EBE0
CaviarData g_VLIGHTS_CAVIARDATA;  // 0x0075B848
CaviarData g_VPT_CAVIARDATA;  // 0x007592D0
CaviarData g_A_CAVIARDATA;  // 0x0075AF00
CaviarData g_APWALL_CAVIARDATA;  // 0x0078A490
CaviarData g_ASGEN_CAVIARDATA;  // 0x007AD120
CaviarData g_APSID_CAVIARDATA;  // 0x007ACCB0
CaviarData g_VA01_CAVIARDATA;  // 0x007AD0E0
CaviarData g_VR_CAVIARDATA[4];  // 0x0076E560, 0xc stride
CaviarData g_VHR_CAVIARDATA2;  // 0x00779560
CaviarData g_VRC_CAVIARDATA[4];  // 0x0075ADB8, 0xc stride
CaviarData g_VI_CAVIARDATA;  // 0x0078B740
CaviarData g_VGMT_CAVIARDATA;  // 0x0077A5B8
CaviarData g_VGMTP_CAVIARDATA;  // 0x0078B720
CaviarData g_UNUSED_CAVIARDATA_VAR2;  // 0x007ACF90
CaviarData g_VWNTU_CAVIARDATA;  // 0x007796A0
CaviarData g_VT_CAVIARDATA;  // 0x0075AD78
CaviarData g_DROP_CAVIARDATA;  // 0x007ACD60
CaviarData g_DROPLET_CAVIARDATA;  // 0x0076EBD0
CaviarData g_VCL_CAVIARDATA;  // 0x007AD170
CaviarData g_UNUSED_CAVIARDATA_VAR3[2];  // 0x007795F0, 0xc stride
CaviarData g_VCLT00_CAVIARDATA;  // 0x00789B58
CaviarData g_VHT_VBP_CAVIARDATA;  // 0x00779608
CaviarData g_VHTP_CAVIARDATA;  // 0x0078A640
CaviarData g_VHTA0_CAVIARDATA[2];  // 0x00779648, 0xc stride
CaviarData g_VHTTP_CAVIARDATA;  // 0x00759310
CaviarData g_VHTTPA0_CAVIARDATA[3];  // 0x0078B750, 0xc stride
CaviarData g_VSP_CAVIARDATA;  // 0x007A6820
CaviarData g_VSPA0_CAVIARDATA[2];  // 0x00779270, 0xc stride
CaviarData g_VSPTF_CAVIARDATA;  // 0x007A7290
CaviarData g_VSPTB_CAVIARDATA;  // 0x007A7280
CaviarData g_VFL_CAVIARDATA;  // 0x007ACC50
CaviarData g_VGS_CAVIARDATA;  // 0x007A7240
CaviarData g_VGSP_CAVIARDATA;  // 0x00798BF0
CaviarData g_VJTP_CAVIARDATA;  // 0x007795B0
CaviarData g_VJT0_CAVIARDATA[2];  // 0x0078A2B8, 0xc stride
CaviarData g_VCU_CAVIARDATA;  // 0x0075AEE0
CaviarData g_VCUP_CAVIARDATA;  // 0x0075B440
CaviarData g_VCUA0_CAVIARDATA[2];  // 0x0078CAB8, 0xc stride
CaviarData g_VCUW_CAVIARDATA;  // 0x0075B088
CaviarData g_VCT_CAVIARDATA;  // 0x0079A680
CaviarData g_VCTP_CAVIARDATA;  // 0x007ACFA0
CaviarData g_VCTB_CAVIARDATA;  // 0x0076E890
CaviarData g_VCT0_CAVIARDATA[2];  // 0x0079A628, 0xc stride
CaviarData g_VWNTT_CAVIARDATA;  // 0x0078B730
CaviarData g_VWNST_CAVIARDATA;  // 0x0078DE20
CaviarData g_VWNAA_CAVIARDATA;  // 0x007AC1A0
CaviarData g_VW_CAVIARDATA[16];  // 0x00787100, 0xc stride
CaviarData g_VM_CAVIARDATA;  // 0x00791D78
CaviarData g_VM13_CAVIARDATA;  // 0x0078A5F0
CaviarData g_VPBR0_CAVIARDATA[4];  // 0x0076E9B0, 0xc stride
CaviarData g_UNUSED_CAVIARDATA_VAR4[2];  // 0x007791E8, 0xc stride
CaviarData g_NW_CAVIARDATA;  // 0x0079A690
CaviarData g_NI_CAVIARDATA;  // 0x007791C0
CaviarData g_NLC_CAVIARDATA;  // 0x00779660
CaviarData g_UNUSED_CAVIARDATA_VAR5[5];  // 0x00787E08, 0xc stride

// ---------------------------------------------------------------------------
// The voxel renderer's workspace, set up once by init_class below. Every
// binding is terranx data the body reads or writes through a fixed address;
// they are file-local because nothing else in the tree reaches them yet.
// ---------------------------------------------------------------------------

namespace {

// 0x009B9100. The 0x20000-byte colour table mem_get allocates here, filled
// with 0xffff, copied into the render record and walked into its two ramps.
int *const CaviarSceneMemory = (int *)0x009B9100;

// 0x009B9108 and 0x009B96B0. The two 256x256 scene surfaces this function
// creates (init) and clears (fill 9 / fill 0).
Buffer *const CaviarSceneBufferA = (Buffer *)0x009B9108;
Buffer *const CaviarSceneBufferB = (Buffer *)0x009B96B0;

// BufferA + 0x54 and BufferB + 0x54: each surface's bits pointer, copied
// into the render record's descriptor block below.
int *const CaviarSceneBufferABits = (int *)0x009B915C;
int *const CaviarSceneBufferBBits = (int *)0x009B9704;

// 0x009B95B0 and 0x009B9B58: two slots read verbatim into the render record;
// 0x009B9C38: passed twice to vox_create_record as the record's name.
int *const CaviarRecordBitsSource = (int *)0x009B95B0;
int *const CaviarRecordOwnerSource = (int *)0x009B9B58;
void *const CaviarRecordName = (void *)0x009B9C38;

// 0x009B9690..0x009B969C: four slots zeroed once the record is built. The
// image clears them in the order 0, 2, 3, 1 - not slot order.
int *const CaviarViewportState = (int *)0x009B9690;

// ForceOldVoxelAlgorithm, the PREFERENCES byte the ini read seeds.
uint8_t *const CaviarVoxelAlgoFlag = (uint8_t *)0x009C0830;

// The scene render record this function fills and hands to vox_create_record;
// the created handle lands at the tail (+0x40, 0x009BB478).
struct CaviarSceneRecord {
  uint32_t field_00_;      // 0x9BB438, zeroed on success
  uint32_t field_04_;      // 0x9BB43C, zeroed on success
  uint32_t field_08_;      // 0x9BB440, zeroed on success
  uint32_t field_0C_;      // 0x9BB444, zeroed on success
  void *buffer_a_bits_;    // 0x9BB448
  void *scene_memory_;     // 0x9BB44C
  void *buffer_b_bits_;    // 0x9BB450
  uint32_t width_a_;       // 0x9BB454
  uint32_t height_a_;      // 0x9BB458
  void *record_bits_;      // 0x9BB45C
  uint32_t width_b_;       // 0x9BB460
  void *record_owner_;     // 0x9BB464
  uint8_t record_flag_;    // 0x9BB468
  uint8_t pad_[0x40 - 0x31];
  void *created_;          // 0x9BB478
};

CaviarSceneRecord *const CaviarScene = (CaviarSceneRecord *)0x009BB438;

} // namespace

/*
Purpose: Boot the CAVIAR voxel renderer. Hands the vx_* file-IO callbacks to
         the engine and, when the machine's CPU report needs showing, opens a
         popup over BasePop's infrastructure and flattens its entry list into
         StringTemp. Then reads the ForceOldVoxelAlgorithm preference, creates
         the two 256x256 scene surfaces plus the shared 0x20000-entry colour
         table, and builds the render record from them. Nonzero returns make
         WinMain abort the boot; 0x17 is the render-record failure.
// ORIGINAL: 0x006185A0 ?init_class@Caviar@@QAAHXZ 0x006185A0-0x00618D16;0x00663150-0x006632F1
// symbol    ?init_class@Caviar@@SAHXZ
//           (static member: WinMain's `call 0x6185a0` sets up no ecx, so a
//           plain member would C2352 - the same rule as palette.h's
//           set_active_window, which is why the emitted name spells S not Q)
// TRIED: explicit __try/__except(EXCEPTION_EXECUTE_HANDLER) returning 0x17
//        around the post-construction body - C2712, VC6 refuses __try in a
//        function that requires object unwinding, and pop needs it. Measured
//        2026-08-29; reverted. The image's own shape is out of C++ reach: a
//        byte scan of .text and SELFMOD finds exactly ONE executable branch
//        into the teardown block at 0x618763 (the `jmp 0x618873` tail of the
//        block above it), so that block is referenced ONLY by the scope
//        table at 0x6632e7 - it IS an __except handler - and the image's
//        per-path teardown chains (state variable [esp+0x325c] walking
//        9..1, 0x12..0xa, 0x1b..0x13, 0x24..0x1c) are its unwind actions.
//        Those chains destroy pop's MEMBERS directly - 0x406820 (StringStruct
//        dtor) at +0x2150/+0x2180, 0x4066c0 on the second one's virtual base
//        at +0x21a8, 0x406910+0x608e10+0x5d4dd0 on the Dialogs slab at
//        +0x21d0 - where the out-of-line ??1BasePop (0x4064D0) spells the
//        same teardown as remove_all (0x402970) calls. This body's returns
//        all run ??1BasePop instead, which is the same work at runtime and
//        not reproducible from any C++ spelling that also links.
// TRIED: int return on vox_init_callbacks tested `test eax, eax` against the
//        image's `test al, al`; the forwarder now returns char and the test
//        agrees (18 -> 19 of 392 at /O2 /GX).
// TRIED: holding the popup StringList in a named local versus re-deriving
//        `&pop + 0x2180` at every use - identical scores (19/392, 195
//        compiled). The image's prologue pushes one register fewer than
//        either spelling; kept the named local for readability.
// TRIED: the image INLINES seek_pos(-1), next_entry and current_entry of the
//        popup's StringList at +0x2180 (~60 of the ~200 remaining divergent
//        instructions); those are BYTE_EXACT out-of-line members in
//        stringstruct.cpp, so __forceinline-ing them for this one site would
//        regress their own claims and every other call site. Ceiling noted,
//        not chased.
Return Value: 0 on success, 4 when a scene surface or the colour table could
              not be created, 0x17 when the render record could not.
Status: REWRITTEN under RAII - the body is faithful; the notes above carry
        what measurement showed about the remaining divergence.
*/
int Caviar::init_class() {
    // The popup infrastructure, its heap, buttons and sprite. Its real
    // constructor (0x00600860, in basepop.cpp) builds every subobject the
    // image's teardown paths name; the destructor (0x004064D0, still a
    // pending forwarder) reverses all of it, so a plain local under RAII
    // replays each early exit's cleanup - and its nontrivial destructor is
    // what makes VC6 emit the image's fs:[0] unwind frame for this function.
    BasePop pop;

    // The six vx_* file-IO callbacks the engine is handed: vx_malloc,
    // vx_free, vx_read, vx_write, vx_seek, vx_tell (0x00618E10..0x00618E90,
    // mapped in recovered/units). Raw addresses because none of the six is
    // source-owned yet.
    unsigned long vx_callbacks[6] = {
        0x00618E10, 0x00618E20, 0x00618E30, 0x00618E50, 0x00618E70,
        0x00618E90};

    if (vox_init_callbacks(vx_callbacks, 1) != 0) {
        // The engine sized up this machine's CPU. Without a stored
        // preference - or with one above the known range - tell the player
        // this is not a CPU the renderer supports, by listing the popup's
        // entries as one space-separated line in StringTemp.
        const uint8_t algo_flag = *CaviarVoxelAlgoFlag;
        if (algo_flag == 0 || 5 < algo_flag) {
            pop.start("jackal", "CAVIAR_INVALIDCPU", -1, 0, 0, 0);

            // The popup's unit-name list at +0x2180, which start() filled.
            // NOT carved into BasePop, for the same reason its header gives
            // for the CheckBox at +0x2228: StringList derives a virtual
            // base, so a declared member would give BasePop's constructor
            // implicit stores the image's does not have. The offset spelling
            // is the one the class already owns (check_box()).
            StringList *const unit_ids = reinterpret_cast<StringList *>(
                reinterpret_cast<uint8_t *>(&pop) + 0x2180);
            StringTemp[0] = 0;
            unit_ids->seek_pos(-1);
            for (int i = 0; i < unit_ids->entry_count_; ++i) {
                unit_ids->next_entry();
                const char *entry_text = 0;
                if (unit_ids->current_ != 0) {
                    entry_text = reinterpret_cast<const char *const *>(
                        unit_ids->current_entry())[1];
                }
                strcat(StringTemp, entry_text);
                strcat(StringTemp, " ");
            }
        }
    }

    if (GetPrivateProfileIntA("PREFERENCES", "ForceOldVoxelAlgorithm", 0,
                              ".\\Alpha Centauri.ini") != 0) {
        *CaviarVoxelAlgoFlag = 1;
    } else {
        WritePrivateProfileStringA("PREFERENCES", "ForceOldVoxelAlgorithm",
                                   "0", ".\\Alpha Centauri.ini");
    }

    if (CaviarSceneBufferA->init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }
    if (CaviarSceneBufferB->init(0x100, 0x100, 0, 0) != 0) {
        return 4;
    }

    *CaviarSceneMemory = reinterpret_cast<int>(mem_get(0x20000));
    if (*CaviarSceneMemory == 0) {
        return 4;
    }

    CaviarSceneBufferA->fill(9);
    CaviarSceneBufferB->fill(0);
    vox_fill_colour_table((void *)*CaviarSceneMemory, 0xffff, 0x10000);

    // The descriptor block, in the image's store order.
    CaviarScene->scene_memory_ = (void *)*CaviarSceneMemory;
    CaviarScene->record_bits_ = (void *)*CaviarRecordBitsSource;
    CaviarScene->buffer_a_bits_ = (void *)*CaviarSceneBufferABits;
    CaviarScene->width_a_ = 0x100;
    CaviarScene->height_a_ = 0x100;
    CaviarScene->record_flag_ = 1;
    CaviarScene->buffer_b_bits_ = (void *)*CaviarSceneBufferBBits;
    CaviarScene->record_owner_ = (void *)*CaviarRecordOwnerSource;
    CaviarScene->width_b_ = 0x100;

    CaviarScene->created_ = (void *)vox_create_record(
        0, CaviarRecordName, CaviarRecordName,
        (void *)&CaviarScene->buffer_a_bits_, 0x80);
    if (CaviarScene->created_ == 0) {
        return 0x17;
    }

    unsigned long *const created =
        (unsigned long *)CaviarScene->created_;
    created[0x58 / 4] = 0x00618DA0;  // &Caviar::object_start, still an artifact
    created[0x38 / 4] = 0;
    created[0x3C / 4] = 0;
    created[0x40 / 4] = 0xff;
    created[0x44 / 4] = 0xff;
    created[0x48 / 4] = 0x100;
    created[0x4C / 4] = 0x100;

    // The record's two 256-entry ramps: table A gets the colour table's own
    // base stepped 0x100 per entry, table B a 0x100-stepped ramp from zero.
    unsigned long base = (unsigned long)*CaviarSceneMemory;
    unsigned long shade = 0;
    unsigned long *const ramp_a = (unsigned long *)created[0x60 / 4];
    unsigned long *const ramp_b = (unsigned long *)created[0x64 / 4];
    for (int index = 0; index < 0x100; ++index) {
        ramp_a[index] = base;
        base += 0x100;
        ramp_b[index] = shade;
        shade += 0x100;
    }

    CaviarViewportState[0] = 0;
    CaviarViewportState[2] = 0;
    CaviarViewportState[3] = 0;
    CaviarViewportState[1] = 0;
    CaviarScene->field_00_ = 0;
    CaviarScene->field_08_ = 0;
    CaviarScene->field_0C_ = 0;
    CaviarScene->field_04_ = 0;
    return 0;
}
