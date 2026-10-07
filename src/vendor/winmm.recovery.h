/*
Historical seed catalogue: git:62a0a9b6610fdb7be399b01425fb0d99f63f6ade:src/unrecovered/005a58d0.cpp
// ORIGINAL: 0x005A58D0 timeGetTime 0x005A58D0-0x005A58D6 FILE BYTE_EXACT
// LEVER: cast the fixed IAT slot to a function-pointer-pointer, call through
// working copy - scaffold materialised by --work
// size      6 bytes
// prototype 
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/

// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "bb098679a802afdbc6fdaac03dfe1ea81453a36f6d4768b1c1cd092b1dbe9342",
//   "component": {
//     "component_id": "winmm-timegettime-thunk",
//     "delivery": "system_import",
//     "evidence": [
//       "Pinned PE thunk and loader relocation at 0x005A58D0.",
//       "src/wave.cpp RECOVERY_IMPORT identifies WINMM.dll!timeGetTime.",
//       "VC6 WINMM.LIB physical short-import record selection is checked by member and occurrence."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record attribution; operating-system DLL implementation and ABI remain unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/WINMM.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte thunk; independent WINMM timeGetTime import slot; physical VC6 short-import record.", "import_symbol": "__imp__timeGetTime@0", "occurrence": 154, "origin": "0x005A58D0", "spans": ["0x005A58D0-0x005A58D6"], "symbol": "timeGetTime", "unit": "WINMM.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
