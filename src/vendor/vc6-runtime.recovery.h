// Original-image bindings for the selected precompiled runtime dependency.
// The original ledger cleanup at 0x5D4990 directly calls this function.
// Its original control flow ends at RET 0x644F20 or RET 0x644F39; the next
// independent argument-processing leaf starts at 0x644F3A. The entry and
// terminal instructions bound the span independently of the library member.
// Its direct lock/unlock and small-block-heap callees share this dependency.
// Original control-flow walks from those entries terminate at 0x647D64,
// 0x647D79, 0x647E1E and 0x648149 respectively, without indirect jumps.
// This does not identify the remaining runtime or prove final-link selection.
// The DLL/export identities below resolve through the original import directory,
// never by copying an address operand into a pointer binding.
// RECOVERY_IMPORT: v1 __imp__HeapFree@12 KERNEL32.dll name:HeapFree
// RECOVERY_IMPORT: v1 __imp__InitializeCriticalSection@4 KERNEL32.dll name:InitializeCriticalSection
// RECOVERY_IMPORT: v1 __imp__EnterCriticalSection@4 KERNEL32.dll name:EnterCriticalSection
// RECOVERY_IMPORT: v1 __imp__LeaveCriticalSection@4 KERNEL32.dll name:LeaveCriticalSection
// RECOVERY_IMPORT: v1 __imp__VirtualFree@12 KERNEL32.dll name:VirtualFree
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
//       "Original ledger cleanup 0x5D4990 calls 0x644EF2. The bounded function has the multithreaded lock/unlock and small-block-heap structure represented by this member."
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
//     {"origin": "0x00647DF4", "spans": ["0x00647DF4-0x00647E1F"], "symbol": "___sbh_find_block", "unit": "build/intel/mt_obj/sbheap.obj", "evidence": "Original _free call at 0x644F03; original traversal advances header records by 0x14, tests the 0x100000 range and returns at 0x647E1E. Header-list storage/count bindings remain separate obligations."},
//     {"origin": "0x00647E1F", "spans": ["0x00647E1F-0x0064814A"], "symbol": "___sbh_free_block", "unit": "build/intel/mt_obj/sbheap.obj", "evidence": "Original _free call at 0x644F10; the reachable original branch graph converges on the epilogue ending in RET at 0x648149. Archive sbheap.obj supplies the name interpretation, not that extent. Deferred/list/heap storage and memmove remain separate verification work."}
//   ]
// }
// RECOVERY_COMPONENT_END
