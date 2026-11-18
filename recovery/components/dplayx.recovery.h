// RECOVERY_IMPORT: v1 __imp__DirectPlayLobbyCreateA@20 DPLAYX.dll ordinal:4
// RECOVERY_IMPORT: v1 __imp__DirectPlayEnumerateA@8 DPLAYX.dll ordinal:2
// RECOVERY_IMPORT: v1 __imp__DirectPlayCreate@12 DPLAYX.dll ordinal:1
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "99ff5ab78c5189fb7090f8c2da08d0da1095833606ca422a051c83f988406739",
//   "component": {
//     "component_id": "dplayx-import-thunks",
//     "delivery": "system_import",
//     "evidence": [
//       "Original thunk boundaries and IAT identities independently checked against the pinned PE.",
//       "Physical external DPLAYX.LIB short-import records identify dependency thunks, not application C++ functions."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record correspondence; operating-system DLL version/ABI/runtime unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/DPLAYX.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__DirectPlayLobbyCreateA@20", "occurrence": 7, "origin": "0x00650208", "spans": ["0x00650208-0x0065020E"], "symbol": "DirectPlayLobbyCreateA", "unit": "DPLAYX.dll"},
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__DirectPlayEnumerateA@8", "occurrence": 5, "origin": "0x0065020E", "spans": ["0x0065020E-0x00650214"], "symbol": "DirectPlayEnumerateA", "unit": "DPLAYX.dll"},
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__DirectPlayCreate@12", "occurrence": 3, "origin": "0x00650214", "spans": ["0x00650214-0x0065021A"], "symbol": "DirectPlayCreate", "unit": "DPLAYX.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
