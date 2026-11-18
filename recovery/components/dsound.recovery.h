// RECOVERY_IMPORT: v1 __imp__DirectSoundCreate@12 DSOUND.dll ordinal:1
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "37679dff670270005be960defafa10fab2ba4eda6706ffed7cbb5818007a6105",
//   "component": {
//     "component_id": "dsound-import-thunks",
//     "delivery": "system_import",
//     "evidence": [
//       "Original thunk boundaries and IAT identities independently checked against the pinned PE.",
//       "Physical external DSOUND.LIB short-import records identify dependency thunks, not application C++ functions."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record correspondence; operating-system DLL version/ABI/runtime unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/DSOUND.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__DirectSoundCreate@12", "occurrence": 6, "origin": "0x006501F0", "spans": ["0x006501F0-0x006501F6"], "symbol": "DirectSoundCreate", "unit": "DSOUND.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
