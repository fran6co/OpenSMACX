// ORIGINAL: 0x006414B0 BYTE_EXACT FILE
// name      _build_tree
// size      563 bytes
// spans     0x006414B0-0x006416E3
// prototype 
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00640EC0 0x006416F0 0x006417D0
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `build_tree` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
//
// SUPERSEDED A HAND-WRITTEN BODY, and its reasoning is kept here
// rather than dropped with the file. That body scored MISMATCH by
// transcribing the disassembly; this one is byte-exact because it
// is the source the disassembly was compiled from. The transcription
// lived at src/unrecovered/006414b0.cpp and is in the history.
//
//   was: this is zlib/deflate's build_tree(deflate_state*, tree_desc*)
//   was: nothing further tried; MISMATCH #9 (xor vs missing insn, a
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
