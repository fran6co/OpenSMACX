// Original-image bindings for the selected precompiled runtime dependency.
// The original ledger cleanup at 0x5D4990 directly calls this function.
// Its original control flow ends at RET 0x644F20 or RET 0x644F39; the next
// independent argument-processing leaf starts at 0x644F3A. The entry and
// terminal instructions bound the span independently of the library member.
// Its direct lock/unlock and small-block-heap callees share this dependency.
// Original control-flow walks from those entries terminate at 0x647D64,
// 0x647D79, 0x647E1E and 0x648149 respectively, without indirect jumps.
// Startup calls _heap_init at 0x646CF6; it creates the heap handle before
// calling __sbh_heap_init at 0x647D9A. Their failure/success returns bound
// 0x647D7A-0x647DB6 and 0x647DB6-0x647DF4 independently of object sizes.
// The local CRT sources are ownership/type evidence, not application code to
// recreate: HEAPINIT.C declares HANDLE _crtheap; SBHEAP.C/WINHEAP.H declare
// the small-block HEADER list, scan/defer pointers and count/capacity values.
// Original initialization establishes _crtheap at 0x9C11C4 (HeapCreate result),
// __sbh_pHeaderList at 0x9C11C0 (HeapAlloc result, 16 headers of 0x14 bytes),
// __sbh_pHeaderScan at 0x9C11B4 (the same list), __sbh_pHeaderDefer at
// 0x9C11B8 and __sbh_cntHeaderList at 0x9C11BC (zero), and
// __sbh_sizeHeaderList at 0x9C11AC (16). These identify CRT-owned storage;
// they are NOT aliases to export as application relocation targets. The
// corresponding COFF common requests need selected-allocation evidence, not
// fabricated C++ definitions or an extent inferred from neighboring symbols.
// This does not identify the remaining runtime or prove final-link selection.
// The DLL/export identities below resolve through the original import directory,
// never by copying an address operand into a pointer binding.
// RECOVERY_IMPORT: v1 __imp__HeapFree@12 KERNEL32.dll name:HeapFree
// RECOVERY_IMPORT: v1 __imp__InitializeCriticalSection@4 KERNEL32.dll name:InitializeCriticalSection
// RECOVERY_IMPORT: v1 __imp__EnterCriticalSection@4 KERNEL32.dll name:EnterCriticalSection
// RECOVERY_IMPORT: v1 __imp__LeaveCriticalSection@4 KERNEL32.dll name:LeaveCriticalSection
// RECOVERY_IMPORT: v1 __imp__VirtualFree@12 KERNEL32.dll name:VirtualFree
// RECOVERY_IMPORT: v1 __imp__HeapCreate@12 KERNEL32.dll name:HeapCreate
// RECOVERY_IMPORT: v1 __imp__HeapDestroy@4 KERNEL32.dll name:HeapDestroy
// RECOVERY_IMPORT: v1 __imp__HeapAlloc@12 KERNEL32.dll name:HeapAlloc
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "schema_version": 1,
//   "kind": "toolchain_component",
//   "component": {
//     "component_id": "vc6-runtime",
//     "ownership": "runtime",
//     "delivery": "toolchain",
//     "evidence": [
//       "Compiler distribution used by recovery/environment; select the pinned LIBCMT.LIB artifact, not a modern CRT replacement.",
//       "Original ledger cleanup 0x5D4990 calls 0x644EF2. The bounded function has the multithreaded lock/unlock and small-block-heap structure represented by this member.",
//       "External source-family evidence in the local compiler distribution: CRT/SRC/HEAPINIT.C sha256 88aa6fcff88895f414bb53c021a318445edce60d5f2e53d66cecba2b182718d2, CRT/SRC/SBHEAP.C sha256 d8a4e6d6daa1548c193bb650b3b646e2e36d428a7cbaf33eb1ec2eec0b07837a, and CRT/SRC/WINHEAP.H sha256 439381d001d3aad0dbcad0dd9f88628df4ea9fb2cc6044226b1c306068c6639b. These identify the heap/global model; they are not proof that recompiling those files reproduces the selected archive. Proprietary source stays external."
//     ],
//     "version": null
//   },
//   "archive_sha256": "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b",
//   "license_evidence": "Proprietary Microsoft runtime supplied by the local compiler distribution. The archive stays external and is not redistributed here; redistribution rights have not been established.",
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"origin": "0x00644EF2", "spans": ["0x00644EF2-0x00644F3A"], "symbol": "_free", "unit": "build/intel/mt_obj/free.obj", "evidence": "Original direct call from 0x5D4990 and original control-flow terminal returns at 0x644F20/0x644F39; library symbol identifies the member, not the original boundary. Lock/heap function and import identities are declared separately; CRT heap storage still needs independent proof."},
//     {"origin": "0x00647D04", "spans": ["0x00647D04-0x00647D65"], "symbol": "__lock", "unit": "build/intel/mt_obj/mlock.obj", "evidence": "Original _free call at 0x644EFD, recursive critical-section initialization and terminal RET at 0x647D64 independently bound this entry; archive mlock.obj supplies its name interpretation. Lock-table storage and allocation/error callees remain separate obligations."},
//     {"origin": "0x00647D65", "spans": ["0x00647D65-0x00647D7A"], "symbol": "__unlock", "unit": "build/intel/mt_obj/mlock.obj", "evidence": "Original _free calls at 0x644F17 and 0x644F23 and terminal RET at 0x647D79; the body selects a lock-table entry and calls LeaveCriticalSection. The shared lock-table object still needs independent storage proof."},
//     {"origin": "0x00647D7A", "spans": ["0x00647D7A-0x00647DB6"], "symbol": "__heap_init", "unit": "build/intel/mt_obj/heapinit.obj", "evidence": "Original startup call at 0x646CF6; HeapCreate result is stored at 0x9C11C4, then the direct call at 0x647D9A initializes the small-block heap. Failure destroys the created handle. The original branch graph terminates at RET 0x647DB1 or 0x647DB5, without a local indirect jump; these bound the body independently of the archive. Common heap-handle storage is a separate obligation."},
//     {"origin": "0x00647DB6", "spans": ["0x00647DB6-0x00647DF4"], "symbol": "___sbh_heap_init", "unit": "build/intel/mt_obj/sbheap.obj", "evidence": "Original direct call at 0x647D9A; HeapAlloc receives the heap handle and 0x140 bytes, and the result establishes the list/scan pointers, zero deferred pointer/count and capacity 16. Failure RET 0x647DD2 and success RET 0x647DF3 bound the original branch graph. The local source HEADER type explains 16 records of 0x14 bytes; global allocations and lifetime remain separate proof obligations."},
//     {"origin": "0x00647DF4", "spans": ["0x00647DF4-0x00647E1F"], "symbol": "___sbh_find_block", "unit": "build/intel/mt_obj/sbheap.obj", "evidence": "Original _free call at 0x644F03; original traversal advances header records by 0x14, tests the 0x100000 range and returns at 0x647E1E. Header-list storage/count bindings remain separate obligations."},
//     {"origin": "0x00647E1F", "spans": ["0x00647E1F-0x0064814A"], "symbol": "___sbh_free_block", "unit": "build/intel/mt_obj/sbheap.obj", "evidence": "Original _free call at 0x644F10; the reachable original branch graph converges on the epilogue ending in RET at 0x648149. Archive sbheap.obj supplies the name interpretation, not that extent. Deferred/list/heap storage and memmove remain separate verification work."},
//     {"origin": "0x00644FD6", "spans": ["0x00644FD6-0x0064500C"], "symbol": "___CxxFrameHandler", "unit": "build/intel/mt_obj/trnsctrl.obj", "evidence": "The compiler-generated EH thunk for ??0Flic@@QAE@XZ at 0x0066332B loads funcinfo 0x67FFB0 into eax and jumps here; every C++ function with unwind state reaches this entry the same way. The 54-byte body matches trnsctrl.obj's member bytes with one relocation (funcinfo dispatch); terminal RET at 0x0064500B bounds it. Archive member supplies the name; try-level iteration and unwind semantics remain unproved."},
//     {"origin": "0x00645598", "spans": ["0x00645598-0x006455C9"], "symbol": "_fclose", "unit": "build/intel/mt_obj/fclose.obj", "evidence": "Original direct call from ?close@Flic@@QAEXXZ 0x00629DF0, its only call target. The 49-byte body checks the stream mode flag (0x40), locks through 0x00647C37, calls __fclose_lk at 0x006455C9 and unlocks; terminal RET at 0x006455C8. Archive fclose.obj member bytes match with relocation slots only."},
//     {"origin": "0x00646178", "spans": ["0x00646178-0x006461A7"], "symbol": "_fread", "unit": "build/intel/mt_obj/fread.obj", "evidence": "Original calls from decode_fli_copy 0x0062AB80 (twice), ?decode_frame@Flic@@QAEHHH@Z 0x0062A070 and the byte-run decoder family. The 47-byte body locks through 0x00647C37 then tail-calls __fread_lk; terminal RET at 0x006461A6. Archive fread.obj member bytes match with relocation slots only."},
//     {"origin": "0x00647330", "spans": ["0x00647330-0x0064735C"], "symbol": "_fseek", "unit": "build/intel/mt_obj/fseek.obj", "evidence": "Original calls from rewind 0x0062A280, UNK2 0x0062A210, decode_frame 0x0062A070, decode_fli_copy 0x0062AB80 and the byte-run decoder family. The 44-byte framed body locks through 0x00647C37, tail-calls __fseek_lk at 0x0064735C and unlocks; terminal RET at 0x0064735B. Archive fseek.obj member bytes match with relocation slots only."},
//     {"origin": "0x00644F3A", "spans": ["0x00644F3A-0x00644F45"], "symbol": "_abs", "unit": "build/intel/mt_obj/abs.obj", "evidence": "Original calls from UNK3 0x0062A2C0, decode_byte_run 0x0062A500 and five more flic-family sites (the dispatcher at 0x0062B670 among them). The 11-byte body is the cdq/xor/sub magnitude idiom plus ret; archive abs.obj member bytes match byte-exactly with no relocations."},
//     {"origin": "0x00647920", "spans": ["0x00647920-0x00647942"], "symbol": "_ftell", "unit": "build/intel/mt_obj/ftell.obj", "evidence": "Original calls from UNK3 0x0062A2C0 and decode_chunk 0x0062ACD0 (twice each in the recorded call lists). The 34-byte framed body locks through 0x00647C37 and tail-calls __ftell_lk; archive ftell.obj member bytes match with relocation slots only."},
//     {"origin": "0x006465F0", "spans": ["0x006465F0-0x00646648"], "symbol": "_memset", "unit": "build/intel/mt_obj/memset.obj", "evidence": "Original calls from decode_byte_run 0x0062A500 and decode_byte_run_trans 0x0062AF40 (and the byte-match catalogue's 0x00644F3A-family neighbors). The 88-byte al-fill loop is byte-exact against memset.obj's plain code section - the member is not a COMDAT, so the pinned binding carries the identity the product COMDAT boundary would prove; zero relocations."}
//   ],
//   "absolute_symbols": [
//     {"symbol": "__except_list", "value": "0x00000000", "unit": "build/intel/mt_obj/exsup.obj", "evidence": "exsup.obj defines __except_list as a COFF absolute with value 0, storage 2. Every /GX function with unwind state emits DIR32 references to it; the linker resolves them all to 0, so the original image operands (the fs:[0] chain-head encodings) equal the candidate operands byte for byte. The TIB chain itself is loader state, never an image address."}
//   ]
// }
// RECOVERY_COMPONENT_END
