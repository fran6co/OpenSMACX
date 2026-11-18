// RECOVERY_IMPORT: v1 __imp__ICSendMessage@16 MSVFW32.dll name:ICSendMessage
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "0c2c6ebfeded71da862f8b8d772590ae0dc55d826b9871b0a1052ea333ff2c24",
//   "component": {
//     "component_id": "vfw32-import-thunks",
//     "delivery": "system_import",
//     "evidence": [
//       "Original thunk boundaries and IAT identities independently checked against the pinned PE.",
//       "Physical external VFW32.LIB short-import records identify dependency thunks, not application C++ functions."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record correspondence; operating-system DLL version/ABI/runtime unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/VFW32.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte FF25 thunk and unique HIGHLOW operand fixup; independent original loader export slot; physical pinned VC6 short code import record. DLL ABI/runtime and linked selection remain unproved.", "import_symbol": "__imp__ICSendMessage@16", "occurrence": 9, "origin": "0x00644DC6", "spans": ["0x00644DC6-0x00644DCC"], "symbol": "ICSendMessage", "unit": "MSVFW32.dll"},
//     {"evidence": "Pinned FF25 thunk644DC0..644DC6 with HIGHLOW operand and original ICClose IAT669260; physical pinned SDK record and real video.cpp calls. DLL runtime and current link selection remain unknown.", "import_symbol": "__imp__ICClose@4", "occurrence": 27, "origin": "0x00644DC0", "spans": ["0x00644DC0-0x00644DC6"], "symbol": "ICClose", "unit": "MSVFW32.dll"},
//     {"evidence": "Pinned FF25 thunk644DCC..644DD2 with HIGHLOW operand and original ICOpen IAT669258; physical pinned SDK record and real video.cpp call. DLL runtime and current link selection remain unknown.", "import_symbol": "__imp__ICOpen@12", "occurrence": 12, "origin": "0x00644DCC", "spans": ["0x00644DCC-0x00644DD2"], "symbol": "ICOpen", "unit": "MSVFW32.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
