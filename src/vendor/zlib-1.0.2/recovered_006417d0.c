// ORIGINAL: 0x006417D0 BYTE_EXACT FILE
// name      _gen_bitlen
// size      545 bytes
// spans     0x006417D0-0x006419F1
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// NOT WRITTEN HERE. This function is zlib 1.0.2, which the image
// names itself - see PROVENANCE.md beside this file. The recovery
// is the upstream `trees.c`, unmodified, and this wrapper exists
// only to carry the annotation so the source stays byte-identical
// to the release. Compiled as C under `/c /O2 /Gy /GR- /GX`;
// `gen_bitlen` is `local`, so it is selected out of the object among
// the STATIC symbols that `/Gy` gives their own COMDAT.
//
// SUPERSEDED A HAND-WRITTEN BODY, and its reasoning is kept here
// rather than dropped with the file. That body scored MISMATCH by
// transcribing the disassembly; this one is byte-exact because it
// is the source the disassembly was compiled from. The transcription
// lived at src/unrecovered/006417d0.cpp and is in the history.
//
//   was: local-variable declaration order (overflow=0 grouped with the
//            desc-> loads vs interleaved between them, matching the
//            original's `push edi; xor esi,esi` placement) does not move
//            the divergence (MISMATCH #9 'push' vs 'mov' either way,
//            95.0% mnemonic similarity); not chased further. This is zlib
//            `trees.c: gen_bitlen(deflate_state *s, tree_desc *desc)` -
//            identified by HEAP_SIZE=0x23d, the MAX_BITS+1=16-short
//            bl_count zero-fill, `tree[heap[heap_max]].Len=0`, and the
//            overflow-redistribution do/while, all matching the real
//            source exactly; `h` is deliberately left un-reinitialised
//            before the final loop; the disassembly has no `mov eax,`
//            before `ret`, so the signature is void(void*, tree_desc*),
//            not the scaffold's int().
// LEVER: upstream-source the recovery IS zlib 1.0.2 `trees.c`, compiled as C
//        under `/c /O2 /Gy /GR- /GX`. Nothing here was written to match
//        the disassembly; the disassembly was compiled from this.
//
#include "trees.c"
