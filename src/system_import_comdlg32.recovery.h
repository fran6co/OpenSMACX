// ORIGINAL: 0x006501F6 GetSaveFileNameA 0x006501F6-0x006501FC EXCLUDED S2a
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.
// ORIGINAL: 0x006501FC GetOpenFileNameA 0x006501FC-0x00650202 EXCLUDED S2a

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x006501F6: size      6 bytes
// seed-history 0x006501F6: prototype
// seed-history 0x006501F6: callers   1   call targets   0
// seed-history 0x006501F6: kind      thunk
// seed-history 0x006501F6: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x006501F6: calls     (none)
// seed-history 0x006501F6: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// seed-history 0x006501FC: size      6 bytes
// seed-history 0x006501FC: prototype
// seed-history 0x006501FC: callers   1   call targets   0
// seed-history 0x006501FC: kind      thunk
// seed-history 0x006501FC: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x006501FC: calls     (none)
// seed-history 0x006501FC: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// RECOVERY_IMPORT: v1 __imp__GetSaveFileNameA@4 comdlg32.dll name:GetSaveFileNameA

// RECOVERY_IMPORT: v1 __imp__GetOpenFileNameA@4 comdlg32.dll name:GetOpenFileNameA

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "1d82261a358979f818e7c799be43d57b48a001dfd3da457f2e04d7c9f680ec00",
//   "component": {
//     "component_id": "vc6-system-import-comdlg32",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK COMDLG32.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/COMDLG32.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__GetSaveFileNameA@4", "occurrence": 15, "origin": "0x006501F6", "spans": ["0x006501F6-0x006501FC"], "symbol": "GetSaveFileNameA", "unit": "comdlg32.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__GetOpenFileNameA@4", "occurrence": 13, "origin": "0x006501FC", "spans": ["0x006501FC-0x00650202"], "symbol": "GetOpenFileNameA", "unit": "comdlg32.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
