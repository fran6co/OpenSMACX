/* zlib 1.0.2 dependency source ownership and retained original-image history.
 * Upstream source, archive digest and license notice are documented in
 * src/vendor/zlib-1.0.2/PROVENANCE.md; no upstream source is modified.
 * Historical source-role wording below is retained as prose; component
 * ownership comes from the exact RECOVERY_COMPONENT identity/span join.
 */

/*
Purpose: zlib 1.0.2's adler32 - the rolling checksum deflate and inflate
         both consult - over 5,532-byte blocks with a sixteen-times
         unrolled inner loop.

// Historical source-role note: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00640A80 sub_640a80 0x00640A80-0x00640BAE FILE
// symbol    _adler32
// Historical emitted symbol: _sub_640a80
// TRIED: `if(len!=0){do}while` (jbe->je); ternary `len>0x15b0?0x15b0:len`
//            (jb->ja); `n -= blocks<<4` (neg->shl). Landed at MNEMONIC_ONLY
//            (127/127 mnemonics match) with `n = n + blocks * -16` mirroring
//            Ghidra's form; remaining diff is byte-level only, not chased.
// size      302 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the Adler-32 checksum of buffer, seeded from adler
Status: Complete
*/

/*
Purpose: Drain a decompressor's ring buffer into a stream: copy up to the
         stream's remaining capacity from the ring's read window, running
         the bytes through an optional decode callback, and repeat once
         more when the read pointer has wrapped back to the ring's start.

         The two halves are deliberately spelled out rather than shared:
         the original has the same block twice, and the second pass only
         runs on the wrap.

// Historical source-role note: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00644930 sub_644930 0x00644930-0x00644A62 FILE
// symbol    _inflate_flush
// Historical emitted symbol: _sub_644930
// TRIED: 3 stack args (IDA agrees), no receiver - ring-buffer flush; landed
//            at MISMATCH #4; not chased further.
// size      306 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0064498D 0x00644A19
Return Value: the status argument, unless a decode callback replaced it
Status: Complete
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
/*
Purpose: Reverse the low `count` bits of a value.

             xor eax,eax / loop: mov esi,ecx / and esi,1 / or eax,esi
             shr ecx,1 / shl eax,1 / dec edx / test edx,edx / jg loop
             shr eax,1 / ret

         The loop is a DO-WHILE: the body runs once before the counter is
         tested, so a count of zero or a negative one still moves one bit
         through and the answer is bit 0 rather than nothing. Writing it as a
         `for` would return 0 for those, which is a different function.

         The trailing `shr eax,1` undoes the last of the `shl`s, which is why
         the shift happens after the OR rather than before it.

// Historical source-role note: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00642940 _bi_reverse 0x00642940-0x0064295F BYTE_EXACT
// Historical emitted symbol: ?leaf_00642940_redirect@@YAIIH@Z
// CORRECTED from sub_642940
//   zlib 1.0.2 trees.c, 31 bytes, byte-exact from upstream
// size      31 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the low `count` bits, reversed
Status: Complete
*/

/*
Purpose: Append a 16-bit value to a byte stream, most significant byte first.

             mov esi,[eax+8] / mov edi,[eax+0x10] / mov edx,ecx / shr edx,8
             mov byte [esi+edi],dl / mov edx,[eax+0x10] / mov esi,[eax+8]
             inc edx / mov [eax+0x10],edx / mov byte [esi+edx],cl
             mov ecx,[eax+0x10] / inc ecx / mov [eax+0x10],ecx / ret

         BIG-endian: the high byte goes first. The cursor at 0x10 is advanced
         between the two stores and again after the second, so it ends two
         past where it started, and the second byte lands at the position the
         FIRST advance produced.

         Both the base at 0x8 and the cursor at 0x10 are RE-READ from the
         stream between the stores rather than kept in registers, so a stream
         that overlapped its own descriptor would see the updated values.
         Preserved for the same reason as in 005CBBC0 - and reached through
         load32/store32 so the compiler cannot cache what the original
         re-reads.

         The CURSOR re-read is pinned by a fixture that points the stream at
         its own descriptor. The BASE re-read is not: separating it needs the
         first byte store to land on the descriptor's own pointer field at
         0x8, and what the corrupted pointer then addresses depends on the
         arena's absolute address, so the outcome could not be asserted
         portably. Two mutants of that line survive the sweep for that reason
         and are left alone rather than papered over.

// Historical source-role note: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x0063D420 _putShortMSB 0x0063D420-0x0063D44F
// Historical emitted symbol: ?leaf_0063d420_redirect@@YAXPAXI@Z
// CORRECTED from sub_63d420
//   zlib 1.0.2 deflate.c, 47 bytes, byte-exact from upstream
// size      47 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063cf00.cpp
// ORIGINAL: 0x0063CF00 deflateInit_ 0x0063CF00-0x0063CF25 BYTE_EXACT
// Historical emitted symbol: ?deflateInit_@@YAHPAXHPBDH@Z
// size      37 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063CF30
// 0x0063CF00  deflateInit_  ->  _deflateInit_
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
*/

