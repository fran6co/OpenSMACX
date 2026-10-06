// Original-image bindings for this upstream dependency.
// These are verification obligations, not claims of current matching.
// Upstream source is unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6",
//   "archive_url": "https://zlib.net/fossils/zlib-1.0.2.tar.gz",
//   "component": {
//     "component_id": "zlib",
//     "delivery": "upstream_source",
//     "evidence": [
//       "src/vendor/zlib-1.0.2/PROVENANCE.md",
//       "git:f5c72d351c696fd91665a009a6d48a76fed05284"
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
//     "deflate.c",
//     "infblock.c",
//     "infcodes.c",
//     "inffast.c",
//     "inflate.c",
//     "inftrees.c",
//     "trees.c"
//   ],
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "files": [
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
//     {"archive_member": "zlib-1.0.2/infutil.h", "path": "infutil.h"},
//     {"archive_member": "zlib-1.0.2/trees.c", "path": "trees.c"},
//     {"archive_member": "zlib-1.0.2/zconf.h", "path": "zconf.h"},
//     {"archive_member": "zlib-1.0.2/zlib.h", "path": "zlib.h"},
//     {"archive_member": "zlib-1.0.2/zutil.h", "path": "zutil.h"}
//   ],
//   "bindings": [
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063cf30.c", "origin": "0x0063CF30", "spans": ["0x0063CF30-0x0063D11C"], "symbol": "_deflateInit2_", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d120.c", "origin": "0x0063D120", "spans": ["0x0063D120-0x0063D193"], "symbol": "_deflateReset", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d1a0.c", "origin": "0x0063D1A0", "spans": ["0x0063D1A0-0x0063D413"], "symbol": "_deflate", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d560.c", "origin": "0x0063D560", "spans": ["0x0063D560-0x0063D5F3"], "symbol": "_lm_init", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d600.c", "origin": "0x0063D600", "spans": ["0x0063D600-0x0063D6EF"], "symbol": "_deflate_stored", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d6f0.c", "origin": "0x0063D6F0", "spans": ["0x0063D6F0-0x0063D812"], "symbol": "_fill_window", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d820.c", "origin": "0x0063D820", "spans": ["0x0063D820-0x0063D891"], "symbol": "_read_buf", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063d8a0.c", "origin": "0x0063D8A0", "spans": ["0x0063D8A0-0x0063DACF"], "symbol": "_deflate_fast", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063dad0.c", "origin": "0x0063DAD0", "spans": ["0x0063DAD0-0x0063DC3F"], "symbol": "_longest_match", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063dc40.c", "origin": "0x0063DC40", "spans": ["0x0063DC40-0x0063DF33"], "symbol": "_deflate_slow", "unit": "deflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063df40.c", "origin": "0x0063DF40", "spans": ["0x0063DF40-0x0063DF84"], "symbol": "_inflateReset", "unit": "inflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063dfe0.c", "origin": "0x0063DFE0", "spans": ["0x0063DFE0-0x0063E0EE"], "symbol": "_inflateInit2_", "unit": "inflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_0063e110.c", "origin": "0x0063E110", "spans": ["0x0063E110-0x0063E4E8"], "symbol": "_inflate", "unit": "inflate.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00640c30.c", "origin": "0x00640C30", "spans": ["0x00640C30-0x00640E41"], "symbol": "_tr_static_init", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00640ec0.c", "origin": "0x00640EC0", "spans": ["0x00640EC0-0x00640F33"], "symbol": "_gen_codes", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00640f40.c", "origin": "0x00640F40", "spans": ["0x00640F40-0x00640FF3"], "symbol": "__tr_stored_block", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641000.c", "origin": "0x00641000", "spans": ["0x00641000-0x00641273"], "symbol": "__tr_align", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641280.c", "origin": "0x00641280", "spans": ["0x00641280-0x006414A3"], "symbol": "__tr_flush_block", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006414b0.c", "origin": "0x006414B0", "spans": ["0x006414B0-0x006416E3"], "symbol": "_build_tree", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006416f0.c", "origin": "0x006416F0", "spans": ["0x006416F0-0x006417C1"], "symbol": "_pqdownheap", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006417d0.c", "origin": "0x006417D0", "spans": ["0x006417D0-0x006419F1"], "symbol": "_gen_bitlen", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641a00.c", "origin": "0x00641A00", "spans": ["0x00641A00-0x00641A6F"], "symbol": "_build_bl_tree", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641a70.c", "origin": "0x00641A70", "spans": ["0x00641A70-0x00641B57"], "symbol": "_scan_tree", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641b60.c", "origin": "0x00641B60", "spans": ["0x00641B60-0x00641DC5"], "symbol": "_send_all_trees", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00641dd0.c", "origin": "0x00641DD0", "spans": ["0x00641DD0-0x00642345"], "symbol": "_send_tree", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642350.c", "origin": "0x00642350", "spans": ["0x00642350-0x00642477"], "symbol": "__tr_tally", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642480.c", "origin": "0x00642480", "spans": ["0x00642480-0x006428BB"], "symbol": "_compress_block", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006428c0.c", "origin": "0x006428C0", "spans": ["0x006428C0-0x00642934"], "symbol": "_set_data_type", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642960.c", "origin": "0x00642960", "spans": ["0x00642960-0x006429EB"], "symbol": "_bi_flush", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006429f0.c", "origin": "0x006429F0", "spans": ["0x006429F0-0x00642A6C"], "symbol": "_bi_windup", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642a70.c", "origin": "0x00642A70", "spans": ["0x00642A70-0x00642B07"], "symbol": "_copy_block", "unit": "trees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642bb0.c", "origin": "0x00642BB0", "spans": ["0x00642BB0-0x00642C1E"], "symbol": "_inflate_blocks_new", "unit": "infblock.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00642c20.c", "origin": "0x00642C20", "spans": ["0x00642C20-0x00643948"], "symbol": "_inflate_blocks", "unit": "infblock.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00643950.c", "origin": "0x00643950", "spans": ["0x00643950-0x00643981"], "symbol": "_inflate_blocks_free", "unit": "infblock.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00643990.c", "origin": "0x00643990", "spans": ["0x00643990-0x006439E5"], "symbol": "_inflate_trees_bits", "unit": "inftrees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_006439f0.c", "origin": "0x006439F0", "spans": ["0x006439F0-0x00643EA1"], "symbol": "_huft_build", "unit": "inftrees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00643fa0.c", "origin": "0x00643FA0", "spans": ["0x00643FA0-0x006440DE"], "symbol": "_inflate_trees_fixed", "unit": "inftrees.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00644180.c", "origin": "0x00644180", "spans": ["0x00644180-0x006448E8"], "symbol": "_inflate_codes", "unit": "infcodes.c"},
//     {"evidence": "git:e381ce551f1fd029f56b84826952d4058af2ed9c:src/vendor/zlib-1.0.2/recovered_00644a70.c", "origin": "0x00644A70", "spans": ["0x00644A70-0x00644DB2"], "symbol": "_inflate_fast", "unit": "inffast.c"},
//     {"evidence": "zlib 1.0.2; deflate.c definition `int deflateInit_(strm, level, version, stream_size)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `?deflateInit_@@YAHPAXHPBDH@Z` was historical metadata, not a current binary measurement", "origin": "0x0063CF00", "spans": ["0x0063CF00-0x0063CF25"], "symbol": "_deflateInit_", "unit": "deflate.c"},
//     {"evidence": "zlib 1.0.2; deflate.c definition `int deflateEnd (strm)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_63d4d0` was historical metadata, not a current binary measurement", "origin": "0x0063D4D0", "spans": ["0x0063D4D0-0x0063D55F"], "symbol": "_deflateEnd", "unit": "deflate.c"},
//     {"evidence": "zlib 1.0.2; inflate.c definition `int inflateEnd(z)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_63df90` was historical metadata, not a current binary measurement", "origin": "0x0063DF90", "spans": ["0x0063DF90-0x0063DFDD"], "symbol": "_inflateEnd", "unit": "inflate.c"},
//     {"evidence": "zlib 1.0.2; inflate.c definition `int inflateInit_(z, version, stream_size)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `?sub_63e0f0@@YAHHHH@Z` was historical metadata, not a current binary measurement", "origin": "0x0063E0F0", "spans": ["0x0063E0F0-0x0063E10A"], "symbol": "_inflateInit_", "unit": "inflate.c"},
//     {"evidence": "zlib 1.0.2; inftrees.c definition `int inflate_trees_free(t, z)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_644100` was historical metadata, not a current binary measurement", "origin": "0x00644100", "spans": ["0x00644100-0x0064413F"], "symbol": "_inflate_trees_free", "unit": "inftrees.c"},
//     {"evidence": "zlib 1.0.2; infcodes.c definition `inflate_codes_statef *inflate_codes_new(bl, bd, tl, td, z)`; source archive sha256 517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6; seed symbol `_sub_644140` was historical metadata, not a current binary measurement", "origin": "0x00644140", "spans": ["0x00644140-0x00644179"], "symbol": "_inflate_codes_new", "unit": "infcodes.c"}
//   ]
// }
// RECOVERY_COMPONENT_END

// ZLIB_SEED_HISTORY_BEGIN (inert provenance; not annotation facts)
// ZLIB_SEED_HISTORY: 0x0063CF00 old-symbol=?deflateInit_@@YAHPAXHPBDH@Z; historical kind=game; callers=1; target=0x0063CF30; flags=hidden;sp_ready;purged_ok
// ZLIB_SEED_HISTORY: 0x0063D4D0 old-symbol=_sub_63d4d0; historical FILE claim; callers=2; indirect sites=0x0063D4EC,0x0063D501,0x0063D516,0x0063D52B,0x0063D53D
// ZLIB_SEED_HISTORY: 0x0063DF90 old-symbol=_sub_63df90; historical kind=game; callers=2; target=0x00643950; indirect site=0x0063DFC5; flags=hidden;sp_ready;purged_ok
// ZLIB_SEED_HISTORY: 0x0063E0F0 old-symbol=?sub_63e0f0@@YAHHHH@Z; historical kind=game; caller=1; target=0x0063DFE0; flags=hidden;sp_ready;purged_ok
// ZLIB_SEED_HISTORY: 0x00644100 old-symbol=_sub_644100; historical status=MISMATCH (not a marker claim); callers=5; callback at +0x24 is a direct one-indirection function pointer, not a vtable; indirect site=0x0064412D
// ZLIB_SEED_HISTORY: 0x00644140 old-symbol=_sub_644140; historical status=MNEMONIC_ONLY (not a marker claim); caller=1; allocator/context at +0x20/+0x28; stored fields at +0x10/+0x11/+0x14/+0x18; allocator call is direct cdecl, not vtable; prior byte-register mismatch remains historical only; indirect site=0x0064414C
// ZLIB_SEED_HISTORY_END
// ORIGINAL: 0x0063CF00 deflateInit_ 0x0063CF00-0x0063CF25 BYTE_EXACT
// ORIGINAL: 0x0063D4D0 _deflateEnd 0x0063D4D0-0x0063D55F BYTE_EXACT
// ORIGINAL: 0x0063DF90 _inflateEnd 0x0063DF90-0x0063DFDD BYTE_EXACT
// ORIGINAL: 0x0063E0F0 _inflateInit_ 0x0063E0F0-0x0063E10A BYTE_EXACT
// ORIGINAL: 0x00644100 _inflate_trees_free 0x00644100-0x0064413F
// ORIGINAL: 0x00644140 _inflate_codes_new 0x00644140-0x00644179
