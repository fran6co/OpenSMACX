// ORIGINAL: 0x00641B60 BYTE_EXACT FILE
// name      _send_all_trees
// size      613 bytes
// spans     0x00641B60-0x00641DC5
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00641DD0
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `send_all_trees` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
//
// SUPERSEDED A HAND-WRITTEN BODY, and its reasoning is kept here
// rather than dropped with the file. That body scored MISMATCH by
// transcribing the disassembly; this one is byte-exact because it
// is the source the disassembly was compiled from. The transcription
// lived at src/unrecovered/00641b60.cpp and is in the history.
//
//   was: nullary int contract - disasm reads 4 stack params before any
//            push beyond ebx/ebp (matches IDA's 4-int guess and Ghidra's
//            zlib send_bits/send_all_trees shape); changed to void, 4 ints.
//            Ghidra's own transcription (send_bits inlined 4x plus a
//            bl_order loop) checked out field-for-field against the raw
//            offsets (0x16b0 bit buffer, 0x16b4 valid-bit count, 0x10/8
//            output cursor+base, 0xa72 dyn-tree array) and compiles as-is.
//            Divergence starts at #1 (push vs mov) - register allocation
//            for the extra C++ temporaries, not chased further.
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