/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/0063d4d0.cpp
// ORIGINAL: 0x0063D4D0 _deflateEnd 0x0063D4D0-0x0063D55F FILE BYTE_EXACT
// Historical emitted symbol: _sub_63d4d0
// CORRECTED from sub_63d4d0
//   zlib 1.0.2 deflate.c, 143 bytes, byte-exact from upstream
// size      143 bytes
// prototype
// callers   2   call targets   0
// kind
// flags
// calls     (none)
// indirect  0x0063D4EC 0x0063D501 0x0063D516 0x0063D52B 0x0063D53D
// working copy - scaffold materialised by --work
*/

/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063df90.cpp
Historical ABI note: the seed rejected a nullary catalogue guess and observed
stack arguments, not ECX, for callee 0x00643950. Its prose said "one pointer
argument", but its declaration had three arguments; pinned zlib infblock.h
and infblock.c define inflate_blocks_free with three. The one-argument prose
is retained as a historical contradiction, not an active ABI assertion.
// ORIGINAL: 0x0063DF90 _inflateEnd 0x0063DF90-0x0063DFDD BYTE_EXACT
// Historical emitted symbol: _sub_63df90
// CORRECTED from sub_63df90
//   zlib 1.0.2 inflate.c, 77 bytes, byte-exact from upstream
// size      77 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00643950
// indirect  0x0063DFC5
// 0x0063DF90  sub_63df90  ->  _sub_63df90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
*/

/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063e0f0.cpp
// ORIGINAL: 0x0063E0F0 _inflateInit_ 0x0063E0F0-0x0063E10A BYTE_EXACT
// Historical emitted symbol: ?sub_63e0f0@@YAHHHH@Z
// CORRECTED from sub_63e0f0
//   zlib 1.0.2 inflate.c, 26 bytes, byte-exact from upstream
// size      26 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063DFE0
// 0x0063E0F0  sub_63e0f0  ->  _sub_63e0f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
*/

