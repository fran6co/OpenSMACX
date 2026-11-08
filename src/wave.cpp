/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// ORIGINAL: 0x00477E40 _$E51 0x00477E40-0x00477E56 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682554 selects init 0x00477E40, which registers exit callback 0x00477E60.", "Original init span 0x00477E40-0x00477E56 and exit span 0x00477E60-0x00477E6A are independently disassembled; both identify receiver 0x007FFF00 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E51 via .CRT$XCU, relocating to g_MULTIWIN_WAVE, Wave construction, paired local fini _$E49 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_MULTIWIN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E16
// ORIGINAL: 0x00477E60 _$E49 0x00477E60-0x00477E6A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682554 selects init 0x00477E40, which registers exit callback 0x00477E60.", "Original init span 0x00477E40-0x00477E56 and exit span 0x00477E60-0x00477E6A are independently disassembled; both identify receiver 0x007FFF00 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E51 via .CRT$XCU, relocating to g_MULTIWIN_WAVE, Wave construction, paired local fini _$E49 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MULTIWIN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E14
// ORIGINAL: 0x00584D00 _$E56 0x00584D00-0x00584D16 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E21
// ORIGINAL: 0x00584D20 _$E54 0x00584D20-0x00584D2A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682644 selects init 0x00584D00, which registers exit callback 0x00584D20.", "Original init span 0x00584D00-0x00584D16 and exit span 0x00584D20-0x00584D2A are independently disassembled; both identify receiver 0x00945780 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E56 via .CRT$XCU, relocating to g_TOP_MENU_WAVE, Wave construction, paired local fini _$E54 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_TOP_MENU_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E19
// ORIGINAL: 0x005AE0F0 _$E61 0x005AE0F0-0x005AE106 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682658 selects init 0x005AE0F0, which registers exit callback 0x005AE110.", "Original init span 0x005AE0F0-0x005AE106 and exit span 0x005AE110-0x005AE11A are independently disassembled; both identify receiver 0x00945E08 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E61 via .CRT$XCU, relocating to g_CRASH_LANDING_WAVE, Wave construction, paired local fini _$E59 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_CRASH_LANDING_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E26
// ORIGINAL: 0x005AE110 _$E59 0x005AE110-0x005AE11A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682658 selects init 0x005AE0F0, which registers exit callback 0x005AE110.", "Original init span 0x005AE0F0-0x005AE106 and exit span 0x005AE110-0x005AE11A are independently disassembled; both identify receiver 0x00945E08 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E61 via .CRT$XCU, relocating to g_CRASH_LANDING_WAVE, Wave construction, paired local fini _$E59 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_CRASH_LANDING_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E24
#include "stdafx.h"
#include <mmsystem.h>
#include "general.h"
#include "wave.h"
#include "voice.h"
#include "fx.h"
#include "dialog.h"
#include "sounddevice.h"
#include "net_class.h"
#include "basepop.h"
#include "ambience.h"
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memset)
#endif
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memset)
#endif








// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.

// RECOVERY_OBJECT: v1 0x007FFF00 ?g_MULTIWIN_WAVE@@3VWave@@A
Wave g_MULTIWIN_WAVE;  // 0x007FFF00
// RECOVERY_OBJECT: v1 0x00945780 ?g_TOP_MENU_WAVE@@3VWave@@A
Wave g_TOP_MENU_WAVE;  // 0x00945780
// RECOVERY_OBJECT: v1 0x00945E08 ?g_CRASH_LANDING_WAVE@@3VWave@@A
Wave g_CRASH_LANDING_WAVE;  // 0x00945E08
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
