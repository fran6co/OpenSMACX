// ORIGINAL: 0x0065021A RtlUnwind 0x0065021A-0x00650220 EXCLUDED S2a
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x0065021A: size      6 bytes
// seed-history 0x0065021A: prototype void (__stdcall RtlUnwind)(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
// seed-history 0x0065021A: callers   2   call targets   0
// seed-history 0x0065021A: kind      thunk
// seed-history 0x0065021A: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x0065021A: calls     (none)
// seed-history 0x0065021A: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// RECOVERY_IMPORT: v1 __imp__RtlUnwind@16 KERNEL32.dll name:RtlUnwind

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "cd4a546f71481319aeee455aa0fb51698a094f9d3e6d8339acf445489a263c68",
//   "component": {
//     "component_id": "vc6-system-import-kernel32",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK KERNEL32.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/KERNEL32.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__RtlUnwind@16", "occurrence": 554, "origin": "0x0065021A", "spans": ["0x0065021A-0x00650220"], "symbol": "RtlUnwind", "unit": "KERNEL32.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