/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/0063d450.cpp
Historical lesson below refers to a retired src/recovered seed, not a live input.
// ORIGINAL: 0x0063D450 _flush_pending 0x0063D450-0x0063D4C2 FILE BYTE_EXACT
// Historical emitted symbol: _sub_63d450
// LEVER: `rep movsd`/`rep movsb` is VC6's /Oi expansion of memcpy(dst, src, n) for a runtime-determined n, not a hand-rolled loop (matches the strcpy lesson already in src/recovered/00639390.cpp). Catalogued as nullary returning int; the disassembly reads a real stack argument at [esp+4] and never sets eax on purpose, so it is a one-`int`-parameter `void` function instead. Min-of-two-sizes has to be spelled `if (count > mySize) count = mySize;` (condition polarity `>`, not `<` on the flipped operands) to get the original's `jbe`; the algebraically-equivalent `if (mySize < count)` compiles to `jae`, and a `cond ? a : b` ternary compiles to `ja` - same logic, three different branch mnemonics.
// CORRECTED from sub_63d450
//   zlib 1.0.2 deflate.c, 114 bytes, byte-exact from upstream
// size      114 bytes
// prototype
// callers   4   call targets   0
// kind
// flags
// calls     (none)
// working copy - scaffold materialised by --work
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
/*
Purpose: Flush complete bytes from the deflate bit accumulator, retaining fewer than eight bits.
// ORIGINAL: 0x00642960 _bi_flush 0x00642960-0x006429EB FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Flush the deflate bit accumulator and finish on a byte boundary.
// ORIGINAL: 0x006429F0 _bi_windup 0x006429F0-0x00642A6C FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Align and copy a stored deflate block, optionally emitting its length and complement.
// ORIGINAL: 0x00642A70 _copy_block 0x00642A70-0x00642B07 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Validate and allocate a configured deflate stream before resetting its state.
// ORIGINAL: 0x0063CF30 _deflateInit2_ 0x0063CF30-0x0063D11C FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Reset a deflate stream while retaining its allocated compression storage.
// ORIGINAL: 0x0063D120 _deflateReset 0x0063D120-0x0063D193 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Drive streaming deflate, pending output, block selection and the zlib header/trailer.
// ORIGINAL: 0x0063D1A0 _deflate 0x0063D1A0-0x0063D413 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Initialize the sliding-window match finder and compression-level tuning.
// ORIGINAL: 0x0063D560 _lm_init 0x0063D560-0x0063D5F3 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Initialize the Huffman descriptors, bit accumulator and first deflate block.
// ORIGINAL: 0x00640BB0 __tr_init 0x00640BB0-0x00640C22 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Generate static Huffman and length/distance encoding tables once.
// ORIGINAL: 0x00640C30 _tr_static_init 0x00640C30-0x00640E41 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Reset dynamic Huffman frequencies and seed the end-of-block symbol.
// ORIGINAL: 0x00640E50 _init_block 0x00640E50-0x00640EB6 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Generate canonical Huffman codes and reverse their transmitted bit order.
// ORIGINAL: 0x00640EC0 _gen_codes 0x00640EC0-0x00640F33 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Emit a stored-block header and copy its uncompressed bytes.
// ORIGINAL: 0x00640F40 __tr_stored_block 0x00640F40-0x00640FF3 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
Purpose: Emit empty static blocks providing the decoder required lookahead.
// ORIGINAL: 0x00641000 __tr_align 0x00641000-0x00641273 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
This unclaimed original identity is routed by the component binding below;
no application substitute body or current measurement claim is recorded here.
*/

/*
// ORIGINAL: 0x0063D6F0 _fill_window 0x0063D6F0-0x0063D812 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063D820 _read_buf 0x0063D820-0x0063D891 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063D8A0 _deflate_fast 0x0063D8A0-0x0063DACF FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063DAD0 _longest_match 0x0063DAD0-0x0063DC3F FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063DC40 _deflate_slow 0x0063DC40-0x0063DF33 FILE
Dependency implementation: unmodified zlib 1.0.2 deflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00641280 __tr_flush_block 0x00641280-0x006414A3 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x006414B0 _build_tree 0x006414B0-0x006416E3 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x006416F0 _pqdownheap 0x006416F0-0x006417C1 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x006417D0 _gen_bitlen 0x006417D0-0x006419F1 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00641A00 _build_bl_tree 0x00641A00-0x00641A6F FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00641A70 _scan_tree 0x00641A70-0x00641B57 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00641B60 _send_all_trees 0x00641B60-0x00641DC5 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00641DD0 _send_tree 0x00641DD0-0x00642345 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00642350 __tr_tally 0x00642350-0x00642477 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00642480 _compress_block 0x00642480-0x006428BB FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x006428C0 _set_data_type 0x006428C0-0x00642934 FILE
Dependency implementation: unmodified zlib 1.0.2 trees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
/*
// ORIGINAL: 0x00642B10 _inflate_blocks_reset 0x00642B10-0x00642BA2 FILE
Dependency implementation: unmodified zlib 1.0.2 infblock.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00642BB0 _inflate_blocks_new 0x00642BB0-0x00642C1E FILE
Dependency implementation: unmodified zlib 1.0.2 infblock.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00643950 _inflate_blocks_free 0x00643950-0x00643981 FILE
Dependency implementation: unmodified zlib 1.0.2 infblock.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00644140 _inflate_codes_new 0x00644140-0x00644179 FILE
Dependency implementation: unmodified zlib 1.0.2 infcodes.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00644910 _inflate_codes_free 0x00644910-0x00644924 FILE
Dependency implementation: unmodified zlib 1.0.2 infcodes.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00644A70 _inflate_fast 0x00644A70-0x00644DB2 FILE
Dependency implementation: unmodified zlib 1.0.2 inffast.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063DF40 _inflateReset 0x0063DF40-0x0063DF84 FILE
Dependency implementation: unmodified zlib 1.0.2 inflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x0063DFE0 _inflateInit2_ 0x0063DFE0-0x0063E0EE FILE
Dependency implementation: unmodified zlib 1.0.2 inflate.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00643990 _inflate_trees_bits 0x00643990-0x006439E5 FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x006439F0 _huft_build 0x006439F0-0x00643EA1 FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00643EB0 _inflate_trees_dynamic 0x00643EB0-0x00643F98 FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00643FA0 _inflate_trees_fixed 0x00643FA0-0x006440DE FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00644100 _inflate_trees_free 0x00644100-0x0064413F FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

/*
// ORIGINAL: 0x00640A50 _zcalloc 0x00640A50-0x00640A63 FILE
Dependency implementation: unmodified zlib 1.0.2 zutil.c.
Original extent is independently pinned in docs/recovery/ghidra-functions.csv.
This source correspondence has no new matching claim.
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
/*
// ORIGINAL: 0x0063E110 _inflate 0x0063E110-0x0063E4E8 FILE
Dependency implementation: unmodified zlib 1.0.2 inflate.c state machine.
The original mapping retains its independently pinned code-only extent.
Trailing compiler switch tables require separate physical-boundary evidence.
This source correspondence grants no matching claim.
*/

