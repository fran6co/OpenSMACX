// ORIGINAL: 0x005D44FC DirectDrawCreate 0x005D44FC-0x005D4502 FILE
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason: its fixed-address bindings have no naming evidence. image_data.py reads 1 of them and every one is zero at load - runtime-filled lookup tables with no .data content, no string, no arithmetic identity beyond 'a table indexed here' - and none is modelled anywhere in src. The homing-first rule does not ask for them to be invented: an unclaimed body with evidence-free bindings waits for the class pass that measures it in context.
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.
// ORIGINAL: 0x00650202 DirectDrawEnumerateA 0x00650202-0x00650208 EXCLUDED S2a

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x005D44FC: size      6 bytes
// seed-history 0x005D44FC: prototype HRESULT (__stdcall DirectDrawCreate)(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
// seed-history 0x005D44FC: callers   3   call targets   0
// seed-history 0x005D44FC: kind      thunk
// seed-history 0x005D44FC: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x005D44FC: calls     (none)
// seed-history 0x00650202: size      6 bytes
// seed-history 0x00650202: prototype HRESULT (__stdcall DirectDrawEnumerateA)(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext)
// seed-history 0x00650202: callers   1   call targets   0
// seed-history 0x00650202: kind      thunk
// seed-history 0x00650202: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00650202: calls     (none)
// seed-history 0x00650202: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "329c7de4664532620fba0e1734e48069a9e8483032efb860db1329d78d91d457",
//   "component": {
//     "component_id": "vc6-system-import-ddraw",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK DDRAW.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/DDRAW.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectDrawCreate@12", "occurrence": 9, "origin": "0x005D44FC", "spans": ["0x005D44FC-0x005D4502"], "symbol": "DirectDrawCreate", "unit": "DDRAW.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectDrawEnumerateA@8", "occurrence": 11, "origin": "0x00650202", "spans": ["0x00650202-0x00650208"], "symbol": "DirectDrawEnumerateA", "unit": "DDRAW.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
