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
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "absolute_symbols": [
//     {
//       "evidence": "exsup.obj defines __except_list as a COFF absolute with value 0, storage 2. Every /GX function with unwind state emits DIR32 references to it; the linker resolves them all to 0, so the original image operands (the fs:[0] chain-head encodings) equal the candidate operands byte for byte. The TIB chain itself is loader state, never an image address.",
//       "symbol": "__except_list",
//       "unit": "build/intel/mt_obj/exsup.obj",
//       "value": 0
//     }
//   ],
//   "archive_sha256": "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b",
//   "component": {
//     "component_id": "vc6-runtime",
//     "delivery": "toolchain",
//     "evidence": [
//       "Compiler distribution used by recovery/environment; select the pinned LIBCMT.LIB artifact, not a modern CRT replacement.",
//       "Original ledger cleanup 0x5D4990 calls 0x644EF2. The bounded function has the multithreaded lock/unlock and small-block-heap structure represented by this member.",
//       "External source-family evidence in the local compiler distribution: CRT/SRC/HEAPINIT.C sha256 88aa6fcff88895f414bb53c021a318445edce60d5f2e53d66cecba2b182718d2, CRT/SRC/SBHEAP.C sha256 d8a4e6d6daa1548c193bb650b3b646e2e36d428a7cbaf33eb1ec2eec0b07837a, and CRT/SRC/WINHEAP.H sha256 439381d001d3aad0dbcad0dd9f88628df4ea9fb2cc6044226b1c306068c6639b. These identify the heap/global model; they are not proof that recompiling those files reproduces the selected archive. Proprietary source stays external."
//     ],
//     "ownership": "runtime",
//     "version": null
//   },
//   "data_sections": [
//     {
//       "evidence": "Pinned LIBCMT intrncvt.obj section 1 is the complete 0x30-byte .data section; its bytes equal image 0x006A6928-0x006A6958 and have no base relocations. Static COFF symbol anchors are _DoubleFormat at offset 0 and _FloatFormat at offset 0x18. This binds the complete section only; it asserts no per-symbol type or extent.",
//       "origin": {
//         "address": 6973736,
//         "address_space": "pe:va",
//         "image_sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
//       },
//       "section": 1,
//       "section_name": ".data",
//       "span": {
//         "end": 6973784,
//         "start": 6973736
//       },
//       "symbols": [
//         {
//           "name": "_DoubleFormat",
//           "offset": 0,
//           "storage": 3
//         },
//         {
//           "name": "_FloatFormat",
//           "offset": 24,
//           "storage": 3
//         }
//       ],
//       "unit": "../build/intel/mt_obj/intrncvt.obj"
//     }
//   ],
//   "kind": "toolchain_component",
//   "license_evidence": "Proprietary Microsoft runtime supplied by the local compiler distribution. The archive stays external and is not redistributed here; redistribution rights have not been established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Original direct call from 0x5D4990 and original control-flow terminal returns at 0x644F20/0x644F39; library symbol identifies the member, not the original boundary. Lock/heap function and import identities are declared separately; CRT heap storage still needs independent proof.", "origin": "0x00644EF2", "spans": ["0x00644EF2-0x00644F3A"], "symbol": "_free", "unit": "build/intel/mt_obj/free.obj"},
//     {"evidence": "Original _free call at 0x644EFD, recursive critical-section initialization and terminal RET at 0x647D64 independently bound this entry; archive mlock.obj supplies its name interpretation. Lock-table storage and allocation/error callees remain separate obligations.", "origin": "0x00647D04", "spans": ["0x00647D04-0x00647D65"], "symbol": "__lock", "unit": "build/intel/mt_obj/mlock.obj"},
//     {"evidence": "Original _free calls at 0x644F17 and 0x644F23 and terminal RET at 0x647D79; the body selects a lock-table entry and calls LeaveCriticalSection. The shared lock-table object still needs independent storage proof.", "origin": "0x00647D65", "spans": ["0x00647D65-0x00647D7A"], "symbol": "__unlock", "unit": "build/intel/mt_obj/mlock.obj"},
//     {"evidence": "Original startup call at 0x646CF6; HeapCreate result is stored at 0x9C11C4, then the direct call at 0x647D9A initializes the small-block heap. Failure destroys the created handle. The original branch graph terminates at RET 0x647DB1 or 0x647DB5, without a local indirect jump; these bound the body independently of the archive. Common heap-handle storage is a separate obligation.", "origin": "0x00647D7A", "spans": ["0x00647D7A-0x00647DB6"], "symbol": "__heap_init", "unit": "build/intel/mt_obj/heapinit.obj"},
//     {"evidence": "Original direct call at 0x647D9A; HeapAlloc receives the heap handle and 0x140 bytes, and the result establishes the list/scan pointers, zero deferred pointer/count and capacity 16. Failure RET 0x647DD2 and success RET 0x647DF3 bound the original branch graph. The local source HEADER type explains 16 records of 0x14 bytes; global allocations and lifetime remain separate proof obligations.", "origin": "0x00647DB6", "spans": ["0x00647DB6-0x00647DF4"], "symbol": "___sbh_heap_init", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original _free call at 0x644F03; original traversal advances header records by 0x14, tests the 0x100000 range and returns at 0x647E1E. Header-list storage/count bindings remain separate obligations.", "origin": "0x00647DF4", "spans": ["0x00647DF4-0x00647E1F"], "symbol": "___sbh_find_block", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original _free call at 0x644F10; the reachable original branch graph converges on the epilogue ending in RET at 0x648149. Archive sbheap.obj supplies the name interpretation, not that extent. Deferred/list/heap storage and memmove remain separate verification work.", "origin": "0x00647E1F", "spans": ["0x00647E1F-0x0064814A"], "symbol": "___sbh_free_block", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original calls at 0x646F07, 0x6470FA and 0x6471A3 enter this small-block allocation routine. Its prologue at 0x64814A follows the free-block RET at 0x648149; its branch graph converges on the epilogue ending in RET at 0x648452. The next independent region-allocation prologue is at 0x648453. The unique pinned sbheap.obj member defines this symbol at offset zero in its 777-byte code section; all non-relocation bytes agree with this independently bounded image interval. SBHEAP.C supplies the heap ownership/type interpretation. Nine COFF relocation records remain reference obligations, including the header-list/count/scan/defer and deferred-group-index storage; masked correspondence does not prove original archive selection or final-link/runtime equivalence.", "origin": "0x0064814A", "spans": ["0x0064814A-0x00648453"], "symbol": "___sbh_alloc_block", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original small-block allocation calls this entry at 0x648216. Its distinct prologue begins at 0x648453 and its terminal RET at 0x648503 precedes the independent group-allocation prologue. The unique pinned sbheap.obj symbol starts at offset zero in a 177-byte code section whose non-relocation bytes agree with this original interval. SBHEAP.C supplies the region-allocation interpretation. Heap/import/storage targets, original archive selection and final-link/runtime equivalence remain separate obligations; this records bounded dependency ownership, not application recovery or a matching claim.", "origin": "0x00648453", "spans": ["0x00648453-0x00648504"], "symbol": "___sbh_alloc_new_region", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original small-block allocation calls this entry at 0x648225. Its distinct prologue begins at 0x648504 and its terminal RET at 0x6485FE precedes the independent resize prologue. The unique pinned sbheap.obj symbol starts at offset zero in a 251-byte code section whose non-relocation bytes agree with this original interval. SBHEAP.C supplies the group-allocation interpretation. Relocation targets, original archive selection and final-link/runtime equivalence remain separate obligations; this records bounded dependency ownership, not application recovery or a matching claim.", "origin": "0x00648504", "spans": ["0x00648504-0x006485FF"], "symbol": "___sbh_alloc_new_group", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "Original heap reallocation calls this entry at 0x647192. Its distinct prologue begins at 0x6485FF and its branch graph ends in RET at 0x6488F4 before a new independent prologue at 0x6488F5. The unique pinned sbheap.obj symbol starts at offset zero in a 758-byte code section with no relocations; every byte agrees with this independently bounded original interval. SBHEAP.C supplies the small-block resize interpretation. Original archive selection, final-link retention and runtime equivalence remain unproved; complete member-byte correspondence is not whole dependency or game acceptance.", "origin": "0x006485FF", "spans": ["0x006485FF-0x006488F5"], "symbol": "___sbh_resize_block", "unit": "build/intel/mt_obj/sbheap.obj"},
//     {"evidence": "The compiler-generated EH thunk for ??0Flic@@QAE@XZ at 0x0066332B loads funcinfo 0x67FFB0 into eax and jumps here; every C++ function with unwind state reaches this entry the same way. The 54-byte body matches trnsctrl.obj's member bytes with one relocation (funcinfo dispatch); terminal RET at 0x0064500B bounds it. Archive member supplies the name; try-level iteration and unwind semantics remain unproved.", "origin": "0x00644FD6", "spans": ["0x00644FD6-0x0064500C"], "symbol": "___CxxFrameHandler", "unit": "build/intel/mt_obj/trnsctrl.obj"},
//     {"evidence": "Original direct call from ?close@Flic@@QAEXXZ 0x00629DF0, its only call target. The 49-byte body checks the stream mode flag (0x40), locks through 0x00647C37, calls __fclose_lk at 0x006455C9 and unlocks; terminal RET at 0x006455C8. Archive fclose.obj member bytes match with relocation slots only.", "origin": "0x00645598", "spans": ["0x00645598-0x006455C9"], "symbol": "_fclose", "unit": "build/intel/mt_obj/fclose.obj"},
//     {"evidence": "Original calls from decode_fli_copy 0x0062AB80 (twice), ?decode_frame@Flic@@QAEHHH@Z 0x0062A070 and the byte-run decoder family. The 47-byte body locks through 0x00647C37 then tail-calls __fread_lk; terminal RET at 0x006461A6. Archive fread.obj member bytes match with relocation slots only.", "origin": "0x00646178", "spans": ["0x00646178-0x006461A7"], "symbol": "_fread", "unit": "build/intel/mt_obj/fread.obj"},
//     {"evidence": "Original calls from rewind 0x0062A280, UNK2 0x0062A210, decode_frame 0x0062A070, decode_fli_copy 0x0062AB80 and the byte-run decoder family. The 44-byte framed body locks through 0x00647C37, tail-calls __fseek_lk at 0x0064735C and unlocks; terminal RET at 0x0064735B. Archive fseek.obj member bytes match with relocation slots only.", "origin": "0x00647330", "spans": ["0x00647330-0x0064735C"], "symbol": "_fseek", "unit": "build/intel/mt_obj/fseek.obj"},
//     {"evidence": "Original calls from UNK3 0x0062A2C0, decode_byte_run 0x0062A500 and five more flic-family sites (the dispatcher at 0x0062B670 among them). The 11-byte body is the cdq/xor/sub magnitude idiom plus ret; archive abs.obj member bytes match byte-exactly with no relocations.", "origin": "0x00644F3A", "spans": ["0x00644F3A-0x00644F45"], "symbol": "_abs", "unit": "build/intel/mt_obj/abs.obj"},
//     {"evidence": "Original calls from UNK3 0x0062A2C0 and decode_chunk 0x0062ACD0 (twice each in the recorded call lists). The 34-byte framed body locks through 0x00647C37 and tail-calls __ftell_lk; archive ftell.obj member bytes match with relocation slots only.", "origin": "0x00647920", "spans": ["0x00647920-0x00647942"], "symbol": "_ftell", "unit": "build/intel/mt_obj/ftell.obj"},
//     {"evidence": "Original calls from decode_byte_run 0x0062A500 and decode_byte_run_trans 0x0062AF40 (and the byte-match catalogue's 0x00644F3A-family neighbors). The 88-byte al-fill loop is byte-exact against memset.obj's plain code section - the member is not a COMDAT, so the pinned binding carries the identity the product COMDAT boundary would prove; zero relocations.", "origin": "0x006465F0", "spans": ["0x006465F0-0x00646648"], "symbol": "_memset", "unit": "build/intel/mt_obj/memset.obj"},
//     {"evidence": "Original calls from Midi::set_fname 0x004C7D80-family and the wider string-using families. The 123-byte scan loop is byte-exact against strlen.obj's plain code section with no relocations.", "origin": "0x006453E0", "spans": ["0x006453E0-0x0064545B"], "symbol": "_strlen", "unit": "build/intel/mt_obj/strlen.obj"},
//     {"evidence": "Faction diplomacy calls the two-character-pointer comparator at this original entry. Independent original control flow returns at6456A2 or6456A9, with the last reachable loop-back at6456E2;6456E3 is unreachable alignment and6456E4 begins the next independent EH prologue. The pinned archive member defines _strcmp at section offset0; its complete132-byte code section equals this original interval with no relocations. This identifies the runtime dependency, not an application recovery or a full-link/game acceptance claim.", "origin": "0x00645660", "spans": ["0x00645660-0x006456E4"], "symbol": "_strcmp", "unit": "build/intel/mt_obj/strcmp.obj"},
//     {"evidence": "Original calls from Midi::set_fname and the file-naming families. VC6 emits _strcpy and _strcat in one strcat.obj section at values 0x0/0x10; strcpy's 16 bytes are byte-exact at 0x00645460 with no relocations, bounded by _strcat's value.", "origin": "0x00645460", "spans": ["0x00645460-0x00645470"], "symbol": "_strcpy", "unit": "build/intel/mt_obj/strcat.obj"},
//     {"evidence": "Global operator delete: 11-byte tail-jump to free with the free slot at offset 5 relocated. Non-relocation bytes match the image at 0x0064557F; operator new follows immediately at 0x0064558A, bounding the extent.", "origin": "0x0064557F", "spans": ["0x0064557F-0x0064558A"], "symbol": "??3@YAXPAX@Z", "unit": "build/intel/mt_obj/delete.obj"},
//     {"evidence": "Global operator new: 14-byte body whose malloc call at offset 7 is relocated. Non-relocation bytes match the image at 0x0064558A; the extent ends before the CRT neighbor at 0x00645598.", "origin": "0x0064558A", "spans": ["0x0064558A-0x00645598"], "symbol": "??2@YAPAXI@Z", "unit": "build/intel/mt_obj/new.obj"},
//     {"evidence": "Unique pinned LIBCMT function symbol; original IDA function extent is independently bounded.", "origin": "0x0064D5B9", "spans": ["0x0064D5B9-0x0064D725"], "symbol": "__ld12cvt", "unit": "../build/intel/mt_obj/intrncvt.obj"},
//     {"evidence": "Original seed identity sub_64d725 retained; pinned archive wrapper code and descriptor/callee relocations agree.", "origin": "0x0064D725", "spans": ["0x0064D725-0x0064D73B"], "symbol": "__ld12tod", "unit": "../build/intel/mt_obj/intrncvt.obj"},
//     {"evidence": "Original seed identity sub_64d73b retained; pinned archive wrapper code and descriptor/callee relocations agree.", "origin": "0x0064D73B", "spans": ["0x0064D73B-0x0064D751"], "symbol": "__ld12tof", "unit": "../build/intel/mt_obj/intrncvt.obj"},
//     {"evidence": "Original seed identity sub_64d751 retained; parser and double conversion relocation order agrees.", "origin": "0x0064D751", "spans": ["0x0064D751-0x0064D77E"], "symbol": "__atodbl", "unit": "../build/intel/mt_obj/intrncvt.obj"},
//     {"evidence": "Original seed identity sub_64d77e retained; parser and float conversion relocation order agrees.", "origin": "0x0064D77E", "spans": ["0x0064D77E-0x0064D7AB"], "symbol": "__atoflt", "unit": "../build/intel/mt_obj/intrncvt.obj"},
//     {"evidence": "Pinned body instructions end with RET at 0x0064F9C4. At 0x0064F587, bounded EAX 0..11 indexes a 12-DWORD switch table at 0x0064F9C5-0x0064F9F5; archive .text section 1 has $L679 at offset 0x4A1 and twelve corresponding DIR32 label relocations in offsets 0x4A1..0x4D0. The mapped physical span deliberately includes 1185 code bytes plus this 48-byte associated table. The next region starts at 0x0064F9F5 with a distinct frame prologue.", "origin": "0x0064F524", "spans": ["0x0064F524-0x0064F9F5"], "symbol": "___strgtold12", "unit": "../build/intel/mt_obj/strgtold.obj"},
//     {"evidence": "Original runtime identity and full FILE span 0x006456E4-0x00645764; this pinned LIBCMT member owns the decorated EH vector destructor iterator and its original target at 0x00645764 is __ArrayUnwind. This records dependency ownership only; original final-link selection, complete relocation closure, and runtime/unwind equivalence remain unknown.", "origin": "0x006456E4", "spans": ["0x006456E4-0x00645764"], "symbol": "??_M@YGXPAXIHP6EX0@Z@Z", "unit": "build/intel/mt_obj/ehvecdtr.obj"},
//     {"evidence": "Original runtime identity and full FILE span 0x006457C2-0x00645844; this pinned LIBCMT member owns the decorated EH vector constructor iterator. The original Wave_Device typed member array reaches this helper. This records dependency ownership only; original final-link selection and runtime/unwind equivalence remain unknown.", "origin": "0x006457C2", "spans": ["0x006457C2-0x00645844"], "symbol": "??_L@YGXPAXIHP6EX0@Z1@Z", "unit": "build/intel/mt_obj/ehvecctr.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x6458a0..0x6458c7 and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x006458A0", "spans": ["0x006458A0-0x006458C7"], "symbol": "_strrchr", "unit": "build/intel/mt_obj/strrchr.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x6458c8..0x6458ef and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x006458C8", "spans": ["0x006458C8-0x006458EF"], "symbol": "__ftol", "unit": "../build/intel/mt_obj/ftol.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x6458f0..0x645928 and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x006458F0", "spans": ["0x006458F0-0x00645928"], "symbol": "_strncmp", "unit": "build/intel/mt_obj/strncmp.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x645c70..0x645d1c and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x00645C70", "spans": ["0x00645C70-0x00645D1C"], "symbol": "_memcmp", "unit": "build/intel/mt_obj/memcmp.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x645e90..0x645f8e and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x00645E90", "spans": ["0x00645E90-0x00645F8E"], "symbol": "_strncpy", "unit": "build/intel/mt_obj/strncpy.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x6473f0..0x647495 and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x006473F0", "spans": ["0x006473F0-0x00647495"], "symbol": "_memchr", "unit": "build/intel/mt_obj/memchr.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x6494a6..0x6494d1 and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. The original call operand targets the separately bound _free entry at0x644EF2. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x006494A6", "spans": ["0x006494A6-0x006494D1"], "symbol": "__freebuf", "unit": "build/intel/mt_obj/_freebuf.obj"},
//     {"evidence": "Pinned IDA entry/extent 0x64c8b3..0x64c8c5 and original terminal/branch instructions independently bound this runtime entry. The unique physical LIBCMT member supplies the symbol interpretation; its complete function section agrees with this interval. This binds dependency correspondence only; original final-link selection and runtime equivalence remain separate obligations.", "origin": "0x0064C8B3", "spans": ["0x0064C8B3-0x0064C8C5"], "symbol": "__frnd", "unit": "../build/intel/mt_obj/frnd.obj"}
//   ]
// }
// RECOVERY_COMPONENT_END
// ORIGINAL: 0x0064D725 sub_64d725 0x0064D725-0x0064D73B
// ORIGINAL: 0x0064D73B sub_64d73b 0x0064D73B-0x0064D751 FILE
// ORIGINAL: 0x0064D751 sub_64d751 0x0064D751-0x0064D77E
// ORIGINAL: 0x0064D77E sub_64d77e 0x0064D77E-0x0064D7AB

// Historical seed notes below are inert prose, not active source metadata:
// Superseded seed src/recovered/0064d725.cpp was retired; Git preserves its source history:
// Historical seed note (inert): catalogued seed symbol hypothesis: ?sub_64d725@@YAHHH@Z (historical only; not the current toolchain symbol)
// Historical seed note (inert): the seed declared sub_64d725 as __cdecl with two int parameters and forwarded both to __ld12cvt, appending the double descriptor address 0x006A6928 as a third argument; its note cites [esp+0xc] as the evidence for two incoming int-sized parameters
// Historical seed note (inert): catalogued seed call target: 0x0064D5B9 (__ld12cvt)
// Historical seed note (inert): historical seed prose called this a byte-exact Mizuchi match, while its current parsed marker had no BYTE_EXACT or SEMANTIC token
// Superseded seed src/recovered/units/0064d73b.cpp was retired; Git preserves its source history:
// Historical seed note (inert): catalogued seed symbol hypothesis: ?sub_64d73b@@YAHHH@Z (historical only; not the current toolchain symbol)
// Historical seed note (inert): the seed body forwarded two int parameters to __ld12cvt and appended its fixed pointer 0x006A6940 (the pinned _FloatFormat offset anchor)
// Historical seed note (inert): catalogued seed call target: 0x0064D5B9 (__ld12cvt)
// Historical seed note (inert): PRESERVED UNIT - measured BYTE_EXACT.
// Historical seed note (inert): measured tier  BYTE_EXACT
// Historical seed note (inert): the preceding BYTE_EXACT statements are retained historical prose only; the parsed ORIGINAL marker had no BYTE_EXACT or SEMANTIC token
// Superseded seed src/recovered/0064d751.cpp was retired; Git preserves its source history:
// Historical seed note (inert): catalogued seed symbol hypothesis: ?sub_64d751@@YAHPAX0@Z (historical only; not the current toolchain symbol)
// Historical seed note (inert): the seed declared a two-pointer __cdecl wrapper; it called ___strgtold12 with a 12-byte local, the address of its second argument, that argument, and four zero arguments, then called sub_64d725 with the local result and first input
// Historical seed note (inert): catalogued seed call targets: 0x0064F524 (___strgtold12) and 0x0064D725 (sub_64d725)
// Superseded seed src/recovered/0064d77e.cpp was retired; Git preserves its source history:
// Historical seed note (inert): catalogued seed symbol hypothesis: ?sub_64d77e@@YAHPAX0@Z (historical only; not the current toolchain symbol)
// Historical seed note (inert): the seed declared a two-pointer __cdecl wrapper; it called ___strgtold12 with a three-DWORD local, the address of its second argument, that argument, and four zero arguments, then called sub_64d73b with the local result and first input
// Historical seed note (inert): catalogued seed call targets: 0x0064F524 (___strgtold12) and 0x0064D73B (sub_64d73b)
/*
Purpose: Retain the original CRT vector-destructor runtime identity ??_M:
         destroy `count` elements of `size` bytes back to front, with the
         runtime's __ArrayUnwind exception behavior. The handwritten
         project loop was removed after its only direct source use moved
         to a typed WaveControlGroup member array. Runtime-provider
         equivalence to the pinned image remains unqualified.

// Dependency-owned iterator identity, not a project leaf implementation.
// Preserve provider selection, relocation closure and unwind-equivalence debt.
// ORIGINAL: 0x006456E4 ??_M@YGXPAXIHP6EX0@Z@Z 0x006456E4-0x00645764 FILE
// Historical project symbol: ?VectorDtorIterator@@YGXPAXIHP6IX0@Z@Z
// Historical project-loop location: git:4ba53acef56137f1aa04151f648ea360681ee511:src/vector_teardown.cpp; no active
// project-defined VectorDtorIterator implementation remains.
// TRIED: explicit __thiscall keyword on a free-function-pointer typedef
//            (C4234 reserved keyword); member-function-pointer/union shim
//            instead. MISMATCH #12 'and' vs 'mov' remains open.
// size      128 bytes
// prototype void (__stdcall ??_M@YGXPAXIHP6EX0@Z@Z)(void*, unsigned int size, int count, void (__thiscall *)(void*))
// callers   160   call targets   2
// kind      library
// flags     lib;frame;sp_ready;purged_ok
// calls     0x006456E4 0x00645764
// indirect  0x00645726
Return Value: n/a
Status: Complete
*/
/*
Purpose: Retain the original CRT vector-constructor runtime identity ??_L:
         construct `count` elements of `size` bytes front to back and
         unwind the constructed prefix through __ArrayUnwind if a
         constructor throws. The handwritten project loop was removed
         after its only direct source use moved to a typed
         WaveControlGroup member array. Runtime-provider equivalence to
         the pinned image remains unqualified.

// Dependency-owned iterator identity, not a project leaf implementation.
// Preserve provider selection, relocation closure and unwind-equivalence debt.
// ORIGINAL: 0x006457C2 ??_L@YGXPAXIHP6EX0@Z1@Z 0x006457C2-0x00645844 FILE
// Historical project symbol: ?VectorCtorIterator@@YGXPAXIHP6IX0@Z1@Z
// Historical project-loop location: git:4ba53acef56137f1aa04151f648ea360681ee511:src/vector_teardown.cpp; no active
// project-defined VectorCtorIterator implementation remains.
// size      130 bytes
// prototype void (__stdcall ??_L@YGXPAXIHP6EX0@Z1@Z)(void*, unsigned int size, int count, void (__thiscall *)(void*), void (__thiscall *)(void*))
// callers   132   call targets   2
// kind      library
// flags     lib;frame;hidden;sp_ready;purged_ok
// calls     0x00645764 0x006457C2
// indirect  0x006457FD
Return Value: n/a
Status: Complete
*/