/*
// ORIGINAL: 0x00642C20 _inflate_blocks 0x00642C20-0x0064390D FILE
Dependency implementation: unmodified zlib 1.0.2 infblock.c state machine.
The original mapping retains its independently pinned code-only extent.
Trailing compiler switch tables require separate physical-boundary evidence.
This source correspondence grants no matching claim.
*/

/*
// ORIGINAL: 0x00644180 _inflate_codes 0x00644180-0x006448E6 FILE
Dependency implementation: unmodified zlib 1.0.2 infcodes.c state machine.
The original mapping retains its independently pinned code-only extent.
Trailing compiler switch tables require separate physical-boundary evidence.
This source correspondence grants no matching claim.
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
/*
// ORIGINAL: 0x00640A70 _zcfree 0x00640A70-0x00640A7C FILE
Dependency implementation: unmodified zlib 1.0.2 zutil.c.
Pinned zlib1.0.2 zutil.c definition and original640A70..640A7C: free(ptr) through already bound runtime free644EF2; callbacks selected at63CF8A and63E034. IDA extent, complete native decode and locally compiled unmodified upstream COFF corroborate source correspondence, not original archive selection; no independent Ghidra row. This is not a current byte claim; dependency retained selection and runtime equivalence remain unproved.
*/

/*
// ORIGINAL: 0x006440E0 _falloc 0x006440E0-0x006440F8 FILE
Dependency implementation: unmodified zlib 1.0.2 inftrees.c.
Pinned zlib1.0.2 inftrees.c static allocator; original inflate_trees_fixed installs its pointer at64400C. Native6440E0..6440F8 decrements the opaque count then forms fixed_mem-relative storage. IDA extent, complete native decode and locally compiled static COFF symbol corroborate source correspondence, not original archive selection; no independent Ghidra row. The fixed_mem original allocation/extent and reference remain unknown; do not infer a data binding from the LEA immediate. This is not a current byte claim; dependency retained selection and runtime equivalence remain unproved.
*/

// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6",
//   "archive_url": "https://zlib.net/fossils/zlib-1.0.2.tar.gz",
//   "component": {
//     "component_id": "zlib",
//     "delivery": "upstream_source",
//     "evidence": [
//       "src/vendor/zlib-1.0.2/PROVENANCE.md",
//       "git:948389543e746ff061ab04cb86e839fdc3600146:src/vendor/zlib-1.0.2/PROVENANCE.md",
//       "existing CMake zlib static target; upstream 1.0.2 archive and zlib.h license notice"
//     ],
//     "ownership": "third_party",
//     "version": "1.0.2"
//   },
//   "flags": [
//     "/c",
//     "/O2",
//     "/Gy",
//     "/GR-",
//     "/GX"
//   ],
//   "kind": "vendor_component",
//   "language": "c",
//   "license_file": "zlib.h",
//   "schema_version": 1,
//   "source_root": "src/vendor/zlib-1.0.2",
//   "units": [
//     "adler32.c",
//     "deflate.c",
//     "infblock.c",
//     "infcodes.c",
//     "inffast.c",
//     "inflate.c",
//     "inftrees.c",
//     "infutil.c",
//     "trees.c",
//     "uncompr.c",
//     "zutil.c"
//   ],
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "files": [
//     {"archive_member": "zlib-1.0.2/adler32.c", "path": "adler32.c"},
//     {"archive_member": "zlib-1.0.2/deflate.c", "path": "deflate.c"},
//     {"archive_member": "zlib-1.0.2/deflate.h", "path": "deflate.h"},
//     {"archive_member": "zlib-1.0.2/infblock.c", "path": "infblock.c"},
//     {"archive_member": "zlib-1.0.2/infblock.h", "path": "infblock.h"},
//     {"archive_member": "zlib-1.0.2/infcodes.c", "path": "infcodes.c"},
//     {"archive_member": "zlib-1.0.2/infcodes.h", "path": "infcodes.h"},
//     {"archive_member": "zlib-1.0.2/inffast.c", "path": "inffast.c"},
//     {"archive_member": "zlib-1.0.2/inffast.h", "path": "inffast.h"},
//     {"archive_member": "zlib-1.0.2/inflate.c", "path": "inflate.c"},
//     {"archive_member": "zlib-1.0.2/inftrees.c", "path": "inftrees.c"},
//     {"archive_member": "zlib-1.0.2/inftrees.h", "path": "inftrees.h"},
//     {"archive_member": "zlib-1.0.2/infutil.c", "path": "infutil.c"},
//     {"archive_member": "zlib-1.0.2/infutil.h", "path": "infutil.h"},
//     {"archive_member": "zlib-1.0.2/trees.c", "path": "trees.c"},
//     {"archive_member": "zlib-1.0.2/uncompr.c", "path": "uncompr.c"},
//     {"archive_member": "zlib-1.0.2/zconf.h", "path": "zconf.h"},
//     {"archive_member": "zlib-1.0.2/zlib.h", "path": "zlib.h"},
//     {"archive_member": "zlib-1.0.2/zutil.c", "path": "zutil.c"},
//     {"archive_member": "zlib-1.0.2/zutil.h", "path": "zutil.h"}
//   ],
//   "bindings": [
//     {"evidence": "src/vendor/zlib-1.0.2/adler32.c source definition and pinned upstream zlib 1.0.2 provenance; physical COFF build/recovery/builds/build-gyzprt2j/CMakeFiles/zlib.dir/src/vendor/zlib-1.0.2/adler32.c.obj defines _adler32 at section 2 (.text); installed strict comparator: COFF bytes/boundary/references pass for 0x00640A80-0x00640BAE; final-link selection remains unknown.", "origin": "0x00640A80", "spans": ["0x00640A80-0x00640BAE"], "symbol": "_adler32", "unit": "adler32.c"},
//     {"evidence": "src/vendor/zlib-1.0.2/infutil.c source definition and pinned upstream zlib 1.0.2 provenance; physical COFF build/recovery/builds/build-gyzprt2j/CMakeFiles/zlib.dir/src/vendor/zlib-1.0.2/infutil.c.obj defines _inflate_flush at section 3 (.text); installed strict comparator: COFF bytes/boundary/references pass for 0x00644930-0x00644A62; final-link selection remains unknown.", "origin": "0x00644930", "spans": ["0x00644930-0x00644A62"], "symbol": "_inflate_flush", "unit": "infutil.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: upstream trees.c defines static bi_reverse with the mapped do-while bit-reversal algorithm. Its callers are tr_static_init with fixed length 5 and gen_codes, which skips zero lengths and passes legal Huffman lengths 1..15. This records source ownership only, not a current byte claim. Component-binary bytes, boundary, and references require a fresh configured component-verifier measurement. Application retained-selection status is UNKNOWN. Whole-library version uniqueness and runtime equivalence are unproved.", "origin": "0x00642940", "spans": ["0x00642940-0x0064295F"], "symbol": "_bi_reverse", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: upstream deflate.c defines static putShortMSB as high-byte then low-byte stores through put_byte. Its five deflate() call sites emit the zlib header, optional preset-dictionary Adler-32, and Adler-32 trailer. In deflate.h internal_state, leading 32-bit i386 fields place pending_buf at +0x08 and pending at +0x10, matching the mapped accesses. This records source ownership only, not a current byte claim. Component-binary bytes, boundary, and references require a fresh configured component-verifier measurement. Application retained-selection status is UNKNOWN. Whole-library version uniqueness and runtime equivalence are unproved.", "origin": "0x0063D420", "spans": ["0x0063D420-0x0063D44F"], "symbol": "_putShortMSB", "unit": "deflate.c"},
//     {"evidence": "zlib 1.0.2; deflate.c definition `int deflateInit_(strm, level, version, stream_size)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `?deflateInit_@@YAHPAXHPBDH@Z` was historical metadata, not a current binary measurement; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063cf00.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063CF00", "spans": ["0x0063CF00-0x0063CF25"], "symbol": "_deflateInit_", "unit": "deflate.c"},
//     {"evidence": "zlib 1.0.2; deflate.c definition `int deflateEnd (strm)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_63d4d0` was historical metadata, not a current binary measurement; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/0063d4d0.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D4D0", "spans": ["0x0063D4D0-0x0063D55F"], "symbol": "_deflateEnd", "unit": "deflate.c"},
//     {"evidence": "zlib 1.0.2; inflate.c definition `int inflateEnd(z)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_63df90` was historical metadata, not a current binary measurement; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063df90.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063DF90", "spans": ["0x0063DF90-0x0063DFDD"], "symbol": "_inflateEnd", "unit": "inflate.c"},
//     {"evidence": "zlib 1.0.2; inflate.c definition `int inflateInit_(z, version, stream_size)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `?sub_63e0f0@@YAHHHH@Z` was historical metadata, not a current binary measurement; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/recovered/0063e0f0.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063E0F0", "spans": ["0x0063E0F0-0x0063E10A"], "symbol": "_inflateInit_", "unit": "inflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 upstream deflate.c lines 390-408 define `local void flush_pending(z_stream *strm)`; local is static in this release. The exact upstream archive SHA256 is recorded by this companion.; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/0063d450.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D450", "spans": ["0x0063D450-0x0063D4C2"], "symbol": "_flush_pending", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: trees.c bi_flush reads bi_valid, emits pending bytes through put_short/put_byte, and retains the residual accumulator. Boundaries refer to the independent physical function contribution; following alignment fill is not part of its original body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00642960", "spans": ["0x00642960-0x006429EB"], "symbol": "_bi_flush", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: trees.c bi_windup writes one or two bytes according to bi_valid and clears the accumulator and its valid-bit count. Boundaries refer to the independent physical function contribution; following alignment fill is not part of its original body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x006429F0", "spans": ["0x006429F0-0x00642A6C"], "symbol": "_bi_windup", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: trees.c copy_block first calls bi_windup, sets last_eob_len to8, optionally emits the length/complement header, then copies the requested bytes. Its mapped call target is _bi_windup at0x006429F0. Boundaries refer to the independent physical function contribution; following alignment fill is not part of its original body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00642A70", "spans": ["0x00642A70-0x00642B07"], "symbol": "_copy_block", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: deflateInit2_ validates version/stream size and compression parameters, supplies zcalloc/zcfree when absent, allocates compression storage, then calls deflateReset; failure reports z_errmsg and releases through deflateEnd. String, allocator and error-table targets require independent qualification. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063CF30", "spans": ["0x0063CF30-0x0063D11C"], "symbol": "_deflateInit2_", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: deflateReset clears totals, pending output and status, resets the checksum, initializes trees through _tr_init and match state through lm_init. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D120", "spans": ["0x0063D120-0x0063D193"], "symbol": "_deflateReset", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: deflate emits the header through putShortMSB, drains flush_pending, dispatches the configured compression function and emits alignment/stored blocks and the checksum trailer. Error and configuration-table targets require independent qualification. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D1A0", "spans": ["0x0063D1A0-0x0063D413"], "symbol": "_deflate", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: lm_init clears hash state and reads per-level tuning from configuration_table before resetting match, position and input state. Configuration-table targets require independent qualification. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D560", "spans": ["0x0063D560-0x0063D5F3"], "symbol": "_lm_init", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: _tr_init calls tr_static_init, attaches the three static tree descriptors, resets bit state, then calls init_block. Descriptor data targets require independent qualification; source leading underscore acquires a second underscore under C decoration. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00640BB0", "spans": ["0x00640BB0-0x00640C22"], "symbol": "__tr_init", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: tr_static_init gates its static tables on static_init_done, generates length/distance mappings and static literal/distance trees, and calls gen_codes/bi_reverse. Mutable tables, constants and the static guard require independent qualification. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00640C30", "spans": ["0x00640C30-0x00640E41"], "symbol": "_tr_static_init", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: init_block clears the literal, distance and bit-length frequency trees, sets END_BLOCK frequency to one, and clears block accounting. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00640E50", "spans": ["0x00640E50-0x00640EB6"], "symbol": "_init_block", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: gen_codes computes next_code from bit-length counts and assigns bi_reverse(next_code[length]++,length) to each nonzero-length symbol. Its bi_reverse target is independently source-bound at642940. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00640EC0", "spans": ["0x00640EC0-0x00640F33"], "symbol": "_gen_codes", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: _tr_stored_block writes the block-type/eof bits, accounts aligned stored length, then calls copy_block with the length-header flag set. Its copy_block target is independently source-bound at642A70; source leading underscore acquires a second underscore under C decoration. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00640F40", "spans": ["0x00640F40-0x00640FF3"], "symbol": "__tr_stored_block", "unit": "trees.c"},
//     {"evidence": "Pinned zlib 1.0.2 source correspondence: _tr_align emits static-tree END_BLOCK codes, flushes complete bytes and conditionally emits another empty block according to last_eob_len/bi_valid. The static_ltree data target requires independent qualification; source leading underscore acquires a second underscore under C decoration. Independently pinned function boundaries agree with the physical upstream COFF contribution; following alignment fill is outside this body span. This records dependency ownership only, not a current byte claim. Fresh configured component-binary bytes, boundary and reference verification is required; any missing data/string/helper evidence must remain unknown. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x00641000", "spans": ["0x00641000-0x00641273"], "symbol": "__tr_align", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063D6F0", "spans": ["0x0063D6F0-0x0063D812"], "symbol": "_fill_window", "unit": "deflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063D820", "spans": ["0x0063D820-0x0063D891"], "symbol": "_read_buf", "unit": "deflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063D8A0", "spans": ["0x0063D8A0-0x0063DACF"], "symbol": "_deflate_fast", "unit": "deflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063DAD0", "spans": ["0x0063DAD0-0x0063DC3F"], "symbol": "_longest_match", "unit": "deflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063DC40", "spans": ["0x0063DC40-0x0063DF33"], "symbol": "_deflate_slow", "unit": "deflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00641280", "spans": ["0x00641280-0x006414A3"], "symbol": "__tr_flush_block", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x006414B0", "spans": ["0x006414B0-0x006416E3"], "symbol": "_build_tree", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x006416F0", "spans": ["0x006416F0-0x006417C1"], "symbol": "_pqdownheap", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x006417D0", "spans": ["0x006417D0-0x006419F1"], "symbol": "_gen_bitlen", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00641A00", "spans": ["0x00641A00-0x00641A6F"], "symbol": "_build_bl_tree", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00641A70", "spans": ["0x00641A70-0x00641B57"], "symbol": "_scan_tree", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00641B60", "spans": ["0x00641B60-0x00641DC5"], "symbol": "_send_all_trees", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00641DD0", "spans": ["0x00641DD0-0x00642345"], "symbol": "_send_tree", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00642350", "spans": ["0x00642350-0x00642477"], "symbol": "__tr_tally", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00642480", "spans": ["0x00642480-0x006428BB"], "symbol": "_compress_block", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates the source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x006428C0", "spans": ["0x006428C0-0x00642934"], "symbol": "_set_data_type", "unit": "trees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00642B10", "spans": ["0x00642B10-0x00642BA2"], "symbol": "_inflate_blocks_reset", "unit": "infblock.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00642BB0", "spans": ["0x00642BB0-0x00642C1E"], "symbol": "_inflate_blocks_new", "unit": "infblock.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00643950", "spans": ["0x00643950-0x00643981"], "symbol": "_inflate_blocks_free", "unit": "infblock.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00644140", "spans": ["0x00644140-0x00644179"], "symbol": "_inflate_codes_new", "unit": "infcodes.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00644910", "spans": ["0x00644910-0x00644924"], "symbol": "_inflate_codes_free", "unit": "infcodes.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00644A70", "spans": ["0x00644A70-0x00644DB2"], "symbol": "_inflate_fast", "unit": "inffast.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063DF40", "spans": ["0x0063DF40-0x0063DF84"], "symbol": "_inflateReset", "unit": "inflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063DFE0", "spans": ["0x0063DFE0-0x0063E0EE"], "symbol": "_inflateInit2_", "unit": "inflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00643990", "spans": ["0x00643990-0x006439E5"], "symbol": "_inflate_trees_bits", "unit": "inftrees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x006439F0", "spans": ["0x006439F0-0x00643EA1"], "symbol": "_huft_build", "unit": "inftrees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00643EB0", "spans": ["0x00643EB0-0x00643F98"], "symbol": "_inflate_trees_dynamic", "unit": "inftrees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00643FA0", "spans": ["0x00643FA0-0x006440DE"], "symbol": "_inflate_trees_fixed", "unit": "inftrees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00644100", "spans": ["0x00644100-0x0064413F"], "symbol": "_inflate_trees_free", "unit": "inftrees.c"},
//     {"evidence": "Unmodified zlib1.0.2 source definition and independently pinned Ghidra extent; exclusive physical VC6 function COMDAT corroborates source correspondence. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00640A50", "spans": ["0x00640A50-0x00640A63"], "symbol": "_zcalloc", "unit": "zutil.c"},
//     {"evidence": "Unmodified zlib1.0.2 state-machine implementation and independently pinned Ghidra code-only extent. Physical function COMDAT also contains compiler switch tables beyond this code span; their allocation/ownership and complete reference qualification remain obligations. Preserve the code-only span and native boundary failure; do not extend it or manufacture separate table declarations. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x0063E110", "spans": ["0x0063E110-0x0063E4E8"], "symbol": "_inflate", "unit": "inflate.c"},
//     {"evidence": "Unmodified zlib1.0.2 state-machine implementation and independently pinned Ghidra code-only extent. Physical function COMDAT also contains compiler switch tables beyond this code span; their allocation/ownership and complete reference qualification remain obligations. Preserve the code-only span and native boundary failure; do not extend it or manufacture separate table declarations. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00642C20", "spans": ["0x00642C20-0x0064390D"], "symbol": "_inflate_blocks", "unit": "infblock.c"},
//     {"evidence": "Unmodified zlib1.0.2 state-machine implementation and independently pinned Ghidra code-only extent. Physical function COMDAT also contains compiler switch tables beyond this code span; their allocation/ownership and complete reference qualification remain obligations. Preserve the code-only span and native boundary failure; do not extend it or manufacture separate table declarations. This is not a current byte claim; any missing data/string/helper evidence must remain unknown. Complete dependency data, retained selection and runtime equivalence remain obligations.", "origin": "0x00644180", "spans": ["0x00644180-0x006448E6"], "symbol": "_inflate_codes", "unit": "infcodes.c"},
//     {"evidence": "Unmodified zlib1.0.2 deflate.c local deflate_stored; pinned deflate63D320..328 selects the level-zero configuration table pointer at698CF8. Independent complete native decode and IDA extent end at RET63D6EE, exclusive63D6EF; the alignment NOP at6EF is not body. Calls fill_window63D6F0, __tr_flush_block641280 and flush_pending63D450; existing physical dependency COFF corroborates the static symbol and TU. This binds dependency correspondence only: current exactness/references, linked selection and runtime equivalence remain obligations.", "origin": "0x0063D600", "spans": ["0x0063D600-0x0063D6EF"], "symbol": "_deflate_stored", "unit": "deflate.c"},
//     {"evidence": "Pinned zlib1.0.2 zutil.c definition and original640A70..640A7C: free(ptr) through already bound runtime free644EF2; callbacks selected at63CF8A and63E034. IDA extent, complete native decode and locally compiled unmodified upstream COFF corroborate source correspondence, not original archive selection; no independent Ghidra row. This is not a current byte claim; dependency retained selection and runtime equivalence remain unproved.", "origin": "0x00640A70", "spans": ["0x00640A70-0x00640A7C"], "symbol": "_zcfree", "unit": "zutil.c"},
//     {"evidence": "Pinned zlib1.0.2 inftrees.c static allocator; original inflate_trees_fixed installs its pointer at64400C. Native6440E0..6440F8 decrements the opaque count then forms fixed_mem-relative storage. IDA extent, complete native decode and locally compiled static COFF symbol corroborate source correspondence, not original archive selection; no independent Ghidra row. The fixed_mem original allocation/extent and reference remain unknown; do not infer a data binding from the LEA immediate. This is not a current byte claim; dependency retained selection and runtime equivalence remain unproved.", "origin": "0x006440E0", "spans": ["0x006440E0-0x006440F8"], "symbol": "_falloc", "unit": "inftrees.c"}
//   ]
// }
// RECOVERY_COMPONENT_END
