// ORIGINAL: 0x00650208 DirectPlayLobbyCreateA 0x00650208-0x0065020E EXCLUDED S2a
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.
// ORIGINAL: 0x0065020E DirectPlayEnumerateA 0x0065020E-0x00650214 EXCLUDED S2a
// ORIGINAL: 0x00650214 DirectPlayCreate 0x00650214-0x0065021A EXCLUDED S2a

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x00650208: size      6 bytes
// seed-history 0x00650208: prototype 
// seed-history 0x00650208: callers   1   call targets   0
// seed-history 0x00650208: kind      thunk
// seed-history 0x00650208: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00650208: calls     (none)
// seed-history 0x00650208: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// seed-history 0x0065020E: size      6 bytes
// seed-history 0x0065020E: prototype 
// seed-history 0x0065020E: callers   2   call targets   0
// seed-history 0x0065020E: kind      thunk
// seed-history 0x0065020E: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x0065020E: calls     (none)
// seed-history 0x0065020E: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// seed-history 0x00650214: size      6 bytes
// seed-history 0x00650214: prototype HRESULT (__stdcall DirectPlayCreate)(LPGUID lpGUID, LPDIRECTPLAY* lplpDP, IUnknown* pUnk)
// seed-history 0x00650214: callers   1   call targets   0
// seed-history 0x00650214: kind      thunk
// seed-history 0x00650214: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00650214: calls     (none)
// seed-history 0x00650214: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// RECOVERY_IMPORT: v1 __imp__DirectPlayLobbyCreateA@20 DPLAYX.dll ordinal:4

// RECOVERY_IMPORT: v1 __imp__DirectPlayEnumerateA@8 DPLAYX.dll ordinal:2

// RECOVERY_IMPORT: v1 __imp__DirectPlayCreate@12 DPLAYX.dll ordinal:1

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "99ff5ab78c5189fb7090f8c2da08d0da1095833606ca422a051c83f988406739",
//   "component": {
//     "component_id": "vc6-system-import-dplayx",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK DPLAYX.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/DPLAYX.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectPlayLobbyCreateA@20", "occurrence": 7, "origin": "0x00650208", "spans": ["0x00650208-0x0065020E"], "symbol": "DirectPlayLobbyCreateA", "unit": "DPLAYX.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectPlayEnumerateA@8", "occurrence": 5, "origin": "0x0065020E", "spans": ["0x0065020E-0x00650214"], "symbol": "DirectPlayEnumerateA", "unit": "DPLAYX.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectPlayCreate@12", "occurrence": 3, "origin": "0x00650214", "spans": ["0x00650214-0x0065021A"], "symbol": "DirectPlayCreate", "unit": "DPLAYX.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
