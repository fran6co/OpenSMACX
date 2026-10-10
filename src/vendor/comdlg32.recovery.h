// RECOVERY_IMPORT: v1 __imp__GetSaveFileNameA@4 comdlg32.dll name:GetSaveFileNameA
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "1d82261a358979f818e7c799be43d57b48a001dfd3da457f2e04d7c9f680ec00",
//   "component": {
//     "component_id": "comdlg32-import-thunks",
//     "delivery": "system_import",
//     "evidence": [
//       "Original thunk boundaries and IAT identities independently checked against the pinned PE.",
//       "Physical external COMDLG32.LIB short-import records identify dependency thunks, not application C++ functions."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record correspondence; operating-system DLL version/ABI/runtime unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/COMDLG32.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__GetSaveFileNameA@4", "occurrence": 15, "origin": "0x006501F6", "spans": ["0x006501F6-0x006501FC"], "symbol": "GetSaveFileNameA", "unit": "comdlg32.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
