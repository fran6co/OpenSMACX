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
//     "infutil.c"
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
//     {"evidence": "src/vendor/zlib-1.0.2/infutil.c source definition and pinned upstream zlib 1.0.2 provenance; physical COFF build/recovery/builds/build-gyzprt2j/CMakeFiles/zlib.dir/src/vendor/zlib-1.0.2/infutil.c.obj defines _inflate_flush at section 3 (.text); installed strict comparator: COFF bytes/boundary/references pass for 0x00644930-0x00644A62; final-link selection remains unknown.", "origin": "0x00644930", "spans": ["0x00644930-0x00644A62"], "symbol": "_inflate_flush", "unit": "infutil.c"}
//   ]
// }
// RECOVERY_COMPONENT_END
