// Vox render-record lifetime; Caviar frontend owns its scene teardown.
#include "stdafx.h"
#include "caviar.h"

// Shared engine count remains with the allocator in caviar.cpp.
extern uint32_t vox_record_count;

/*
Purpose: Release a render record and the four tables it owns, then drop the
         count of records in existence. The colour and shadow tables and the
         two ramp pointer arrays are each freed only when non-null; the record
         itself is freed whatever it holds.
// ORIGINAL: 0x00639630 sub_639630 0x00639630-0x00639694 BYTE_EXACT
// symbol    ?vox_release_record@@YAXPAUVoxRenderRecord@@@Z
// size      100 bytes
// prototype void (__cdecl sub_639630)(int param_1)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0063963D 0x0063964E 0x0063965F 0x00639670 0x0063967A
//
// Historical homing evidence (2026-09-05): the record it tears down is the one VoxRenderRecord
// models and its caller is Caviar::close_class. Every release still uses
// the engine's live release callback, shared with caviar_free_record.
Return Value: n/a
Status: Complete
*/
void __cdecl vox_release_record(VoxRenderRecord *record) {
    if (record->colour_table_ != 0) {
        CaviarOutputMessageWindow.io_release_(record->colour_table_);
    }
    if (record->shadow_table_ != 0) {
        CaviarOutputMessageWindow.io_release_(record->shadow_table_);
    }
    if (record->ramp_a_ptr_ != 0) {
        CaviarOutputMessageWindow.io_release_(record->ramp_a_ptr_);
    }
    if (record->ramp_b_ptr_ != 0) {
        CaviarOutputMessageWindow.io_release_(record->ramp_b_ptr_);
    }
    CaviarOutputMessageWindow.io_release_(record);
    if (vox_record_count != 0) {
        --vox_record_count;
    }
}

// 0x00639380, the engine's shutdown hook, is a nullsub - a function whose
// whole body is one `ret`. close_class calls it, so a body that does nothing
// is the faithful spelling; the engine module owns its definition.
// ORIGINAL: 0x00639380 nullsub_16 0x00639380-0x00639381 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x00639380-0x00639381 is a single RET", "source:src/caviar.cpp Caviar::close_class calls vox_engine_close for the original shutdown hook", "pinned/current owner facts identify one original caller; the source body remains deliberately empty"], "reason": "Rehome the no-op shutdown-hook identity onto the already-called, out-of-line vox_engine_close owner; no separate adapter body is retained."}
// symbol    ?vox_engine_close@@YAXXZ
// kind      game
void vox_engine_close() {
}

