// ORIGINAL: 0x006501F0 DirectSoundCreate 0x006501F0-0x006501F6 EXCLUDED S2a
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x006501F0: size      6 bytes
// seed-history 0x006501F0: prototype
// seed-history 0x006501F0: callers   1   call targets   0
// seed-history 0x006501F0: kind      thunk
// seed-history 0x006501F0: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x006501F0: calls     (none)
// seed-history 0x006501F0: excluded - expressibility - C++ EH unwind funclet; borrows the enclosing function's frame (docs/EXCLUSIONS.md section 2a)
// RECOVERY_IMPORT: v1 __imp__DirectSoundCreate@12 DSOUND.dll ordinal:1

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "37679dff670270005be960defafa10fab2ba4eda6706ffed7cbb5818007a6105",
//   "component": {
//     "component_id": "vc6-system-import-dsound",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK DSOUND.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/DSOUND.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__DirectSoundCreate@12", "occurrence": 6, "origin": "0x006501F0", "spans": ["0x006501F0-0x006501F6"], "symbol": "DirectSoundCreate", "unit": "DSOUND.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
