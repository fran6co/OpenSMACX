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
//     "inflate.c",
//     "infutil.c",
//     "trees.c"
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
//     {"evidence": "Pinned zlib 1.0.2 upstream deflate.c lines 390-408 define `local void flush_pending(z_stream *strm)`; local is static in this release. The exact upstream archive SHA256 is recorded by this companion.; historical identity/claim restored from git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/0063d450.cpp. Source correspondence only; no current COFF qualification is stored here. Fresh configured component measurement is required. Application retained selection, whole-library version uniqueness and runtime equivalence remain unproved.", "origin": "0x0063D450", "spans": ["0x0063D450-0x0063D4C2"], "symbol": "_flush_pending", "unit": "deflate.c"}
//   ]
// }
// RECOVERY_COMPONENT_END
