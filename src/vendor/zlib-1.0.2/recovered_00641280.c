// ORIGINAL: 0x00641280 BYTE_EXACT FILE
// name      __tr_flush_block
// size      547 bytes
// spans     0x00641280-0x006414A3
// prototype 
// callers   3   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640E50 0x00640F40 0x006414B0 0x00641A00 0x00641B60 0x00642480 0x006428C0 0x006429F0
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `tr_flush_block` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
//
// SUPERSEDED A HAND-WRITTEN BODY, and its reasoning is kept here
// rather than dropped with the file. That body scored MISMATCH by
// transcribing the disassembly; this one is byte-exact because it
// is the source the disassembly was compiled from. The transcription
// lived at src/unrecovered/00641280.cpp and is in the history.
//
//   was: bi_valid load hoisted before the static/dyn branch (matches instr
//            order to #51/553 bytes) but the interleaved cmp/mov/jne scheduling
//            for uVar4==uVar5 (bi_valid loaded between cmp and jne) did not
//            reproduce from either hoisting or duplicating the load per-branch.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
