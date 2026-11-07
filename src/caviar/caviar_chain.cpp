#include "stdafx.h"
#include "caviar.h"

/*
Purpose: Walk a node chain and return the node the walk stops at. Each node
         is {uint8_t flags; ...; float extent; int next_offset}: bit 0 of the
         flags byte says a next node exists, +8 is the extent and +0xc the
         byte offset to it. The walk continues while the extent, scaled by
         the caller's factor, stays at or under the cut, and stops at the
         node BEFORE one whose scaled extent is past it.
// ORIGINAL: 0x006385D0 sub_6385d0 0x006385D0-0x00638611 FILE BYTE_EXACT
// symbol    _vox_walk_scaled_chain
// size      65 bytes
// callers   3   call targets   0
// kind      game
// flags     sp_ready;purged_ok
//
// HOMED from src/recovered/units/006385d0.cpp.
// NAMED 2026-09-04 from the body's own arithmetic: a flags/offset chain walked
// while a scaled float stays under a constant. Its binding g_00670fe8 is the
// 1.4142f at 0x00670FE8, vox_sqrt_two here.
// LEVER: BYTE_EXACT on the FPU operand order. Each extent is read into its
//        OWN local first and multiplied afterwards - `float extent =
//        *(float *)(cursor + 8);` then `extent * scale` - which is what makes
//        VC6 load the node's extent into st(0) and `fmul` the caller's
//        factor, the image's order. Writing the product as one expression
//        (`*(float *)(cursor + 8) * scale`, either operand order, named local
//        or not) loads [esp+8] first and fmul's the node: 15 of 23.
Return Value: the node the walk stopped at.
*/
extern "C" CvrShape *__cdecl vox_walk_scaled_chain(CvrShape *node, float scale) {
    CvrShape *cursor = node;
    unsigned char flags = static_cast<unsigned char>(cursor->field_0_);
    while (flags & 1) {
        // The extent is read into its own local first: the image loads the
        // node's extent into st(0) and multiplies by the caller's factor, so
        // the read has to be a statement of its own ahead of the multiply.
        float extent = cursor->scale_;
        // FCOMP/FNSTSW tests C0, so unordered products continue as well as
        // values below the cut. A negated '<' would stop on NaN instead.
        if (extent * scale >= vox_sqrt_two) {
            break;
        }
        CvrShape *next = reinterpret_cast<CvrShape *>(
            reinterpret_cast<unsigned char *>(cursor) + cursor->next_shape_offset_);
        float next_extent = next->scale_;
        // The second image gate accepts C0 or C3, including unordered.
        if (next_extent * scale > vox_sqrt_two) {
            break;
        }
        flags = static_cast<unsigned char>(next->field_0_);
        cursor = next;
    }
    return cursor;
}
