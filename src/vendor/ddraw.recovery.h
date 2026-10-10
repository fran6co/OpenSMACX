// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "329c7de4664532620fba0e1734e48069a9e8483032efb860db1329d78d91d457",
//   "component": {
//     "component_id": "ddraw-directdrawcreate-thunk",
//     "delivery": "system_import",
//     "evidence": [
//       "Pinned original loader thunk5D44FC..5D4502 and HIGHLOW operand fixup at5D44FE.",
//       "src/general.cpp RECOVERY_IMPORT independently identifies DDRAW.dll!DirectDrawCreate.",
//       "Physical VC6 DDRAW.LIB member DDRAW.dll occurrence9 qualifies its short code import; root CMake already linksddraw."
//     ],
//     "ownership": "system",
//     "version": "VC6 short-import record attribution; operating-system DLL implementation and ABI remain unverified."
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 archive /home/fran6co/opt/vc6/LIB/DDRAW.LIB; redistribution rights are not established.",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned six-byte thunk; independent DDRAW DirectDrawCreate loader slot; physical VC6 short-import record.", "import_symbol": "__imp__DirectDrawCreate@12", "occurrence": 9, "origin": "0x005D44FC", "spans": ["0x005D44FC-0x005D4502"], "symbol": "DirectDrawCreate", "unit": "DDRAW.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
