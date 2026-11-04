// SDK GUID contributions used by the recovered application.
// Complete sections are verification dependencies; this adds no SDK C++ substitutes.
// RECOVERY_COMPONENT_BEGIN v1
// {
//   "schema_version": 1,
//   "kind": "toolchain_component",
//   "component": {
//     "component_id": "vc6-dxguid",
//     "ownership": "third_party",
//     "delivery": "toolchain",
//     "evidence": [
//       "Root CMake already links dxguid. Pinned SDK archive provides named DirectDraw and DirectPlay interface identifiers as separate complete initialized .rdata contributions.",
//       "The pinned image contains these complete sections; typed QueryInterface call sites in Buffer/movie_media/Net use these independently named SDK identifiers. These are dependency data, never application-function recoveries."
//     ],
//     "version": "Exact local VC6 DXGUID.LIB archive pin; SDK release attribution beyond this pin is unverified."
//   },
//   "archive_sha256": "1f41db9101614d4ac19a681d399c074e6166b566315ac62c4d913556582aef85",
//   "license_evidence": "Local proprietary VC6 SDK archive /home/fran6co/opt/vc6/LIB/DXGUID.LIB; redistribution rights are not established. Only source-owned metadata is tracked.",
//   "bindings": [],
//   "absolute_symbols": [],
//   "data_sections": [
//     {
//       "unit": "guid155.obj",
//       "section": 1,
//       "section_name": ".rdata",
//       "origin": {
//         "image_sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//         "address_space": "pe:va",
//         "address": 6756288
//       },
//       "span": {
//         "start": 6756288,
//         "end": 6756304
//       },
//       "symbols": [
//         {
//           "name": "_IID_IDirectPlay2A",
//           "offset": 0,
//           "storage": 2
//         }
//       ],
//       "evidence": "Complete relocation-free SDK GUID section; archive symbol identity and fixed IID value independently identify the entire 16-byte object. Existing typed DirectDraw/DirectPlay QueryInterface uses identify the interface; fresh physical archive/image checks are required, with no current-link selection claim."
//     },
//     {
//       "unit": "guid5.obj",
//       "section": 1,
//       "section_name": ".rdata",
//       "origin": {
//         "image_sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//         "address_space": "pe:va",
//         "address": 6749224
//       },
//       "span": {
//         "start": 6749224,
//         "end": 6749240
//       },
//       "symbols": [
//         {
//           "name": "_IID_IDirectDrawSurface2",
//           "offset": 0,
//           "storage": 2
//         }
//       ],
//       "evidence": "Complete relocation-free SDK GUID section; archive symbol identity and fixed IID value independently identify the entire 16-byte object. Existing typed DirectDraw/DirectPlay QueryInterface uses identify the interface; fresh physical archive/image checks are required, with no current-link selection claim."
//     },
//     {
//       "unit": "guid3.obj",
//       "section": 1,
//       "section_name": ".rdata",
//       "origin": {
//         "image_sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//         "address_space": "pe:va",
//         "address": 6749240
//       },
//       "span": {
//         "start": 6749240,
//         "end": 6749256
//       },
//       "symbols": [
//         {
//           "name": "_IID_IDirectDraw2",
//           "offset": 0,
//           "storage": 2
//         }
//       ],
//       "evidence": "Complete relocation-free SDK GUID section; archive symbol identity and fixed IID value independently identify the entire 16-byte object. Existing typed DirectDraw/DirectPlay QueryInterface uses identify the interface; fresh physical archive/image checks are required, with no current-link selection claim."
//     }
//   ],
//   "image": {
//     "sha256": "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf",
//     "address_space": "pe:va"
//   }
// }
// RECOVERY_COMPONENT_END
