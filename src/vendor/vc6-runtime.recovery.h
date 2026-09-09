// Original-image bindings for the selected precompiled runtime dependency.
// The original ledger cleanup at 0x5D4990 directly calls this function.
// Its original control flow ends at RET 0x644F20 or RET 0x644F39; the next
// independent argument-processing leaf starts at 0x644F3A. The entry and
// terminal instructions bound the span independently of the library member.
// This does not identify the remaining runtime or prove final-link selection.
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
//     {"origin": "0x00644EF2", "spans": ["0x00644EF2-0x00644F3A"], "symbol": "_free", "unit": "build/intel/mt_obj/free.obj", "evidence": "Original direct call from 0x5D4990 and original control-flow terminal returns at 0x644F20/0x644F39; library symbol identifies the member, not the original boundary. References to locks, small-block heap, CRT heap storage and HeapFree still require independent bindings."}
//   ]
// }
// RECOVERY_COMPONENT_END
