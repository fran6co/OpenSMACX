// ORIGINAL: 0x00644DC0 ICClose 0x00644DC0-0x00644DC6 FILE
// TRIED: original is `jmp dword ptr [addr]` (6B), a linker-generated import-thunk shape; every spelling tried (extern fn-ptr var, static const fn-ptr, raw deref-of-address) compiles under this build's /O1 to push+call+ret (13B), never a bare tail jmp
// System import ownership companion; no executable implementation is asserted.
// These are SDK loader thunks, not application bodies or EH cleanup funclets.
// Misclassifications in preserved seed-history below are historical only.
// ORIGINAL: 0x00644DC6 ICSendMessage 0x00644DC6-0x00644DCC FILE
// TRIED: direct call through the IAT-style global cast to a function- pointer type; VC6 never folds it to the bare tail jmp, it always emits a push/call sequence instead (tried a plain forwarding wrapper too, same divergence)
// ORIGINAL: 0x00644DCC ICOpen 0x00644DCC-0x00644DD2 FILE
// TRIED: indirect call through a typed function-pointer global (plain deref, extern-declared-slot, __declspec(dllimport)) all lower to push/push/push/call/ret, not the original's bare indirect jmp - VC6 does not tail-call an INDIRECT stdcall callee here

// Historical generated-seed metadata; these facts do not claim current matching.
// seed-history 0x00644DC0: size      6 bytes
// seed-history 0x00644DC0: prototype LRESULT (__stdcall ICClose)(HIC hic)
// seed-history 0x00644DC0: callers   3   call targets   0
// seed-history 0x00644DC0: kind      thunk
// seed-history 0x00644DC0: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00644DC0: calls     (none)
// seed-history 0x00644DC6: size      6 bytes
// seed-history 0x00644DC6: prototype LRESULT (__stdcall ICSendMessage)(HIC hic, UINT msg, DWORD dw1, DWORD dw2)
// seed-history 0x00644DC6: callers   4   call targets   0
// seed-history 0x00644DC6: kind      thunk
// seed-history 0x00644DC6: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00644DC6: calls     (none)
// seed-history 0x00644DCC: size      6 bytes
// seed-history 0x00644DCC: prototype HIC (__stdcall ICOpen)(DWORD fccType, DWORD fccHandler, UINT wMode)
// seed-history 0x00644DCC: callers   1   call targets   0
// seed-history 0x00644DCC: kind      thunk
// seed-history 0x00644DCC: flags     hidden;thunk;sp_ready;purged_ok
// seed-history 0x00644DCC: calls     (none)
// RECOVERY_IMPORT: v1 __imp__ICClose@4 MSVFW32.dll name:ICClose

// RECOVERY_IMPORT: v1 __imp__ICSendMessage@16 MSVFW32.dll name:ICSendMessage

// RECOVERY_IMPORT: v1 __imp__ICOpen@12 MSVFW32.dll name:ICOpen

// SDK archive occurrence selectors are physical indices, not member-name guesses.
// Original-image bindings for this dependency.
// These are verification obligations, not claims of current matching.
// Dependency inputs are unmodified; this companion owns its recovery metadata.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "0c2c6ebfeded71da862f8b8d772590ae0dc55d826b9871b0a1052ea333ff2c24",
//   "component": {
//     "component_id": "vc6-system-import-vfw32",
//     "delivery": "system_import",
//     "evidence": [
//       "pinned original PE import thunk",
//       "pinned VC6 SDK VFW32.LIB"
//     ],
//     "ownership": "system",
//     "version": "VC6 SDK short-import record attribution only"
//   },
//   "kind": "system_import_component",
//   "license_evidence": "Local proprietary VC6 SDK archive at /home/fran6co/opt/vc6/LIB/VFW32.LIB; not redistributed by this proposal; redistribution rights are unestablished",
//   "schema_version": 1,
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   },
//   "bindings": [
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__ICClose@4", "occurrence": 27, "origin": "0x00644DC0", "spans": ["0x00644DC0-0x00644DC6"], "symbol": "ICClose", "unit": "MSVFW32.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__ICSendMessage@16", "occurrence": 9, "origin": "0x00644DC6", "spans": ["0x00644DC6-0x00644DCC"], "symbol": "ICSendMessage", "unit": "MSVFW32.dll"},
//     {"evidence": "Pinned original six-byte loader thunk and exact short-import physical occurrence; no current-selection claim", "import_symbol": "__imp__ICOpen@12", "occurrence": 12, "origin": "0x00644DCC", "spans": ["0x00644DCC-0x00644DD2"], "symbol": "ICOpen", "unit": "MSVFW32.dll"}
//   ]
// }
// RECOVERY_COMPONENT_END
