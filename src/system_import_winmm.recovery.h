// ORIGINAL: 0x005A58D0 timeGetTime 0x005A58D0-0x005A58D6 FILE BYTE_EXACT
// LEVER: cast the fixed IAT slot to a function-pointer-pointer, call through
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x005A58D0: size      6 bytes
// seed-history 0x005A58D0: prototype
// seed-history 0x005A58D0: callers   1   call targets   0
// seed-history 0x005A58D0: kind      thunk
// seed-history 0x005A58D0: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x005A58D0: calls     (none)

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "bb098679a802afdbc6fdaac03dfe1ea81453a36f6d4768b1c1cd092b1dbe9342",
//   "component": {
//     "component_id": "vc6-system-import-winmm",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK WINMM.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/WINMM.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__timeGetTime@0", "occurrence": 154, "origin": "0x005A58D0", "spans": ["0x005A58D0-0x005A58D6"], "symbol": "timeGetTime", "unit": "WINMM.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
