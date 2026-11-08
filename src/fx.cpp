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

// The effects and ambience TU (0x00445450-0x004483C5): the menu wave
// resources, the FX bank and its Effect waves, the popup wave callbacks, the
// music/ambience state machines, and the Sound-family virtuals emitted after
// them. The boundary is inferred from the image; the original file name is
// unknown.
#include "stdafx.h"
#include "wave.h"
#include "fx.h"
#include "autosound_defaults.h"
#include "autosound.h"
#include "win.h"
#include "basepop.h"
#include "popup.h"
#include "ambience.h"
#include "game.h"
#include "general.h"
#include "veh.h"
#include "faction.h"
#include <mmsystem.h>
#include "sound.h"
#include "strings.h"
#include "map.h"
#include "mapwin.h"



// Sound::get_type and the other small Sound-family virtuals at the end of this
// TU (0x004482F0..0x00448384) are emitted here in the image, after the
// ambience code; Sound::set_type stays with its constructors in sound.cpp.


// Separate writable path arguments identified by original relocated operands.
// Names describe their roles; original names and complete allocation bounds are unknown.
// RECOVERY_OBJECT: v1 0x00683CA4 _EffectFxPathPrefix
static char EffectFxPathPrefix[] = "fx\\";
// RECOVERY_OBJECT: v1 0x00683CA8 _EffectFxPathWaveSuffix
static char EffectFxPathWaveSuffix[] = ".wav";
// RECOVERY_OBJECT: v1 0x00683CB0 _EffectBinaryReadMode
static char EffectBinaryReadMode[] = "rb";
// RECOVERY_OBJECT: v1 0x00683CB4 _EffectFallbackWaveSuffix
static char EffectFallbackWaveSuffix[] = ".wav";

// FX publishes these existing owners; they are defined in their domain TUs.
extern uint32_t BasePopDefaultBlock2E64[70];
extern Wave g_OK_WAVE, g_ALPHAMENU_WAVE, g_SCOOT_WAVE, g_PASSOVER_WAVE;
extern Wave g_MENU_UP_WAVE, g_MENU_DOWN_WAVE;

// Original FX initialization copies 70 words from 0x00683AD8. The shared
// text-block type supplies the extent and callback-visible field meanings.
// RECOVERY_OBJECT: v2 0x00683AD8 0x00683BF0 _FxPopupSoundDefaults 0x00445CD0
const BasePopTextBlock FxPopupSoundDefaults = {"", -1, 127, 50, 0, 0};



// The signed pitch cycle is loaded and stored as a dword by ok_callback.
// This start binding does not infer the extent of neighboring audio storage.
// RECOVERY_OBJECT: v1 0x0074DAA8 ?EffectOkPitch@@3HA
int EffectOkPitch;

// One pointer; the next mapped object, EffectOkPitch, begins at 0x0074DAA8.
// RECOVERY_OBJECT: v2 0x0074DAA0 0x0074DAA4 ?FactionMusic@@3PAVFactionAmbience@@A 0x00491380
FactionAmbience *FactionMusic = nullptr;
// RECOVERY_OBJECT: v1 0x00683BF4 ?LastAmbienceVehicle@@3HA
int LastAmbienceVehicle = -1;

// These exact cells are source-owned here; the transition marker's wider
// external lifetime census remains open for 0x0074C5E4.
// RECOVERY_OBJECT: v2 0x00683BF0 0x00683BF4 ?AmbienceCurrentEffect@@3HA 0x00446B54
int AmbienceCurrentEffect = -1;
// RECOVERY_OBJECT: v2 0x0074C5E4 0x0074C5E8 ?AmbienceTransitionMarker@@3HA 0x00446C59
int AmbienceTransitionMarker = 0;

// Original447DD0 individually reads/writes each four-byte sequencer cell.
// RECOVERY_OBJECT: v1 0x00683C30 _AAmbiencePass
static int AAmbiencePass = 6;             //683C30
// RECOVERY_OBJECT: v1 0x00683C34 _AAmbienceRandomRange
static int AAmbienceRandomRange = 1;      //683C34
// RECOVERY_OBJECT: v1 0x00683C38 _AAmbiencePhase
static int AAmbiencePhase = 3;            //683C38
// RECOVERY_OBJECT: v1 0x00683C3C _AAmbienceCents
static int AAmbienceCents = -600;         //683C3C
// RECOVERY_OBJECT: v1 0x00683C40 _AAmbienceCentsStep
static int AAmbienceCentsStep = 200;      //683C40
// RECOVERY_OBJECT: v1 0x00683C44 _AAmbienceLevel
static int AAmbienceLevel = 90;           //683C44
// RECOVERY_OBJECT: v1 0x00683C48 _AAmbienceTrack0
static int AAmbienceTrack0 = 18;          //683C48
// RECOVERY_OBJECT: v1 0x00683C4C _AAmbienceTrack1
static int AAmbienceTrack1 = 21;          //683C4C
// RECOVERY_OBJECT: v1 0x00683C50 _AAmbienceTrack2
static int AAmbienceTrack2 = 24;          //683C50
// RECOVERY_OBJECT: v1 0x0074DAE8 _AAmbiencePhase3Cents
static int AAmbiencePhase3Cents = 0;      //74DAE8
// RECOVERY_OBJECT: v1 0x0074DAF0 _AAmbienceAlternate
static int AAmbienceAlternate = 0;        //74DAF0

// RECOVERY_OBJECT: v1 0x0074DAD4 _BAmbienceHostility
static uint8_t BAmbienceHostility = 0;    //74DAD4
// RECOVERY_OBJECT: v1 0x00683C18 _BAmbienceAmbientPass
static int BAmbienceAmbientPass = 3;      //683C18
// RECOVERY_OBJECT: v1 0x0074DAD8 _BAmbienceMusicPass
static int BAmbienceMusicPass = 0;        //74DAD8
// RECOVERY_OBJECT: v1 0x0074DADC _BAmbienceTrackPass
static int BAmbienceTrackPass = 0;        //74DADC
// RECOVERY_OBJECT: v1 0x00683C1C _BAmbienceAlternate
static uint8_t BAmbienceAlternate = 1;    //683C1C

UAmbience::UAmbience() {}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// THE AMBIENCE SEQUENCER STATE. Every one of these is an ordinary int the
// image keeps in .data at a fixed address, and the four variants below read
// and write them as one shared bank. They are named from what the bodies do
// with them - the increment, the wrap point, the virtual slot the value
// feeds - because no catalogue row covers any of them. The binding spelling
// is the one the codegen needs: a dereferenced const pointer folds to the
// absolute address, where an `extern T` definition would not.
//
//   AmbienceClockArmed   0x0074C5E8  byte, set once, gates the first clock read
//   AmbienceClock        0x0074D8EC  the last clock reading
//   AmbienceGeneralTick  0x00683C14  clamped to 18, incremented once a pass
//   MAmbienceStep        0x00683BF8  play_trackset argument, cycles 5..7
//   MAmbienceStepBase    0x00683BFC  the closing xpose_trackset argument
//   MAmbiencePass        0x0074DAAC  the five-second pass counter
//   MAmbienceCents       0x0074DAB0  +150 a pass, wraps +600 back to -600
//   UAmbienceStep        0x00683C00  play_trackset argument, wraps +600 to -600
//   UAmbienceStepBase    0x00683C04  play_trackset argument, fixed
//   UAmbienceStepEvery   0x00683C08  the pass count UAmbience waits for
//   UAmbienceTrackset    0x00683C0C  toggles between 4 and 5
//   UAmbiencePass0       0x0074DAB4  wraps at 9
//   UAmbiencePass1       0x0074DAB8  wraps at 6
//   UAmbienceCents       0x0074DABC  +100 a pass
//   UAmbiencePass2       0x0074DAC0  compared against UAmbienceStepEvery
//   UAmbiencePass3       0x0074DAC4  wraps at 15
//   SAmbienceCents       0x0074DAC8  +100 a pass, wraps +400 back to -300
//   SAmbiencePass0       0x0074DACC  wraps at 1
//   SAmbiencePass1       0x0074DAD0  wraps at 7
// RECOVERY_OBJECT: v1 0x0074C5E8 _AmbienceClockArmed
static uint8_t AmbienceClockArmed = 0; // Original 0x0074C5E8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074D8EC _AmbienceClock
static int AmbienceClock = 0; // Original 0x0074D8EC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C14 _AmbienceGeneralTick
static int AmbienceGeneralTick = 17; // Original 0x00683C14, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683BF8 _MAmbienceStep
static int MAmbienceStep = 5; // Original 0x00683BF8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683BFC _MAmbienceStepBase
static int MAmbienceStepBase = 1; // Original 0x00683BFC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAAC _MAmbiencePass
static int MAmbiencePass = 0; // Original 0x0074DAAC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB0 _MAmbienceCents
static int MAmbienceCents = 0; // Original 0x0074DAB0, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C00 _UAmbienceStep
static int UAmbienceStep = 2; // Original 0x00683C00, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C04 _UAmbienceStepBase
static int UAmbienceStepBase = 3; // Original 0x00683C04, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C08 _UAmbienceStepEvery
static int UAmbienceStepEvery = 10; // Original 0x00683C08, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x00683C0C _UAmbienceTrackset
static int UAmbienceTrackset = 5; // Original 0x00683C0C, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB4 _UAmbiencePass0
static int UAmbiencePass0 = 0; // Original 0x0074DAB4, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAB8 _UAmbiencePass1
static int UAmbiencePass1 = 0; // Original 0x0074DAB8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DABC _UAmbienceCents
static int UAmbienceCents = 0; // Original 0x0074DABC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC0 _UAmbiencePass2
static int UAmbiencePass2 = 0; // Original 0x0074DAC0, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC4 _UAmbiencePass3
static int UAmbiencePass3 = 0; // Original 0x0074DAC4, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAC8 _SAmbienceCents
static int SAmbienceCents = 0; // Original 0x0074DAC8, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DACC _SAmbiencePass0
static int SAmbiencePass0 = 0; // Original 0x0074DACC, independently read initial bytes.
// RECOVERY_OBJECT: v1 0x0074DAD0 _SAmbiencePass1
static int SAmbiencePass1 = 0; // Original 0x0074DAD0, independently read initial bytes.

// Fixed-slot bindings carried from 00447c00.cpp
// AmbienceClockArmed's sibling at 0x00683C20: UAmbience's own one-shot
// gate, compared against 0x18 and never written here.
// RECOVERY_OBJECT: v1 0x00683C20 _UAmbienceGate
static int UAmbienceGate = 24; // Original 0x00683C20, independently read initial bytes.

// Fixed-slot bindings carried from 00447c70.cpp
// GAmbience's play_trackset argument at 0x00683C24: cycled 7..9.
// RECOVERY_OBJECT: v1 0x00683C24 _GAmbienceStep
static int GAmbienceStep = 7; // Original 0x00683C24, independently read initial bytes.

// Fixed-slot bindings carried from 00447cd0.cpp
// SAmbience's one-shot flag at 0x00683C28, set to 1 on entry.
// RECOVERY_OBJECT: v1 0x00683C28 _SAmbienceArmed
static int SAmbienceArmed = 1; // Original 0x00683C28, independently read initial bytes.


// Inferred Wave-resource storage/lifetime contribution; original TU identity remains unproven.

// ORIGINAL: 0x00445450 _$E26 0x00445450-0x00445474 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068208C selects init 0x00445450, which registers exit callback 0x00445480.", "Original init span 0x00445450-0x00445474 and exit span 0x00445480-0x00445494 are independently disassembled; both identify receiver 0x0074C5F0 and Wave constructor/destructor 0x004C66E0/0x004C67C0. CPU bank vector helpers prove count45 and stride0x6C.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E26 via .CRT$XCU, relocating to g_CPU_WAVES, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_CPU_WAVES. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E16
// ORIGINAL: 0x00445480 _$E24 0x00445480-0x00445494 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068208C selects init 0x00445450, which registers exit callback 0x00445480.", "Original init span 0x00445450-0x00445474 and exit span 0x00445480-0x00445494 are independently disassembled; both identify receiver 0x0074C5F0 and Wave constructor/destructor 0x004C66E0/0x004C67C0. CPU bank vector helpers prove count45 and stride0x6C.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E26 via .CRT$XCU, relocating to g_CPU_WAVES, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_CPU_WAVES. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E14
// ORIGINAL: 0x004454A0 _$E21 0x004454A0-0x004454B6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682090 selects init 0x004454A0, which registers exit callback 0x004454C0.", "Original init span 0x004454A0-0x004454B6 and exit span 0x004454C0-0x004454CA are independently disassembled; both identify receiver 0x00749C88 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical pre-regrouping menu.cpp physical COFF selected local STATIC init _$E21 via .CRT$XCU, relocating to g_MENU_UP_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_MENU_UP_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E21
// ORIGINAL: 0x004454C0 _$E19 0x004454C0-0x004454CA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682090 selects init 0x004454A0, which registers exit callback 0x004454C0.", "Original init span 0x004454A0-0x004454B6 and exit span 0x004454C0-0x004454CA are independently disassembled; both identify receiver 0x00749C88 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical pre-regrouping menu.cpp physical COFF selected local STATIC init _$E21 via .CRT$XCU, relocating to g_MENU_UP_WAVE, Wave construction, paired local fini _$E19 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MENU_UP_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E19
// ORIGINAL: 0x004454D0 _$E26 0x004454D0-0x004454E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682094 selects init 0x004454D0, which registers exit callback 0x004454F0.", "Original init span 0x004454D0-0x004454E6 and exit span 0x004454F0-0x004454FA are independently disassembled; both identify receiver 0x0074D960 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical pre-regrouping menu.cpp physical COFF selected local STATIC init _$E26 via .CRT$XCU, relocating to g_MENU_DOWN_WAVE, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_MENU_DOWN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E26
// ORIGINAL: 0x004454F0 _$E24 0x004454F0-0x004454FA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682094 selects init 0x004454D0, which registers exit callback 0x004454F0.", "Original init span 0x004454D0-0x004454E6 and exit span 0x004454F0-0x004454FA are independently disassembled; both identify receiver 0x0074D960 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical pre-regrouping menu.cpp physical COFF selected local STATIC init _$E26 via .CRT$XCU, relocating to g_MENU_DOWN_WAVE, Wave construction, paired local fini _$E24 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_MENU_DOWN_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E24
// ORIGINAL: 0x00445500 _$E31 0x00445500-0x00445516 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682098 selects init 0x00445500, which registers exit callback 0x00445520.", "Original init span 0x00445500-0x00445516 and exit span 0x00445520-0x0044552A are independently disassembled; both identify receiver 0x00749C18 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E31 via .CRT$XCU, relocating to g_SCOOT_WAVE, Wave construction, paired local fini _$E29 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_SCOOT_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E31
// ORIGINAL: 0x00445520 _$E29 0x00445520-0x0044552A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682098 selects init 0x00445500, which registers exit callback 0x00445520.", "Original init span 0x00445500-0x00445516 and exit span 0x00445520-0x0044552A are independently disassembled; both identify receiver 0x00749C18 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E31 via .CRT$XCU, relocating to g_SCOOT_WAVE, Wave construction, paired local fini _$E29 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_SCOOT_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E29
// ORIGINAL: 0x00445530 _$E36 0x00445530-0x00445546 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068209C selects init 0x00445530, which registers exit callback 0x00445550.", "Original init span 0x00445530-0x00445546 and exit span 0x00445550-0x0044555A are independently disassembled; both identify receiver 0x0074D8F0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E36 via .CRT$XCU, relocating to g_OK_WAVE, Wave construction, paired local fini _$E34 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_OK_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E36
// ORIGINAL: 0x00445550 _$E34 0x00445550-0x0044555A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x0068209C selects init 0x00445530, which registers exit callback 0x00445550.", "Original init span 0x00445530-0x00445546 and exit span 0x00445550-0x0044555A are independently disassembled; both identify receiver 0x0074D8F0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E36 via .CRT$XCU, relocating to g_OK_WAVE, Wave construction, paired local fini _$E34 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_OK_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E34
// ORIGINAL: 0x00445560 _$E41 0x00445560-0x00445576 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x006820A0 selects init 0x00445560, which registers exit callback 0x00445580.", "Original init span 0x00445560-0x00445576 and exit span 0x00445580-0x0044558A are independently disassembled; both identify receiver 0x0074D9D0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E41 via .CRT$XCU, relocating to g_PASSOVER_WAVE, Wave construction, paired local fini _$E39 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_PASSOVER_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E41
// ORIGINAL: 0x00445580 _$E39 0x00445580-0x0044558A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x006820A0 selects init 0x00445560, which registers exit callback 0x00445580.", "Original init span 0x00445560-0x00445576 and exit span 0x00445580-0x0044558A are independently disassembled; both identify receiver 0x0074D9D0 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E41 via .CRT$XCU, relocating to g_PASSOVER_WAVE, Wave construction, paired local fini _$E39 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance.", "The six source-owned Wave resource declarations and their paired compiler-generated lifetimes are grouped in src/fx_wave_resources.cpp in pinned CRT-slot order 0x0068208C..0x006820A0. Generic Wave methods and the FX manager remain separate; this is inferred source correspondence, not proof of historical object boundaries or linked selection."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_PASSOVER_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E39

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// FX::init supplies these receivers; array teardown independently witnesses
// the CPU bank's 45 elements and 0x6C stride.
// FX::init loads and configures these existing menu sound owners.

// RECOVERY_OBJECT: v2 0x0074C5F0 0x0074D8EC ?g_CPU_WAVES@@3PAVWave@@A 0x00445CD0
Wave g_CPU_WAVES[45];  // 0x0074C5F0, 0x6c stride
// RECOVERY_OBJECT: v1 0x00749C88 ?g_MENU_UP_WAVE@@3VWave@@A
Wave g_MENU_UP_WAVE;  // 0x00749C88
// RECOVERY_OBJECT: v1 0x0074D960 ?g_MENU_DOWN_WAVE@@3VWave@@A
Wave g_MENU_DOWN_WAVE;  // 0x0074D960
// RECOVERY_OBJECT: v1 0x00749C18 ?g_SCOOT_WAVE@@3VWave@@A
Wave g_SCOOT_WAVE;  // 0x00749C18
// RECOVERY_OBJECT: v1 0x0074D8F0 ?g_OK_WAVE@@3VWave@@A
Wave g_OK_WAVE;  // 0x0074D8F0
// RECOVERY_OBJECT: v1 0x0074D9D0 ?g_PASSOVER_WAVE@@3VWave@@A
Wave g_PASSOVER_WAVE;  // 0x0074D9D0

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// RECOVERY_OBJECT: v1 0x00749CF8 ?g_FX@@3VFX@@A
FX g_FX;

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// RECOVERY_OBJECT: v1 0x0074DA40 ?g_AMBIENCE@@3VAmbience@@A
Ambience g_AMBIENCE;  // 0x0074DA40
/*
Purpose: Tear down an Effect, which is a Wave and nothing more.
// ORIGINAL: 0x004482C0 ??1Effect@@QAE@XZ 0x004482C0-0x004482C5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x448290 array destruction and 0x4482A0 construction-unwind each pass0x4482C0 for97elements of0x6Cbytes; original0x4482C0..0x4482C5 is a direct tail toWave::~Wave at0x4C67C0.", "src/fx.h owns Effect effects_[97]; Effect adds initialization policy but no members or destructor work beyond its Wave base. Implicit destruction preserves the existing virtual hierarchy.", "Native compiler controls distinguish the empty user-defined destructor (derived vptr store plus base tail) from implicit destruction emitted inFX (base tail only). Original compiler command and complete linked/runtime preservation remain separate obligations."], "reason": "The real FX array owns the compiler-generated Effect complete destructor; retain its independent original identity without a handwritten forwarding body."}
// symbol    ??1Effect@@UAE@XZ
// CORRECTED from 0x004482C0-0x004482C4, which the artifact carried: the body
//   is one `E9 rel32`, five bytes, and a four-byte span cannot decode a single
//   instruction - `measure` read "0 of 0 instructions, 4 discounted as
//   relocations", which is what a span one byte short looks like.
// Implicit Effect destruction is owned by the FX array below. The original
// FX teardown and construction unwind both select this five-byte tail
// to Wave::~Wave. No additional derived destructor work is present.
// The historical experiments below are superseded: removing the empty
// user-defined destructor AND its declaration emits the direct base
// tail in this owner with the consistent virtual Sound/Wave hierarchy.
// A stale header is not required to explain the original lifetime.
// TRIED 2026-08-30, post-virtualization sweep (the claim regressed when
//   ~Sound went virtual): empty user dtor measures `mov [ecx],0; ret` - the
//   base-dtor call is ELIDED outright; qualified `this->Wave::~Wave()` gives
//   23 instructions of SEH frame + vptr store + TWO base calls; a no-user-
//   dtor implicit compiles the dtor straight into the fx.cpp caller and the
//   symbol disappears; `#pragma auto_inline(off)` on ~Wave changes nothing.
//   Evidence for the class pass: the image's Sound vtable slot 2 holds
//   ??_GSound (0x004C92D0, verified in the image), so ~Sound IS virtual and
//   the catalogue's QAE spelling hides UAE - yet the original's ??1Effect
//   still folds to a 5-byte tail-jmp, which our VC6 refuses for a virtual
//   base dtor. Something about the original Effect model (a friend-side
//   emission, a different TU split, or /GX flags) remains unmodelled.
//   MEASURED 2026-08-30, VoiceTx class pass: the regression reproduces on the
//   PRISTINE tree - `git checkout HEAD` of every file this pass touched and
//   this body still measures 0/1 (`mov [ecx], <vtable>` + the jump) - so the
//   red claim was inherited, not caused here. The structural reading the
//   evidence supports: the shipped Effect.obj was compiled against a sound.h
//   that did not yet have `virtual ~Sound` (a stale-TU build), which is why
//   the SAME 12.00.8168 compiler both puts ??_GSound in slot 2 of every
//   Sound-family vtable AND emits this storeless tail-jmp - the two shapes
//   cannot come from one consistent header, and no source this tree can hold
//   reproduces both. The model choice stands with the virtual ~Sound, because
//   slot 2 is measured in the image's tables and ??1Effect is one 5-byte
//   thorn; leave the claim red and the TRIED wall standing.
// size      5 bytes
// prototype void (__thiscall ??1Effect@@QAE@XZ)(Effect* this)
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C67C0
//
// PROMOTED from src/recovered/units/004482c0.cpp. The whole body is the tail
// jump into `??1Wave@@QAE@XZ` that a trivial derived destructor emits, which
// is why `class Effect : public Wave` is the fix and an empty inline stub is
// not - that compiles to nothing at all.
Return Value: n/a
Status: Complete
*/


// ORIGINAL: 0x00448350 ??_GEffect@@UAEPAXI@Z 0x00448350-0x00448371 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00448350-0x00448371", "Pinned class constructor ??0Effect@@QAE@XZ at 0x004482D0 stores original class/view table VA 0x66b7a4 at 0x004482D8 (mov dword ptr [esi], 0x66b7a4); the current genuine owner vftable ??_7Effect@@6B@ selects generated ??_GEffect@@UAEPAXI@Z in slot 2 (+0x8) through weak entry ??_EEffect@@UAEPAXI@Z (mode 2).", "Pinned ??_GEffect@@UAEPAXI@Z at 0x00448350 calls the source-mapped ??1Effect@@QAE@XZ at 0x004482C0, then tests byte ptr [ebp + 8], 1; je 0x44836a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/fx.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Effect scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GEffect@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00445590 _$E21 0x00445590-0x004455A6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a4 selects initializer0x445590..0x4455a6, registering exit0x4455b0..0x4455ba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x749cf8 bound to?g_FX@@3VFX@@A.", "Original startup instruction evidence: [[\"0x445590\", \"mov\", \"ecx, 0x749cf8\"], [\"0x445595\", \"call\", \"0x4482a0\"], [\"0x44559a\", \"push\", \"0x4455b0\"], [\"0x44559f\", \"call\", \"0x645398\"], [\"0x4455a4\", \"pop\", \"ecx\"], [\"0x4455a5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455b0\", \"mov\", \"ecx, 0x749cf8\"], [\"0x4455b5\", \"jmp\", \"0x448290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/fx.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FX@@3VFX@@A\", \"??0FX@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FX@@3VFX@@A\", \"??1FX@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f050; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_FX@@3VFX@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E46
// ORIGINAL: 0x004455B0 _$E19 0x004455B0-0x004455BA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a4 selects initializer0x445590..0x4455a6, registering exit0x4455b0..0x4455ba. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x749cf8 bound to?g_FX@@3VFX@@A.", "Original startup instruction evidence: [[\"0x445590\", \"mov\", \"ecx, 0x749cf8\"], [\"0x445595\", \"call\", \"0x4482a0\"], [\"0x44559a\", \"push\", \"0x4455b0\"], [\"0x44559f\", \"call\", \"0x645398\"], [\"0x4455a4\", \"pop\", \"ecx\"], [\"0x4455a5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455b0\", \"mov\", \"ecx, 0x749cf8\"], [\"0x4455b5\", \"jmp\", \"0x448290\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/fx.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FX@@3VFX@@A\", \"??0FX@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FX@@3VFX@@A\", \"??1FX@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f050; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_FX@@3VFX@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E44
// ORIGINAL: 0x004455C0 _$E21 0x004455C0-0x004455D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a8 selects initializer0x4455c0..0x4455d6, registering exit0x4455e0..0x4455ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74da40 bound to?g_AMBIENCE@@3VAmbience@@A.", "Original startup instruction evidence: [[\"0x4455c0\", \"mov\", \"ecx, 0x74da40\"], [\"0x4455c5\", \"call\", \"0x4c8460\"], [\"0x4455ca\", \"push\", \"0x4455e0\"], [\"0x4455cf\", \"call\", \"0x645398\"], [\"0x4455d4\", \"pop\", \"ecx\"], [\"0x4455d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455e0\", \"mov\", \"ecx, 0x74da40\"], [\"0x4455e5\", \"jmp\", \"0x4483c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/ambience.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_AMBIENCE@@3VAmbience@@A\", \"??0Ambience@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_AMBIENCE@@3VAmbience@@A\", \"??1Ambience@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f05c; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_AMBIENCE@@3VAmbience@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E51
// ORIGINAL: 0x004455E0 _$E19 0x004455E0-0x004455EA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820a8 selects initializer0x4455c0..0x4455d6, registering exit0x4455e0..0x4455ea. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74da40 bound to?g_AMBIENCE@@3VAmbience@@A.", "Original startup instruction evidence: [[\"0x4455c0\", \"mov\", \"ecx, 0x74da40\"], [\"0x4455c5\", \"call\", \"0x4c8460\"], [\"0x4455ca\", \"push\", \"0x4455e0\"], [\"0x4455cf\", \"call\", \"0x645398\"], [\"0x4455d4\", \"pop\", \"ecx\"], [\"0x4455d5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4455e0\", \"mov\", \"ecx, 0x74da40\"], [\"0x4455e5\", \"jmp\", \"0x4483c0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/ambience.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_AMBIENCE@@3VAmbience@@A\", \"??0Ambience@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_AMBIENCE@@3VAmbience@@A\", \"??1Ambience@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f05c; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_AMBIENCE@@3VAmbience@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E49

// ORIGINAL: 0x004455F0 ?wave_it@@YAXH@Z 0x004455F0-0x0044564D FILE BYTE_EXACT
// size      93 bytes
// prototype
// callers   7   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C6920 0x004C6B10 0x004C6CE0
//
// PRESERVED UNIT - measured MNEMONIC_ONLY. Scaffold preamble stripped by hand
// 2026-09-06: the body below is spelled against the real Wave / GamePreferences
// / PopupWaveLastIndex declarations, so the homing carries no fixed-slot
// bindings and no cast.
//
// The old forwarder in pending_bodies.cpp came out by hand: home_bodies.py
// clears a forwarder against the CLASS it names, and a free function has
// none, so the tool never saw this one.
// TRIED: computing `wave` before the store, `g_CPU_WAVES[n]` subscript over
//   `g_CPU_WAVES + n`, and a `const int chosen` copy of the argument - all
//   three compile to the same 29 instructions. The two that disagree with the
//   image are the second `lea` of the index chain landing in ecx where the
//   image keeps it in eax (a register-allocation placement, the same family
//   hide() records above), and the `mov [PopupWaveLastIndex], eax` store,
//   whose absolute operand is a relocation on this side and a fixed address
//   on the image's.

void __cdecl wave_it(int wave_id) {
    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) {
        if (!g_WAVE_GENERAL.Wave::is_playing()) {
            if (!(g_CPU_WAVES + PopupWaveLastIndex)->Wave::is_playing()) {
                PopupWaveLastIndex = wave_id;
                (g_CPU_WAVES + wave_id)->Wave::load();
                (g_CPU_WAVES + wave_id)->Wave::play();
            }
        }
    }
}

// ORIGINAL: 0x00445650 ?ok_callback@@YAXXZ 0x00445650-0x0044569D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bca75cb0:src/recovered/00445650.cpp", "image:original complete function instructions", "source:src/wave.cpp"], "reason": "Home the original behavioral dependency in its base/audio domain using typed state and existing named callees; retained seed provenance and original spans remain restoration evidence, not runtime acceptance."}
void __cdecl ok_callback() {
    g_OK_WAVE.Wave::set_pitch(EffectOkPitch);
    g_OK_WAVE.Wave::play();
    int pitch = EffectOkPitch;
    if (pitch >= 0) {
        pitch -= 300;
    } else {
        pitch += 200;
    }
    EffectOkPitch = pitch;
    if (pitch <= -1200 || pitch >= 1200) {
        EffectOkPitch = 0;
    }
}


// The button hover hook plays the already-owned pass-over effect.
// ORIGINAL: 0x004456A0 ?button_hover_callback@@YAXXZ 0x004456A0-0x004456AA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00445BB0 Effect initializer publishes 0x004456A0 to hook 0x009B7AA0", "image:0x004456A0-0x004456AA complete MOV receiver 0x0074D9D0 and tail JMP 0x004C6920; NOP padding to independently mapped 0x004456B0", "src/flatbutton.cpp original 0x00608500 hover callback consumer; superseded seed retained in git:de91fffe", "src/wave.cpp g_PASSOVER_WAVE and Wave::play"], "reason": "Provisional behavior-derived name for the nullary button-hover sound callback installed by Effect initialization and consumed by FlatButton hover; its complete entry tail-calls play on the already-owned pass-over Wave, rather than embedding an executable address."}
void __cdecl button_hover_callback() {
    g_PASSOVER_WAVE.Wave::play();
}


/*
Purpose: Sound a popup's wave. Nothing sounds unless bit 0x400 of the flag
         word is set, the popup is armed, and its index is inside the
         forty-five entry bank. Wave 0x19 becomes 0x25 when the selected
         base is producing a secret project; wave 0x2B only sounds when the
         millisecond clock modulo three is one. With both the voiceover
         wave and the last-played bank wave idle, the index is remembered
         and its bank entry loads and plays. Afterwards - whether or not
         anything sounded - wave 0x19 fires the owner's virtual at 0x138
         and wave 0x10 plays effect 0x38.
// ORIGINAL: 0x004456B0 ?popup_wave_callback@@YAXPAUPopupWave@@H@Z 0x004456B0-0x004457BE BYTE_EXACT
// symbol    ?popup_wave_callback@@YAXPAUBasePopTextBlock@@H@Z
// TRIED     the two differing instructions are the vtable temp - image `mov eax, [ecx]` against the compiled `mov edx, [ecx]` - and the register is chosen by the SEAM, not by spelling. VC6 reserves `__thiscall` (C4234), so a plain `(*(Slot **)owner)[78](owner)` cannot be written at all, which is why the ORIGINAL()/pointer-to-member machinery exists; that machinery is what takes eax and pushes the vtable load into edx. Byte-exactness needs a REAL class with a no-argument virtual at slot 78. It is not Win: 0x138 is Win's on_key slot (0x005F5D10) and on_key takes four arguments while this call pushes none, so naming Win here would be invention.
// size      270 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446A00 0x004C6920 0x004C6B10 0x004C6CE0
// indirect  0x00445715 0x004457A0
Return Value: n/a
Status: Complete
*/
// The last-played entry of the wave bank (popup.h). Zero-initialised, as
// the image's dword at 0x0074DAA4 is.
int PopupWaveLastIndex;  // 0x0074DAA4

void __cdecl popup_wave_callback(BasePopTextBlock *popup, int) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) {
        return;
    }
    if (!popup) {
        return;
    }
    const int32_t index = popup->field_104_;
    if (index < 0) {
        return;
    }
    if (!popup->field_108_) {
        return;
    }
    if (index >= 0x2D) {
        return;
    }
    if (index == 0x19) {
        if (BaseCurrent->queue_production_id[0] < -FacilitySPStart) {
            popup->field_104_ = 0x25;
        }
    }
    if (popup->field_104_ == 0x2B && timeGetTime() % 3 != 1) {
        return;
    }
    const int32_t chosen = popup->field_104_;
    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) {
        if (!g_WAVE_GENERAL.Wave::is_playing() &&
            !(g_CPU_WAVES + PopupWaveLastIndex)->Wave::is_playing()) {
            PopupWaveLastIndex = chosen;
            (g_CPU_WAVES + chosen)->Wave::load();
            (g_CPU_WAVES + chosen)->Wave::play();
        }
    }
    if (popup->field_104_ == 0x19) {
        if (FactionMusic) FactionMusic->production();
    }
    if (popup->field_104_ == 0x10) {
        g_FX.FX::play(0x38);
    }
}

// The window procedure's callback delegates to the active faction ambience.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004457C0 ?input_callback@@YAXXZ 0x004457C0-0x004457D3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/004457c0.cpp original identity and complete 19-byte span", "image:0x004457C0-0x004457D3 null-test and virtual tail jump at offset 0x13C", "src/ambience.cpp FactionMusic at 0x0074DAA0; src/ambience.h FactionAmbience event slots75-89"], "reason": "Original nullary window-message callback consults the source-owned faction music pointer and tail-dispatches its genuine general event at virtual slot79; the compatible exact seed remains historical evidence, not current source acceptance."}
void __cdecl input_callback() {
    if (FactionMusic != 0) FactionMusic->general();
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004457E0 ?load_music@@YAXH@Z 0x004457E0-0x00445BA4;0x00654CF0-0x00654D52 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004457e0.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void __cdecl load_music(int faction_id) {
    StringTemp[0] = 0;
    strcat(StringTemp, "fx\\");
    if (FactionMusic) {
        delete FactionMusic;
        FactionMusic = nullptr;
    }
    const char *const name = Players[faction_id].search_key;
    FactionAmbience *music;
    if (stricmp(name, "gaians") == 0) {
        strcat(StringTemp, "gset1.amb");
        music = new GAmbience;
    } else if (stricmp(name, "hive") == 0) {
        strcat(StringTemp, "mset1.amb");
        music = new MAmbience;
    } else if (stricmp(name, "univ") == 0) {
        strcat(StringTemp, "uset2.amb");
        music = new UAmbience;
    } else if (stricmp(name, "morgan") == 0) {
        strcat(StringTemp, "mset1.amb");
        music = new MAmbience;
    } else if (stricmp(name, "spartans") == 0) {
        strcat(StringTemp, "sset1.amb");
        music = new SAmbience;
    } else if (stricmp(name, "believers") == 0) {
        strcat(StringTemp, "bset1.amb");
        music = new BAmbience;
    } else if (stricmp(name, "peace") == 0) {
        strcat(StringTemp, "gset1.amb");
        music = new GAmbience;
    } else {
        strcat(StringTemp, "aset1.amb");
        music = new AAmbience;
    }
    const uint32_t preferences = GameDataStateStorage.fields_.game_preferences_;
    FactionMusic = music;
    if ((preferences & 0x800) && music) {
        music->load(StringTemp);
        FactionMusic->begin();
    }
}

/*
Purpose: Initialise an Effect wave. Registers the Effect family's three
         callbacks into the Win hook slots, resolves the filename against the
         fx\\ directory (appending .wav when the name carries no extension),
         and hands the resolved path and the adjusted mode to Wave::init. A
         mode with neither bit 0 nor bit 7 gains bit 4 - the streamed-and-
         looped combination - and a mode with neither bit 2 nor bit 7 runs the
         wave's own vtable slot 0x8C once Wave::init returns.
// ORIGINAL: 0x00445BB0 ?init@Effect@@QAEXPADJ@Z 0x00445BB0-0x00445CC6 FILE BYTE_EXACT
// size      278 bytes
// prototype void (__thiscall ?init@Effect@@QAEXPADJ@Z)(Effect* this, int8*, long)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C69B0 0x00645470 0x00645598 0x00645646 0x00645DD0
// indirect  0x00445CB9
// LEVER: the three callback stores must be `#define`d constants, not const
//   pointer variables - caviar.cpp's lever, re-measured here. A header-defined
//   `const func_effect_callback X = (func_effect_callback)0xADDR;` compiles
//   `mov eax, dword ptr [X]` + `mov [hook], eax` (87 instructions, 6/84);
//   the macro folds the immediate (`mov dword ptr [hook], 0x4457c0`, 84/84
//   instruction count, 79/84 agreeing).
// LEVER: `if (file) { fclose } else { rebuild }` vs the image's arm order.
//   The image falls THROUGH the fopen test into the rebuild and JUMPS to the
//   fclose, so the guard is spelled `if (!file) { rebuild } else { fclose }`.
// TRIED: the mode block ahead of the three stores (72/84), a separate
//   `long mode` local for the adjusted flags (5/84), and the three stores in
//   reverse order (79/84, but lower similarity). Best is stores, then the
//   mode block, then the path build - the image's own order.
// TRIED: /G6 (Pentium-Pro scheduling) on this body, 4/84 - the remaining 5
//   instructions are the three hook stores interleaved into the push/test
//   sequence where the image keeps them contiguous after `mov edi, ecx`.
//   No source ordering tried moved them; scheduling plateau.
// PROMOTED from src/recovered/units/00445bb0.cpp.
Return Value: n/a
Status: Complete
*/
void Effect::init(char *fname, long flags) {
    WinMessageHook = input_callback;
    WinOkHook = ok_callback;
    WinButtonHoverHook = button_hover_callback;
    if ((static_cast<uint8_t>(flags) & 0x81) == 0) {
        flags |= 4;
    }
    char path[0x100];
    path[0] = '\0';
    // Original path arguments use four distinct writable objects.
    // Equal .wav contents do not identify one shared original object.
    strcat(path, EffectFxPathPrefix);
    strcat(path, fname);
    if (!strchr(path, '.')) {
        strcat(path, EffectFxPathWaveSuffix);
    }
    FILE *const file = fopen(path, EffectBinaryReadMode);
    if (!file) {
        // No such file under fx\\: rebuild the path from the raw argument,
        // extension pass included, and let Wave::init report what it finds.
        path[0] = '\0';
        strcat(path, fname);
        if (!strchr(path, '.')) {
            strcat(path, EffectFallbackWaveSuffix);
        }
    } else {
        fclose(file);
    }
    Wave::init(path, flags);
    if ((static_cast<uint8_t>(flags) & 0x84) == 0) {
        load();
    }
}

// Initialize the effect and CPU-voice banks in original call order. Qualified
// Wave calls were direct in the executable; Effect[4].set_pitch is virtual.
// ORIGINAL: 0x00445CD0 ?init@FX@@QAEXXZ 0x00445CD0-0x004468E1
// RECOVERY_BINDING: v1 {"evidence": ["Existing seed git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00445cd0.cpp preserves original span445CD0..4468E1 and is retained as a hint.", "Original instructions store this+28EC=-1, publish callback4456B0, copy70words from683AD8, initialize/group the Effect array and global Wave objects, and reset37words at9BC080. The sole indirect call446034 targets Effect[4] virtual set_pitch at slot38."], "reason": "Initialize the real effect and CPU-voice banks and publish source-owned popup/audio defaults; replaces the executable-address forwarder."}
// TRIED: register-faithful transcription (self=ecx, no ebx-copy) diverges at instr #2 (mov ebx,ecx vs push) - /O2 register allocation differs for this 3089-byte, ~150-call function; not chased further.
void FX::init() {
    engine_effect_index_ = -1;
    BasePopDefaultField3094 = popup_wave_callback;
    memcpy(BasePopDefaultBlock2E64, &FxPopupSoundDefaults, sizeof(FxPopupSoundDefaults));

    g_OK_WAVE.Wave::load("fx\\ok.wav", 0);
    g_OK_WAVE.Wave::set_volume(0x3C);
    WaveDeviceGlobal.add_to_group(1, &g_OK_WAVE);
    effects_[0].init("fx\\psi attack.wav", 0);
    WaveDeviceGlobal.add_to_group(1, &effects_[0]);
    effects_[1].init("fx\\energy1.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[1]);
    effects_[2].init("projectile1.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[2]);
    effects_[3].init("projectile2.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[3]);
    effects_[56].init("fx\\mindworms", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[56]);
    effects_[59].init("fx\\planet buster lead.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[59]);
    effects_[60].init("fx\\planet buster.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[60]);
    effects_[57].init("fx\\cpu turn complete", 0x80);
    WaveDeviceGlobal.add_to_group(0, &effects_[57]);
    effects_[58].init("fx\\cpu new data", 0x80);
    WaveDeviceGlobal.add_to_group(0, &effects_[58]);
    effects_[61].init("fx\\psi blow", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[61]);
    effects_[62].init("fx\\boom", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[62]);
    effects_[64].init("fx\\infantry", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[64]);
    effects_[65].init("fx\\military land", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[65]);
    effects_[66].init("fx\\non military land", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[66]);
    effects_[67].init("fx\\grav ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[67]);
    effects_[68].init("fx\\hover", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[68]);
    effects_[69].init("fx\\military ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[69]);
    effects_[70].init("fx\\ship", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[70]);
    effects_[71].init("fx\\jet", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[71]);
    effects_[72].init("fx\\engine2", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[72]);
    effects_[75].init("fx\\missile", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[75]);
    effects_[4].init("wpn laser2.wav", 0xC0);
    effects_[4].set_pitch(1100);
    WaveDeviceGlobal.add_to_group(1, &effects_[4]);
    effects_[73].init("wpn particle.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[73]);
    effects_[74].init("wpn gatlin laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[74]);
    effects_[76].init("wpn chaos gun.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[76]);
    effects_[77].init("wpn fusion laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[77]);
    effects_[78].init("wpn tachyon bolt.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[78]);
    effects_[79].init("wpn plasma shard.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[79]);
    effects_[80].init("wpn quantum laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[80]);
    effects_[81].init("wpn graviton gun.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[81]);
    effects_[85].init("wpn Resonance Laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[85]);
    effects_[86].init("wpn res bolt.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[86]);
    effects_[87].init("wpn string disruptor.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[87]);
    effects_[90].init("wpn spore launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[90]);
    effects_[88].init("wpn singularity laser.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[88]);
    effects_[89].init("wpn missile launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[89]);
    effects_[82].init("wpn conventional payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[82]);
    effects_[83].init("wpn tetonic payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[83]);
    effects_[84].init("wpn fungal payload.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[84]);
    effects_[91].init("alien infantry.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[91]);
    effects_[92].init("alien non military.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[92]);
    effects_[93].init("alien ogre.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[93]);
    effects_[94].init("spore launcher.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[94]);
    effects_[95].init("sea lurker.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[95]);
    effects_[96].init("isle of the deep.wav", 0xC0);
    WaveDeviceGlobal.add_to_group(1, &effects_[96]);
    memset(&AutoSoundDefaults, 0, sizeof(AutoSoundDefaults));

    g_CPU_WAVES[0].Wave::init("fx\\CPU pod recovered.wav", 0x80);
    g_CPU_WAVES[1].Wave::init("fx\\CPU data pod recovered.wav", 0x80);
    g_CPU_WAVES[2].Wave::init("fx\\CPU invalid command.wav", 0x80);
    g_CPU_WAVES[3].Wave::init("fx\\CPU invalid base site.wav", 0x80);
    g_CPU_WAVES[4].Wave::init("fx\\CPU invalid terraform.wav", 0x80);
    g_CPU_WAVES[5].Wave::init("fx\\CPU improved already.wav", 0x80);
    g_CPU_WAVES[6].Wave::init("fx\\CPU lacks ability.wav", 0x80);
    g_CPU_WAVES[7].Wave::init("fx\\CPU combat units only.wav", 0x80);
    g_CPU_WAVES[8].Wave::init("fx\\CPU nocando.wav", 0x80);
    g_CPU_WAVES[9].Wave::init("fx\\CPU insufficient energy.wav", 0x80);
    g_CPU_WAVES[10].Wave::init("fx\\CPU nn already linked.wav", 0x80);
    g_CPU_WAVES[11].Wave::init("fx\\CPU PT insufficient exp.wav", 0x80);
    g_CPU_WAVES[12].Wave::init("fx\\CPU system error.wav", 0x80);
    g_CPU_WAVES[13].Wave::init("fx\\CPU combat report.wav", 0x80);
    g_CPU_WAVES[14].Wave::init("fx\\CPU artifact lost.wav", 0x80);
    g_CPU_WAVES[15].Wave::init("fx\\CPU faction gone.wav", 0x80);
    g_CPU_WAVES[16].Wave::init("fx\\CPU indigenous lifeforms.wav", 0x80);
    g_CPU_WAVES[17].Wave::init("fx\\CPU improvement destroyed.wav", 0x80);
    g_CPU_WAVES[18].Wave::init("fx\\CPU enemy pt gone.wav", 0x80);
    g_CPU_WAVES[20].Wave::init("fx\\CPU terraform complete.wav", 0x80);
    g_CPU_WAVES[21].Wave::init("fx\\CPU mission accomplished.wav", 0x80);
    g_CPU_WAVES[22].Wave::init("fx\\CPU pt compromised.wav", 0x80);
    g_CPU_WAVES[23].Wave::init("fx\\CPU mission aborted.wav", 0x80);
    g_CPU_WAVES[24].Wave::init("fx\\CPU resource shortfall.wav", 0x80);
    g_CPU_WAVES[25].Wave::init("fx\\CPU prod complete.wav", 0x80);
    g_CPU_WAVES[26].Wave::init("fx\\CPU max population.wav", 0x80);
    g_CPU_WAVES[27].Wave::init("fx\\CPU new orders.wav", 0x80);
    g_CPU_WAVES[28].Wave::init("fx\\CPU drone riots.wav", 0x80);
    g_CPU_WAVES[29].Wave::init("fx\\CPU drone riots over.wav", 0x80);
    g_CPU_WAVES[30].Wave::init("fx\\CPU golden age begun.wav", 0x80);
    g_CPU_WAVES[32].Wave::init("fx\\CPU request confirmation.wav", 0x80);
    g_CPU_WAVES[33].Wave::init("fx\\CPU time up.wav", 0x80);
    g_CPU_WAVES[34].Wave::init("fx\\CPU units engaged.wav", 0x80);
    g_CPU_WAVES[35].Wave::init("fx\\CPU are you certain1.wav", 0x80);
    g_CPU_WAVES[36].Wave::init("fx\\CPU project initiated.wav", 0x80);
    g_CPU_WAVES[37].Wave::init("fx\\CPU project complete.wav", 0x80);
    g_CPU_WAVES[38].Wave::init("fx\\CPU enemy project initiated.wav", 0x80);
    g_CPU_WAVES[39].Wave::init("fx\\CPU unit lost.wav", 0x80);
    g_CPU_WAVES[40].Wave::init("fx\\CPU nutrient resources.wav", 0x80);
    g_CPU_WAVES[42].Wave::init("fx\\CPU mineral resources.wav", 0x80);
    g_CPU_WAVES[41].Wave::init("fx\\CPU energy resources.wav", 0x80);
    g_CPU_WAVES[43].Wave::init("fx\\CPU please don't go.wav", 0x80);
    g_CPU_WAVES[44].Wave::init("fx\\CPU review your options.wav", 0x80);
    for (int index = 0; index < 45; ++index) {
        WaveDeviceGlobal.add_to_group(0, &g_CPU_WAVES[index]);
    }
    g_ALPHAMENU_WAVE.Wave::load("fx\\amenu2.wav", 0x11);
    WaveDeviceGlobal.add_to_group(1, &g_ALPHAMENU_WAVE);
    g_ALPHAMENU_WAVE.Sound::set_pan(-32);
    g_ALPHAMENU_WAVE.Wave::set_volume(0x64);
    g_SCOOT_WAVE.Wave::load("fx\\scoot.wav", 0x11);
    WaveDeviceGlobal.add_to_group(1, &g_SCOOT_WAVE);
    g_PASSOVER_WAVE.Wave::load("fx\\passover.wav", 0x11);
    g_PASSOVER_WAVE.Wave::set_volume(0x3C);
    WaveDeviceGlobal.add_to_group(1, &g_PASSOVER_WAVE);
    g_MENU_UP_WAVE.Wave::load("fx\\menu up.wav", 0x10);
    WaveDeviceGlobal.add_to_group(1, &g_MENU_UP_WAVE);
    g_MENU_UP_WAVE.Wave::set_pitch(0xC8);
    g_MENU_UP_WAVE.Wave::set_volume(0x2D);
    g_MENU_DOWN_WAVE.Wave::load("fx\\menu down.wav", 0x10);
    WaveDeviceGlobal.add_to_group(1, &g_MENU_DOWN_WAVE);
    g_MENU_DOWN_WAVE.Wave::set_volume(0x64);
    g_MENU_DOWN_WAVE.Wave::set_pitch(0x12C);
}

/*
Purpose: Range-and-parity screen test for one map coordinate: halve the
         base coordinate read through the map geometry pointer, quarter the
         map height, wrap the sum into range against half the height, and
         report whether the requested half-coordinate falls inside the
         computed band.

// ORIGINAL: 0x004468F0 sub_4468f0 0x004468F0-0x0044695B FILE BYTE_EXACT
// symbol    _sub_4468f0
// size      107 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Status: Coverage transcription - unclaimed body (direct transcription of
        the seed's materialised /O2 arithmetic; no caller in the product
        yet, so the body stands on its own disassembly).
*/
extern "C" int __cdecl sub_4468f0(int a1) {
    const int center = MapWinTable[0]->center_x_ / 2;
    const int quarter = MapLongitudeBounds / 4;
    const int half_width = MapLongitudeBounds / 2;
    const int half = a1 / 2;
    int limit = quarter + center;
    if (limit > half_width) {
        limit -= half_width;
    }
    if (center < quarter) {
        if (half <= center || half >= limit) {
            return -1;
        }
    } else if (half < center && half > limit) {
        return -1;
    }
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00446960 ?play@FX@@QAEXHHH@Z 0x00446960-0x004469F9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/unrecovered/00446960.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void FX::play(int effect, int, int) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) return;
    if (effect == 1 || effect == 0 || effect == 2) {
        if (FactionMusic) FactionMusic->hostility();
        effects_[effect].set_volume(60);
        if (effect == 0) {
            // Original receiver is effects_[0], not a polymorphic FX.
            const int playing = effects_[0].is_playing();
            if (playing) {
                effects_[0].set_pitch(effects_[0].pitch_ + 100);
            } else {
                effects_[0].set_pitch(0);
            }
        }
    }
    const unsigned char release_flags = effects_[effect].flags_54_ >> 4;
    if (release_flags & 1) effects_[effect].load();
    effects_[effect].play();
}
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

/*
Purpose: Trigger one sound effect from the process-wide bank.
// ORIGINAL: 0x00446A00 ?play@FX@@QAEXH@Z 0x00446A00-0x00446A8E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original446A00..446A8E; retired seed git:117282ee:src/unrecovered/00446a00.cpp records the same identity and spans with its shr/test TRIED note kept below.", "0x009A6490 is game.h's GamePreferences and 0x0074DAA0 is popup.h's PopupWaveOwnerSlot; the receiver is fx.h's own FX with effects_ at 0x0 in 0x6C-byte Effect records, and the wave-owner slot 82 plus the effect record's slots 35/7 run through member-fn-pointer unions because the slots' signatures are not declared anywhere.", "The bit-4 release test is the seed's measured ((flags >> 4) & 1) walk, and the wave-owner refresh runs only for effect kinds 0/1/2 exactly as the seed orders them."], "reason": "Recover the sound bank's play trigger on the typed FX receiver; the shr+test release mask stays spelled as the seed measured it."}
// TRIED (seed): 141/142 bytes match (edit distance 1). Only the bit-4 test
//         near the end differs: original is `shr ecx,4 / test cl,1`, every
//         rephrasing tried in the scaffold still lowered to a single fused
//         `test` against the shifted-away mask.
Status: Coverage transcription - unclaimed body
*/
void FX::play(int effect) {
    if (!(GameDataStateStorage.fields_.game_preferences_ & 0x400)) return;
    if (effect == 1 || effect == 0 || effect == 2) {
        if (FactionMusic) FactionMusic->hostility();
        if (effect == 0) {
            const int playing = effects_[0].is_playing();
            if (playing) {
                effects_[0].set_pitch(effects_[0].pitch_ + 100);
            } else {
                effects_[0].set_pitch(0);
            }
        }
    }
    const unsigned char release_flags = effects_[effect].flags_54_ >> 4;
    if (release_flags & 1) effects_[effect].load();
    effects_[effect].play();
}


/*
// ORIGINAL: 0x00446A90 ?stop@FX@@QAEXH@Z 0x00446A90-0x00446AA9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?stop@FX@@QAEXH@Z)(FX* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AA2
Status: Complete
*/
void FX::stop(int a1) {
    effects_[a1].stop();
}

/*
// ORIGINAL: 0x00446AB0 ?release@FX@@QAEXH@Z 0x00446AB0-0x00446AC9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?release@FX@@QAEXH@Z)(FX* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AC2
Status: Complete
*/
void FX::release(int a1) {
    effects_[a1].release();
}

/*
// ORIGINAL: 0x00446AD0 ?fade@FX@@QAEXH@Z 0x00446AD0-0x00446AE9 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?fade@FX@@QAEXH@Z)(FX* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00446AE2
Status: Complete
*/
void FX::fade(int a1) {
    effects_[a1].fade();
}

/*
Purpose: Set a selected effect's pitch through the real virtual method.
// ORIGINAL: 0x00446AF0 ?set_pitch@FX@@QAEXHH@Z 0x00446AF0-0x00446B10 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The 0x6C-stride receiver is the FX-owned Effect array, and slot 38 is Wave::set_pitch in the established virtual hierarchy.","evidence":["Original 0x00446AF0-0x00446B10 multiplies the effect index by 0x6C and dispatches slot 38 with the pitch argument.","fx.h owns Effect[97] at offset zero; wave.h declares virtual set_pitch at slot 38; git:b7e5b127:src/recovered/units/00446af0.cpp preserves the historical exact seed."]}
*/
void FX::set_pitch(int effect, int pitch) {
    effects_[effect].set_pitch(pitch);
}

/*
Purpose: Adjust the active engine effect's pitch if one is selected.
// ORIGINAL: 0x00446B10 ?engine_pitch@FX@@QAEXH@Z 0x00446B10-0x00446B37 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The trailing signed FX member selects one Effect, and the indirect call is its inherited virtual Wave::set_pitch.","evidence":["Original 0x00446B10-0x00446B37 reads signed this+0x28EC, skips negative values and calls slot 38 on the 0x6C-stride Effect array.","FX::init writes -1 to the trailing member; git:b7e5b127:src/recovered/00446b10.cpp preserves the historical exact seed and its signedness note."]}
*/
void FX::engine_pitch(int pitch) {
    if (engine_effect_index_ >= 0) {
        effects_[engine_effect_index_].set_pitch(pitch);
    }
}


// ORIGINAL: 0x004483C0 j_??1Ambience@@QAE@XZ 0x004483C0-0x004483C5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned final Ambience table slot 2 selects 0x00448390, which calls 0x004483C0.", "Pinned 0x004483C0 is a five-byte tail to 0x004C7670, the full destructor now recovered as Midi::~Midi in src/sound.cpp.", "src/ambience.h leaves the Ambience destructor implicit over Midi. Its compiler-generated complete stage tail-calls Midi::~Midi without an intervening derived-table store; the class deleting path retains that stage.", "Historical TRIED prose predates class-stage identification. No new bare-jump wrapper or substitute source behavior is introduced."], "reason": "Bind the original destructor tail to the real compiler-generated empty Ambience derived stage, which delegates destruction to Midi. Preserve the historical catalog and TRIED note as provenance, not current class identification."}
// symbol    ??1Ambience@@UAE@XZ
// Historical TRIED: an authored bare-jump/linker-thunk scaffold was rejected. The tail is instead emitted by the implicit Ambience destructor over Midi; no handwritten wrapper or machine-code body is needed. The target 0x004C7670 is the separately mapped Midi destructor.

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00446B40 0x00446CD8 0x00446D10 0x00446B9A
// ORIGINAL: 0x00446B40 ?ambience@@YAHH@Z 0x00446B40-0x00446CD6
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00446b40.cpp", "recovery/tasks/ambience-faction-art-owner-oct5.json", "pinned complete original spans; fresh native owner compilation and actual-source behavior controls"], "reason": "Complete semantic recovery through genuine source-owned members, globals and lifetime operations; retain original defects and historical obligations. Current byte restoration remains unfinished."}
// symbol    ?ambience@@YAHH@Z
// TRIED: guard bit 0x800, no-op-if-unchanged, in-range switch dispatch on *DAT_0074daa0's vtable (14 cases mapping to slots verified against the raw jump table at 0x446CD8, not Ghidra's summary, which collapsed two distinct case targets), else stop the old FX entry (slot 0x28), start the new one (slot 0x5c), revert on failure (slot 0x20), and FX::play() the result. Landed at 0.89 similarity; diverges at #108 where case 0xc8 and case 0xd0 - same vtable slot, genuinely two separate blocks in the original (`mov eax`/`call [eax+..]` vs `mov edx`/`call [edx+..]`) - compile to one shared jump target here. Tried plain int* stride (`g_00749cf8 + id*27`) against explicit byte-offset char* arithmetic for the FX table addressing; neither changed the divergence.
int __cdecl ambience(int ambience_id) {
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x800) == 0) {
        return ambience_id;
    }

    if (AmbienceCurrentEffect == ambience_id) {
        return AmbienceCurrentEffect;
    }

    if (ambience_id >= 0xC8 && ambience_id < 0xD6) {
        if (FactionMusic == nullptr) {
            return ambience_id;
        }

        switch (ambience_id) {
        case 0xC8: FactionMusic->general(); break;
        case 0xC9: FactionMusic->tech(); break;
        case 0xCA: FactionMusic->popup1(); break;
        case 0xCB: FactionMusic->eot(); break;
        case 0xCC: FactionMusic->hostility(); break;
        case 0xCD: FactionMusic->basewin_show(); break;
        case 0xCE: FactionMusic->basewin_hide(); break;
        case 0xCF: FactionMusic->terraform(); break;
        case 0xD0: FactionMusic->general(); break;
        case 0xD1: FactionMusic->new_base(); break;
        case 0xD2: FactionMusic->energy_resources(); break;
        case 0xD3: FactionMusic->design_window_show(); break;
        case 0xD4: FactionMusic->design_window_hide(); break;
        case 0xD5: FactionMusic->base_liberated(); break;
        }
        return ambience_id;
    }

    AmbienceTransitionMarker = -1;
    const int previous_effect = AmbienceCurrentEffect;
    if (previous_effect >= 0) {
        g_FX.effect(previous_effect).fade();
    }

    AmbienceCurrentEffect = ambience_id;
    if (ambience_id >= 0) {
        if (g_FX.effect(ambience_id).is_playing() != 0) {
            g_FX.effect(AmbienceCurrentEffect).stop();
        }
        g_FX.play(AmbienceCurrentEffect);
    }

    return AmbienceCurrentEffect;
}

// Every branch retains its original probe effect even when it differs from
// the effect played. These are real bank elements, not independent globals.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00446D10 ?ambience_veh@@YAXH@Z 0x00446D10-0x004471EE FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00446d10.cpp", "src/ambience.h", "src/midi.h", "src/fx.h"], "reason": "Original constructor staging, actual event tables and compiler-call contracts identify the genuine audio owner operation; compatible historical seed spans and claims remain available for restoration, not an acceptance verdict."}
void __cdecl ambience_veh(int vehicle) {
    if (vehicle < 0) return;
    const int old = LastAmbienceVehicle;
    // Original quirk: previous VEHICLE id selects an effect directly if<97.
    if (old >= 0 && old != vehicle && old < 97) g_FX.effect(old).fade();
    const uint32_t prefs = GameDataStateStorage.fields_.game_preferences_;
    LastAmbienceVehicle = vehicle;
    if (!(prefs & 0x800)) return;
    if (Vehs[vehicle].order != 0) return;
    const int prototype = Vehs[vehicle].proto_id;
    const VehPrototype &unit = VehPrototypes[prototype];
    if (unit.plan == 8) {
        if (!(Players[Vehs[vehicle].faction_id].rule_flags & 0x80)) {
            g_FX.play(0x42); return;
        }
        if (g_FX.effect(0x5C).is_playing()) return;
        g_FX.play(0x5C); return;
    }
    const unsigned int chassis = unit.chassis_id;
    if (chassis == 0) {
        const unsigned int weapon = unit.weapon_id;
        if (Weapon[weapon].offense_rating == 0) {
            if (g_FX.effect(0x42).is_playing()) return;
            g_FX.play(0x42); return;
        }
        if (!(Players[Vehs[vehicle].faction_id].rule_flags & 0x80)) {
            if (g_FX.effect(0x40).is_playing()) return;
            g_FX.play(0x40); return;
        }
        if (prototype == 0x10 || prototype == 0x11 || prototype == 0x12) {
            if (g_FX.effect(0x5D).is_playing()) return;
            g_FX.play(0x5D); return;
        }
        if (prototype == 8) {
            if (!g_FX.effect(0x38).is_playing()) g_FX.play(0x38);
            return;
        }
        if (prototype != 0xF) {
            if (g_FX.effect(0x5B).is_playing()) return;
            g_FX.play(0x5B); return;
        }
        if (g_FX.effect(0x5E).is_playing()) return;
        g_FX.play(0x5E); return;
    }
    if (chassis == 1) {
        if (g_FX.effect(0x48).is_playing()) return;
        g_FX.play(0x48); return;
    }
    if (chassis == 2) {
        if (g_FX.effect(0x31).is_playing()) return;
        g_FX.play(0x31); return;
    }
    if (prototype == 0xE) {
        if (g_FX.effect(0x5F).is_playing()) return;
        g_FX.play(0x5F); return;
    }
    if (prototype == 9) {
        if (g_FX.effect(0x60).is_playing()) return;
        g_FX.play(0x60); return;
    }
    if (chassis == 3) {
        if (g_FX.effect(0x45).is_playing()) return;
        g_FX.play(0x45); return;
    }
    if (chassis == 4) {
        if (g_FX.effect(0x46).is_playing()) return;
        g_FX.play(0x46); return;
    }
    if (chassis == 5) {
        if (g_FX.effect(0x47).is_playing()) return;
        g_FX.play(0x47); return;
    }
    if (chassis == 6) {
        if (g_FX.effect(0x44).is_playing()) return;
        g_FX.play(0x44); return;
    }
    if (chassis == 7) {
        if (g_FX.effect(0x43).is_playing()) return;
        g_FX.play(0x43); return;
    }
    if (chassis == 8) {
        if (g_FX.effect(0x4B).is_playing()) return;
        g_FX.play(0x4B); return;
    }
    if (prototype != 8) {
        if (static_cast<int8_t>(Chassis[chassis].triad) == 1) {
            if (g_FX.effect(0xC).is_playing()) return;
            g_FX.play(0xC); return;
        }
        if (static_cast<int8_t>(Chassis[chassis].triad) == 2) {
            if (g_FX.effect(0x12).is_playing()) return;
            g_FX.play(0x12); return;
        }
        if (unit.plan == 0xC) {
            if (g_FX.effect(0x14).is_playing()) return;
            g_FX.play(0x14); return;
        }
        const int offense = Weapon[unit.weapon_id].offense_rating;
        if (offense != 0) {
            if (offense <= Chassis[chassis].speed) {
                if (g_FX.effect(0x1A).is_playing()) return;
                g_FX.play(0x1A); return;
            }
            if (offense != 0) {
                if (g_FX.effect(7).is_playing()) return;
                g_FX.play(7); return;
            }
        }
        if (g_FX.effect(0x2E).is_playing()) return;
        g_FX.play(0x2E); return;
    }
    if (!g_FX.effect(0x38).is_playing()) g_FX.play(0x38);
}


/*
Purpose: Run the shared Ambience constructor, publish this variant's vtable,
         and zero the fields this variant owns.

             push esi / mov esi,ecx / call ??0Ambience / xor eax,eax
             mov [esi],0x66c0f8 / mov [esi+0x58],eax ... [esi+0x64],eax
             mov byte [esi+0x6c],al / mov byte [esi+0x6d],al
             mov [esi+0x68],eax / mov eax,esi / pop esi / ret

         The four variants share this shape and differ only in the vtable and
         in how far the tail runs: FactionAmbience stops at 0x68, MAmbience and
         GAmbience add 0x70, SAmbience adds 0x70 and 0x74. That tail is what
         evidences each variant's extent, and it is why the four bodies are
         written out rather than routed through one helper - sharing a body
         would lose the mutation signal that keeps them distinct, exactly as
         the note on 0x004C75B0 in this file already warns.

         The base constructor runs FIRST and publishes the base vtable; the
         store at [this] then overwrites it. Ordering them the other way would
         leave the base's vtable in place.

         0x6c and 0x6d are BYTES. 0x6e and 0x6f are not written, so whatever
         the storage held survives there.

// ORIGINAL: 0x004471F0 ??0FactionAmbience@@QAE@XZ 0x004471F0-0x00447219 BYTE_EXACT
// symbol    ??0FactionAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: the vtable store must NOT be `volatile`. A volatile store is a scheduling barrier for VC6, so the `xor eax, eax` that feeds the seven zero field stores could not be hoisted above it - 13 of 15, with the image's `xor eax, eax` / `mov [esi], vtable` pair in the wrong order. Dropping the qualifier gives 15 of 15, BYTE_EXACT. The store stays alive without it because the seven field stores that follow are not dead, so nothing lets VC6 prove the object unobserved. Measured the same 15 of 15 with the `object[0x000 / 4]` idiom and with the zero stores written before the vtable store, so only the qualifier matters.
//   The catalogued name is the mangled CONSTRUCTOR, `??0FactionAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      41 bytes
// prototype void (__thiscall ??0FactionAmbience@@QAE@XZ)(FactionAmbience* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
// RECOVERY_GENERATED: v1 vtable 0x0066C0F8 0x0066C260 ??_7FactionAmbience@@6B@ 0x004471F0
FactionAmbience::FactionAmbience() {
    // The LEVER/construct-method notes above describe the retired spelling;
    // the compiler now owns base construction, table staging and unwinding.
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_6C_ = 0;
    field_6D_ = 0;
    field_68_ = 0;
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447220 ?begin@FactionAmbience@@QAEXXZ 0x00447220-0x00447221 BYTE_EXACT
// symbol    ?begin@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?begin@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::begin() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447230 ?tech@FactionAmbience@@QAEXXZ 0x00447230-0x00447231 BYTE_EXACT
// symbol    ?tech@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::tech() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447240 ?terraform@FactionAmbience@@QAEXXZ 0x00447240-0x00447241 BYTE_EXACT
// symbol    ?terraform@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?terraform@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::terraform() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447250 ?production@FactionAmbience@@QAEXXZ 0x00447250-0x00447251 BYTE_EXACT
// symbol    ?production@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?production@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::production() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447260 ?general@FactionAmbience@@QAEXXZ 0x00447260-0x00447261 BYTE_EXACT
// symbol    ?general@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?general@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::general() {
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447270 ?basewin_show@FactionAmbience@@QAEXXZ 0x00447270-0x00447275 BYTE_EXACT
// symbol    ?basewin_show@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_show() {
    field_6C_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447280 ?basewin_hide@FactionAmbience@@QAEXXZ 0x00447280-0x00447285 BYTE_EXACT
// symbol    ?basewin_hide@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::basewin_hide() {
    field_6C_ = 0;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447290 ?design_window_show@FactionAmbience@@QAEXXZ 0x00447290-0x00447295 BYTE_EXACT
// symbol    ?design_window_show@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?design_window_show@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_show() {
    field_6D_ = 1;
}


/*
Purpose: FactionAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x004472A0 ?design_window_hide@FactionAmbience@@QAEXXZ 0x004472A0-0x004472A5 BYTE_EXACT
// symbol    ?design_window_hide@FactionAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?design_window_hide@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::design_window_hide() {
    field_6D_ = 0;
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472B0 ?new_base@FactionAmbience@@QAEXXZ 0x004472B0-0x004472B1 BYTE_EXACT
// symbol    ?new_base@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?new_base@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::new_base() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472C0 ?popup1@FactionAmbience@@QAEXXZ 0x004472C0-0x004472C1 BYTE_EXACT
// symbol    ?popup1@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::popup1() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472D0 ?eot@FactionAmbience@@QAEXXZ 0x004472D0-0x004472D1 BYTE_EXACT
// symbol    ?eot@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::eot() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472E0 ?hostility@FactionAmbience@@QAEXXZ 0x004472E0-0x004472E1 BYTE_EXACT
// symbol    ?hostility@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?hostility@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::hostility() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004472F0 ?energy_resources@FactionAmbience@@QAEXXZ 0x004472F0-0x004472F1 BYTE_EXACT
// symbol    ?energy_resources@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?energy_resources@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::energy_resources() {
}

/*
Purpose: FactionAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447300 ?base_liberated@FactionAmbience@@QAEXXZ 0x00447300-0x00447301 BYTE_EXACT
// symbol    ?base_liberated@FactionAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?base_liberated@FactionAmbience@@QAEXXZ)(FactionAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void FactionAmbience::base_liberated() {
}


/*
Purpose: As 004471F0, with the MAmbience vtable and one more field.
// ORIGINAL: 0x00447310 ??0MAmbience@@QAE@XZ 0x00447310-0x0044733C BYTE_EXACT
// symbol    ??0MAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0MAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0MAmbience@@QAE@XZ)(MAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
// RECOVERY_GENERATED: v1 vtable 0x0066BE28 0x0066BF90 ??_7MAmbience@@6B@ 0x00447310
MAmbience::MAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
}


// THE SLOT-7 DISPATCH. Sound declares play(unsigned long) before play(), and
// VC6 gives same-name overloads their slots in REVERSE declaration order, so
// `reinterpret_cast<SlotSeven *>(this)->slot007()` lands one slot low. Sound's table is not this file's to move,
// so the four begin() variants reach slot 7 through a named holder instead -
// the class exists only to spell the convention and is never instantiated.

// ORIGINAL: 0x00447340 ?begin@MAmbience@@QAEXXZ 0x00447340-0x00447356 BYTE_EXACT
// symbol    ?begin@MAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447347 0x00447351
// 0x00447340  ?begin@MAmbience@@QAEXXZ  ->  ?begin@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::begin() {
    this->play_trackset(1);
    play();
}


// ORIGINAL: 0x00447360 ?general@MAmbience@@QAEXXZ 0x00447360-0x0044746A FILE BYTE_EXACT
// symbol    ?general@MAmbience@@UAEXXZ
// size      266 bytes
// prototype void (__thiscall ?general@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044737E 0x0044738B 0x00447399 0x004473E2 0x004473FA 0x00447461
void MAmbience::general() {
    DWORD (WINAPI *const get_time)() = timeGetTime;
    if ((AmbienceClockArmed & 1) == 0) {
        AmbienceClockArmed |= 1;
        AmbienceClock = get_time();
    }
    if (!is_trackset_playing(0x64)) {
        unsigned int now = get_time();
        unsigned int elapsed = now - static_cast<unsigned int>(AmbienceClock);
        int pass = ++MAmbiencePass;
        if (elapsed >= 0x1388 && pass >= 5) {
            MAmbiencePass = 0;
            xpose_trackset(MAmbienceStep, MAmbienceCents);
            int step = (MAmbienceStep)++;
            play_trackset(step);
            if (MAmbienceStep > 7) {
                MAmbienceStep = 5;
            }
            MAmbienceCents += 0x96;
            if (MAmbienceCents > 0x258) {
                MAmbienceCents = -600;
            }
            AmbienceClock = now;
            field_70_ += 0x64;
            if (field_70_ > 0x1f4) {
                field_70_ = -700;
            }
            xpose_trackset(MAmbienceStepBase, field_70_);
        }
    }
}


// ORIGINAL: 0x00447470 ?tech@MAmbience@@QAEXXZ 0x00447470-0x0044748F BYTE_EXACT
// symbol    ?tech@MAmbience@@UAEXXZ
// size      31 bytes
// prototype void (__thiscall ?tech@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447477 0x00447487
// 0x00447470  ?tech@MAmbience@@QAEXXZ  ->  ?tech@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::tech() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(0x69);
    }
}

// ORIGINAL: 0x00447490 ?energy_resources@MAmbience@@QAEXXZ 0x00447490-0x004474AF BYTE_EXACT
// symbol    ?energy_resources@MAmbience@@UAEXXZ
// size      31 bytes
// prototype void (__thiscall ?energy_resources@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447497 0x004474A7
// 0x00447490  ?energy_resources@MAmbience@@QAEXXZ  ->  ?energy_resources@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::energy_resources() {
    if (!this->is_trackset_playing(0x64)) {
        this->play_trackset(4);
    }
}

// ORIGINAL: 0x004474B0 ?production@MAmbience@@QAEXXZ 0x004474B0-0x004474DF FILE BYTE_EXACT
// symbol    ?production@MAmbience@@UAEXXZ
// size      47 bytes
// prototype void (__thiscall ?production@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474B7 0x004474CB 0x004474D7
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004474B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004474b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?production@MAmbience@@QAEXXZ  at 0x004474B0  (47 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void MAmbience::production() {
    Ambience *const self = this;
    if (!self->is_trackset_playing(0x64)) {
        self->xpose_trackset(4, field_70_);
        self->play_trackset(4);
    }
}


// ORIGINAL: 0x004474E0 ?new_base@MAmbience@@QAEXXZ 0x004474E0-0x00447507 BYTE_EXACT
// symbol    ?new_base@MAmbience@@UAEXXZ
// size      39 bytes
// prototype void (__thiscall ?new_base@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004474E7 0x004474F3 0x004474FF
// 0x004474E0  ?new_base@MAmbience@@QAEXXZ  ->  ?new_base@MAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MAmbience::new_base() {
    // Unconditional, with no slot-70 gate - unlike tech() and
    // energy_resources() on this same class. And 0x64 here is the slot-67
    // argument, not the threshold it is elsewhere in the family.
    this->stop_trackset(4);
    this->stop_trackset(0x69);
    this->play_trackset(0x64);
}

// ORIGINAL: 0x00447510 ?begin@UAmbience@@QAEXXZ 0x00447510-0x00447526 BYTE_EXACT
// symbol    ?begin@UAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447515 0x0044751E
// 0x00447510  ?begin@UAmbience@@QAEXXZ  ->  ?begin@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::begin() {
    play();
    this->play_trackset(0);
}

// ORIGINAL: 0x00447530 ?basewin_show@UAmbience@@QAEXXZ 0x00447530-0x0044754A FILE BYTE_EXACT
// symbol    ?basewin_show@UAmbience@@UAEXXZ
// size      26 bytes
// prototype void (__thiscall ?basewin_show@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044753E
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447530
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447530/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?basewin_show@UAmbience@@QAEXXZ  at 0x00447530  (26 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void UAmbience::basewin_show() {
    if (!field_6C_) {
        this->play_trackset(6);
        field_6C_ = 1;
    }
}


// ORIGINAL: 0x00447550 ?basewin_hide@UAmbience@@QAEXXZ 0x00447550-0x00447563 BYTE_EXACT
// symbol    ?basewin_hide@UAmbience@@UAEXXZ
// size      19 bytes
// prototype void (__thiscall ?basewin_hide@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447557
// 0x00447550  ?basewin_hide@UAmbience@@QAEXXZ  ->  ?basewin_hide@UAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void UAmbience::basewin_hide() {
    this->stop_trackset(6);
    field_6C_ = 0;
}

// ORIGINAL: 0x00447570 ?general@UAmbience@@QAEXXZ 0x00447570-0x00447684 FILE BYTE_EXACT
// symbol    ?general@UAmbience@@UAEXXZ
// size      276 bytes
// prototype void (__thiscall ?general@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004475D1 0x004475EB 0x00447620 0x00447650 0x00447675
void UAmbience::general() {
    if (field_6C_ != 0) {
        return;
    }
    ++UAmbiencePass3;
    ++UAmbiencePass0;
    ++UAmbiencePass1;
    ++UAmbiencePass2;
    if (UAmbiencePass3 == 0xf) {
        UAmbienceTrackset = (UAmbienceTrackset == 4) + 4;
        play_trackset(UAmbienceTrackset);
        UAmbiencePass3 = 0;
    }
    if (UAmbiencePass0 == 9) {
        play_trackset(0);
        UAmbiencePass0 = 0;
    }
    if (UAmbiencePass1 == 6) {
        UAmbienceStep = (UAmbienceStep != 2) + 1;
        xpose_trackset(UAmbienceStep, UAmbienceCents);
        UAmbienceCents += 0x64;
        if (UAmbienceStep > 0x258) {
            UAmbienceStep = -600;
        }
        play_trackset(UAmbienceStep);
        UAmbiencePass1 = 0;
    }
    if (UAmbiencePass2 == UAmbienceStepEvery) {
        play_trackset(UAmbienceStepBase);
        UAmbiencePass2 = 0;
    }
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447690 ?tech@UAmbience@@QAEXXZ 0x00447690-0x00447691 BYTE_EXACT
// symbol    ?tech@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::tech() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476A0 ?popup1@UAmbience@@QAEXXZ 0x004476A0-0x004476A1 BYTE_EXACT
// symbol    ?popup1@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::popup1() {
}

/*
Purpose: UAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x004476B0 ?eot@UAmbience@@QAEXXZ 0x004476B0-0x004476B1 BYTE_EXACT
// symbol    ?eot@UAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void UAmbience::eot() {
}

// ORIGINAL: 0x004476C0 ?begin@SAmbience@@QAEXXZ 0x004476C0-0x004476D6 BYTE_EXACT
// symbol    ?begin@SAmbience@@UAEXXZ
// size      22 bytes
// prototype void (__thiscall ?begin@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004476C5 0x004476CE
// 0x004476C0  ?begin@SAmbience@@QAEXXZ  ->  ?begin@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::begin() {
    play();
    this->play_trackset(0x11);
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// ORIGINAL: 0x004476E0 ?tech@SAmbience@@QAEXXZ 0x004476E0-0x004476E4 BYTE_EXACT
// symbol    ?tech@SAmbience@@UAEXXZ
// size      4 bytes
// prototype void (__thiscall ?tech@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Original SAmbience table 0x0066BB58 slot76 selects this member increment,
// not the empty FactionAmbience::tech. The raw-accessor prose above is history.
void SAmbience::tech() {
    ++field_58_;
}


// ORIGINAL: 0x004476F0 ?general@SAmbience@@QAEXXZ 0x004476F0-0x004477A9 FILE BYTE_EXACT
// symbol    ?general@SAmbience@@UAEXXZ
// LEVER: the delegate vtable - declaring Sound's 33 + the sequencer's 42 on
//   Ambience (ambience.h slots 33..74) is what turned the VCall shims into
//   real `call dword ptr [eax+0xNNN]` dispatches and brought this body from
//   an unverifiable artifact to 51 of 51 instructions structurally.
// TRIED: clamp through a named local read once (`int tick = *p; if (tick >
//   0x12) { tick = 0x11; *p = tick; }`) - the image holds the clamped tick in
//   ecx across the store AND the xpose argument, this spelling still lands it
//   in eax, 1 of 51. Plain `if (*p > 0x12) *p = 0x11;` gives the immediate
//   store the image does not emit. Also open: `(*p)++ > n` compiles
//   `lea ecx,[eax+1]` where the image has `mov edx,eax; inc eax`, and the
//   trailing `++*p` compiles load/inc/store where the image has one
//   `inc dword ptr [0x683c14]`. Register-preference plateau, not a semantic
//   one - same 51 instructions, same calls, same order.
// size      185 bytes
// prototype void (__thiscall ?general@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447727 0x00447738 0x00447767 0x00447773

void SAmbience::general() {
    // Read the clamped shared tick at the xpose call. Keep the later play_trackset
    // read after xpose, since that callback may change AmbienceGeneralTick.
    if (AmbienceGeneralTick > 0x12) {
        AmbienceGeneralTick = 0x11;
    }
    if ((SAmbiencePass0)++ > 1) {
        xpose_trackset(AmbienceGeneralTick, SAmbienceCents);
        play_trackset(AmbienceGeneralTick);
        SAmbiencePass0 = 0;
    }
    if ((SAmbiencePass1)++ > 7) {
        xpose_trackset(0x10, SAmbienceCents);
        play_trackset(0x10);
        SAmbiencePass1 = 0;
    }
    SAmbienceCents += 0x64;
    if (SAmbienceCents > 0x190) {
        SAmbienceCents = -300;
    }
    ++AmbienceGeneralTick;
}

// ORIGINAL: 0x004477B0 ?hostility@SAmbience@@QAEXXZ 0x004477B0-0x00447831 FILE BYTE_EXACT
// symbol    ?hostility@SAmbience@@UAEXXZ
// LEVER: field signedness. field_70_ and field_74_ are int32_t, not
//   uint32_t - the image compares each against a positive bound with `jle`
//   and wraps it downward past zero, and `uint32_t` spelled `jbe` twice for a
//   2-instruction loss. Together with the delegate vtable on Ambience this
//   took the homed artifact from 16 of 46 to 46 of 46.
// LEVER: the artifact's `VCall2 *self = (VCall2 *)this` pun and its
//   `field_70_ + 1` / store local are both gone - the member form with the
//   declared virtuals is byte-identical and carries no semantic debt.
// size      129 bytes
// prototype void (__thiscall ?hostility@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004477B7 0x004477D7 0x00447802 0x00447810
void SAmbience::hostility() {
    if (is_trackset_playing(0x64)) {
        return;
    }
    if (field_70_ == 0) {
        field_70_ = 0x64;
    }
    if (is_trackset_playing(field_70_)) {
        return;
    }
    field_70_ += 1;
    if (field_70_ > 0x67) {
        field_70_ = 0x65;
    }
    xpose_trackset(field_70_, field_74_);
    play_trackset(field_70_);
    field_74_ += 0x64;
    if (field_74_ > 0x12c) {
        field_74_ = 0xfffffed4;
    }
}


// ORIGINAL: 0x00447840 ?production@SAmbience@@QAEXXZ 0x00447840-0x0044784B BYTE_EXACT
// symbol    ?production@SAmbience@@UAEXXZ
// size      11 bytes
// prototype void (__thiscall ?production@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447844
// 0x00447840  ?production@SAmbience@@QAEXXZ  ->  ?production@SAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void SAmbience::production() {
    this->play_trackset(0xf);
}


/*
Purpose: As 004471F0, with the SAmbience vtable and two more fields.
// ORIGINAL: 0x00447850 ??0SAmbience@@QAE@XZ 0x00447850-0x0044787F BYTE_EXACT
// symbol    ??0SAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0SAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      47 bytes
// prototype void (__thiscall ??0SAmbience@@QAE@XZ)(SAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
// RECOVERY_GENERATED: v1 vtable 0x0066BB58 ? ??_7SAmbience@@6B@ 0x00447850
SAmbience::SAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
    field_74_ = 0;
}

// ORIGINAL: 0x00447880 ?base_liberated@SAmbience@@QAEXXZ 0x00447880-0x004478C3 FILE BYTE_EXACT
// symbol    ?base_liberated@SAmbience@@UAEXXZ
// size      67 bytes
// prototype void (__thiscall ?base_liberated@SAmbience@@QAEXXZ)(SAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447887 0x00447893 0x0044789F 0x004478AB 0x004478BB
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447880
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447880/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_liberated@SAmbience@@QAEXXZ  at 0x00447880  (67 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void SAmbience::base_liberated() {
    Ambience *const self = this;
    self->stop_trackset(0x65);
    self->stop_trackset(0x66);
    self->stop_trackset(0x67);
    if (!self->is_trackset_playing(0x64)) {
        self->play_trackset(0x64);
    }
}
// ORIGINAL: 0x004478D0 ??0BAmbience@@QAE@XZ 0x004478D0-0x00447911 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/004478d0.cpp", "src/ambience.h", "src/midi.h", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004478d0.cpp"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ??0BAmbience@@QAE@XZ
// RECOVERY_GENERATED: v1 vtable 0x0066B9F0 0x0066BB58 ??_7BAmbience@@6B@ 0x004478D0
BAmbience::BAmbience() {
    field_70_ = 1;
    field_74_ = 0;
    field_78_ = 20;
    field_7C_ = 15;
}

// ORIGINAL: 0x00447920 ?begin@BAmbience@@QAEXXZ 0x00447920-0x00447936 BYTE_EXACT
// symbol    ?begin@BAmbience@@UAEXXZ
void BAmbience::begin() {
    play_trackset(0x1A);
    play();
}

// ORIGINAL: 0x00447940 ?general@BAmbience@@QAEXXZ 0x00447940-0x00447AA9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447940.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?general@BAmbience@@UAEXXZ
void BAmbience::general() {
    field_74_ += 500;
    if (field_74_ > 500) field_74_ = -1000;
    const uint8_t hostility = BAmbienceHostility;
    ++BAmbienceAmbientPass;
    ++BAmbienceMusicPass;
    ++BAmbienceTrackPass;
    if (hostility) {
        const bool playing = Midi::is_trackset_playing(4);
        if (playing) return;
        BAmbienceHostility = playing;
    }
    if (field_6D_) {
        const int track = field_78_++;
        play_trackset(track);
        if (field_78_ > 22) field_78_ = 20;
        if (BAmbienceAmbientPass >= 3) {
            play_trackset(field_7C_);
            BAmbienceAmbientPass = 0;
        }
        field_7C_ += 2;
        if (field_7C_ == 21) field_7C_ = 16;
        if (field_7C_ == 22) field_7C_ = 16;
    }
    if (BAmbienceAmbientPass >= 3) {
        const int track = BAmbienceAlternate ? 24 : 25;
        xpose_trackset(track, field_74_);
        if (field_70_ > 1) {
            xpose_trackset(11, field_74_);
            play_trackset(11);
        }
        play_trackset(track);
        BAmbienceAlternate = !BAmbienceAlternate;
    }
    if (BAmbienceMusicPass >= 5) {
        xpose_trackset(13, field_74_);
        play_trackset(13);
        BAmbienceMusicPass = 0;
    }
}

// ORIGINAL: 0x00447AB0 ?design_window_show@BAmbience@@QAEXXZ 0x00447AB0-0x00447AB5 BYTE_EXACT
// symbol    ?design_window_show@BAmbience@@UAEXXZ
void BAmbience::design_window_show() {
    field_6D_ = 1;
}

// ORIGINAL: 0x00447AC0 ?design_window_hide@BAmbience@@QAEXXZ 0x00447AC0-0x00447AEB BYTE_EXACT
// symbol    ?design_window_hide@BAmbience@@UAEXXZ
void BAmbience::design_window_hide() {
    field_6D_ = 0;
    stop_trackset(0x14);
    stop_trackset(0x15);
    stop_trackset(0x16);
}

// ORIGINAL: 0x00447AF0 ?hostility@BAmbience@@QAEXXZ 0x00447AF0-0x00447B2D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447af0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?hostility@BAmbience@@UAEXXZ
void BAmbience::hostility() {
    if (!BAmbienceHostility) {
        BAmbienceHostility = 1;
        xpose_trackset(15, field_74_);
        play_trackset(15);
        play_trackset(27);
    }
}

// ORIGINAL: 0x00447B30 ?new_base@BAmbience@@QAEXXZ 0x00447B30-0x00447B58 BYTE_EXACT
// symbol    ?new_base@BAmbience@@UAEXXZ
void BAmbience::new_base() {
    const int reported = field_74_;
    field_70_ += 1;
    xpose_trackset(0x0D, reported);
    play_trackset(0x0D);
}

// ORIGINAL: 0x00447B60 ?base_liberated@BAmbience@@QAEXXZ 0x00447B60-0x00447B8B BYTE_EXACT
// symbol    ?base_liberated@BAmbience@@UAEXXZ
void BAmbience::base_liberated() {
    stop_trackset(0x1B);
    if (!is_trackset_playing(0x64)) {
        play_trackset(0x64);
    }
}


/*
Purpose: As 004471F0, with the GAmbience vtable and one more field.
// ORIGINAL: 0x00447B90 ??0GAmbience@@QAE@XZ 0x00447B90-0x00447BBC BYTE_EXACT
// symbol    ??0GAmbience@@QAE@XZ
// DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the
// derived-stage vptr overlay store left with the de-management sweep.

// LEVER: same as 0x004471F0 - the vtable store must not be `volatile`, because the barrier stops VC6 hoisting the `xor eax, eax` the zero field stores share. Dropping it took this body byte-exact too.
//   The catalogued name is the mangled CONSTRUCTOR, `??0GAmbience@@QAE@XZ`,
//   but this tree spells the body as a `construct()` METHOD - see
//   `Win::construct`, the idiom it follows. Without this fact `measure`
//   cannot find the body in the object at all and refuses.
// size      44 bytes
// prototype void (__thiscall ??0GAmbience@@QAE@XZ)(GAmbience* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C8460
Return Value: `this`
Status: Complete
*/
// RECOVERY_GENERATED: v1 vtable 0x0066BF90 0x0066C0F8 ??_7GAmbience@@6B@ 0x00447B90
GAmbience::GAmbience() {
    // Historical construct-method notes above do not describe this real ctor.
    field_70_ = 0;
}

// ORIGINAL: 0x00447BC0 ?begin@GAmbience@@QAEXXZ 0x00447BC0-0x00447BDD BYTE_EXACT
// symbol    ?begin@GAmbience@@UAEXXZ
// size      29 bytes
// prototype void (__thiscall ?begin@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447BC5 0x00447BD5
// 0x00447BC0  ?begin@GAmbience@@QAEXXZ  ->  ?begin@GAmbience@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void GAmbience::begin() {
    play();
    // Alone among the six begin() overrides, this one also stores its mode.
    field_68_ = 0x18;
    this->play_trackset(0x18);
}


/*
Purpose: GAmbience raises the flag byte only when it is CLEAR.

             mov al,[ecx+0x6C] / test al,al / jne done
             mov byte [ecx+0x6C],1 / done: ret

         The guard is not redundant. Its sibling basewin_hide below stores
         unconditionally, and an unconditional store here would agree for the
         values 0 and 1 and differ for every other: a byte holding 5 keeps 5,
         where `field_6C_ = 1` would overwrite it.
// ORIGINAL: 0x00447BE0 ?basewin_show@GAmbience@@QAEXXZ 0x00447BE0-0x00447BEC BYTE_EXACT
// symbol    ?basewin_show@GAmbience@@UAEXXZ
// size      12 bytes
// prototype void (__thiscall ?basewin_show@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_show() {
    if (field_6C_ == 0) {
        field_6C_ = 1;
    }
}


/*
Purpose: GAmbience records the event in a flag byte and does nothing else;
         the legacy implementation is a single store.
// ORIGINAL: 0x00447BF0 ?basewin_hide@GAmbience@@QAEXXZ 0x00447BF0-0x00447BF5 BYTE_EXACT
// symbol    ?basewin_hide@GAmbience@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?basewin_hide@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::basewin_hide() {
    field_6C_ = 0;
}

// ORIGINAL: 0x00447C00 ?general@GAmbience@@QAEXXZ 0x00447C00-0x00447C58 FILE BYTE_EXACT
// symbol    ?general@GAmbience@@UAEXXZ
// size      88 bytes
// prototype void (__thiscall ?general@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C0F
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@GAmbience@@QAEXXZ  at 0x00447C00  (88 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::general() {
    const uint32_t arg68 = field_68_;

    this->xpose_trackset(arg68, field_70_);

    if (UAmbienceGate == 0x18) {
        field_70_ += 300;
        if (field_70_ > 600) {
            field_70_ = -600;
        }
        return;
    }
    field_70_ += 700;
    if (field_70_ > 1200) {
        field_70_ = -700;
    }
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447C60 ?tech@GAmbience@@QAEXXZ 0x00447C60-0x00447C61 BYTE_EXACT
// symbol    ?tech@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?tech@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::tech() {
}

// ORIGINAL: 0x00447C70 ?terraform@GAmbience@@QAEXXZ 0x00447C70-0x00447C9A FILE BYTE_EXACT
// symbol    ?terraform@GAmbience@@UAEXXZ
// size      42 bytes
// prototype void (__thiscall ?terraform@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447C79
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447C70
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447c70/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?terraform@GAmbience@@QAEXXZ  at 0x00447C70  (42 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::terraform() {
    this->play_trackset(GAmbienceStep);
    if (++GAmbienceStep > 9) {
        GAmbienceStep = 7;
    }
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CA0 ?production@GAmbience@@QAEXXZ 0x00447CA0-0x00447CA1 BYTE_EXACT
// symbol    ?production@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?production@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::production() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CB0 ?popup1@GAmbience@@QAEXXZ 0x00447CB0-0x00447CB1 BYTE_EXACT
// symbol    ?popup1@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?popup1@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::popup1() {
}

/*
Purpose: GAmbience ignores this event; the legacy implementation is a bare return.
// ORIGINAL: 0x00447CC0 ?eot@GAmbience@@QAEXXZ 0x00447CC0-0x00447CC1 BYTE_EXACT
// symbol    ?eot@GAmbience@@UAEXXZ
// size      1 bytes
// prototype void (__thiscall ?eot@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void GAmbience::eot() {
}

// ORIGINAL: 0x00447CD0 ?new_base@GAmbience@@QAEXXZ 0x00447CD0-0x00447D07 FILE BYTE_EXACT
// symbol    ?new_base@GAmbience@@UAEXXZ
// size      55 bytes
// prototype void (__thiscall ?new_base@GAmbience@@QAEXXZ)(GAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00447CE5 0x00447CFF
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447CD0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447cd0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?new_base@GAmbience@@QAEXXZ  at 0x00447CD0  (55 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void GAmbience::new_base() {
    SAmbienceArmed = 1;

    this->stop_trackset(field_68_);
    field_68_ = 0x19;
    field_70_ = 0;
    this->play_trackset(0x19);
}

// ORIGINAL: 0x00447D20 sub_447d20 0x00447D20-0x00447DA5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x00447D20..0x00447DA5: virtual calls at offsets 0x118, 0x114 and 0x10C have the argument/return behavior restored by Midi::is_trackset_playing, Midi::xpose_trackset and Midi::play_trackset.", "Pinned original initializes 0x00683C2C to signed -0x4B0; only this function has an observed source writer/use. Source owns a signed int initialized to that value; full original allocation and indirect writers remain unknown.", "Historical settings-checkbox/repaint interpretation is retained as superseded binding history in stringstruct.cpp, not as current behavior."], "reason": "Recover the pinned trackset cascade through the existing Midi status, transpose and play virtual methods. The dynamic subclass, original helper/member distinction, call lifecycle and full allocation of the shared transposition cell remain unproven."}
// symbol    ?advance_trackset_transposition@Midi@@QAEXXZ
void Midi::advance_trackset_transposition() {
    if (!is_trackset_playing(1)) {
        if (!is_trackset_playing(2)) {
            if (MidiTracksetTransposition == 0) {
                MidiTracksetTransposition = -0x4B0;
            }
            MidiTracksetTransposition = MidiTracksetTransposition + 0x12C;
            xpose_trackset(1, MidiTracksetTransposition);
            play_trackset(1);
            return;
        }
    }
    if (!is_trackset_playing(2)) {
        xpose_trackset(2, MidiTracksetTransposition);
        play_trackset(2);
    }
}

// ORIGINAL: 0x00447DB0 ?begin@AAmbience@@QAEXXZ 0x00447DB0-0x00447DC6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/00447db0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?begin@AAmbience@@UAEXXZ
void AAmbience::begin() {
    play();
    play_trackset(3);
}

// ORIGINAL: 0x00447DD0 ?general@AAmbience@@QAEXXZ 0x00447DD0-0x00448273 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f7bbce35:src/recovered/units/00447dd0.cpp", "src/ambience.h", "src/midi.h"], "reason": "Original audio event tables and compiler-call contracts identify this genuine owner operation; compatible historical seeds and matching obligations remain retained, not an acceptance verdict."}
// symbol    ?general@AAmbience@@UAEXXZ
void AAmbience::general() {
    ++AAmbiencePass;
    if (AAmbiencePhase == 1) {
        if (AAmbiencePass % 7 != 0) goto finish_phase;
        xpose_trackset(14, AAmbienceCents);
        xpose_trackset(15, AAmbienceCents);
        xpose_trackset(16, AAmbienceCents);
        xpose_trackset(13, AAmbienceCents);
        AAmbienceCents += AAmbienceCentsStep;
        play_trackset(14);
        play_trackset(2);
        play_trackset(1);
        if (AAmbienceCents >= 300 || AAmbienceCents < -900)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        if (AAmbiencePass > 10) play_trackset(15);
        if (AAmbiencePass > 20) play_trackset(16);
        if (AAmbiencePass > 25) play_trackset(13);
    }
    if (AAmbiencePhase == 2) {
        play_trackset(27);
        AAmbienceCents += AAmbienceCentsStep;
        if (AAmbienceCents >= 100 || AAmbienceCents < -600)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        AAmbienceLevel += 10;
        AAmbienceAlternate ^= 1;
        if (AAmbienceAlternate) play_trackset(AAmbienceTrack1);
        else play_trackset(AAmbienceTrack0);
        if (AAmbienceTrack2 != 25) play_trackset(AAmbienceTrack2);
        if (++AAmbienceTrack0 > 20) AAmbienceTrack0 = 18;
        if (++AAmbienceTrack1 > 23) AAmbienceTrack1 = 21;
        if (++AAmbienceTrack2 > 26) AAmbienceTrack2 = 24;
    }
    if (AAmbiencePhase == 3) {
        AAmbiencePhase3Cents += AAmbienceCentsStep;
        if (AAmbiencePhase3Cents >= 100 || AAmbiencePhase3Cents < -600)
            AAmbienceCentsStep = -AAmbienceCentsStep;
        xpose_trackset(8, AAmbiencePhase3Cents);
        xpose_trackset(3, AAmbiencePhase3Cents);
        xpose_trackset(8, AAmbiencePhase3Cents);
        xpose_trackset(6, AAmbiencePhase3Cents);
        xpose_trackset(5, AAmbiencePhase3Cents);
        xpose_trackset(2, AAmbiencePhase3Cents);
        xpose_trackset(4, AAmbiencePhase3Cents);
        if (AAmbiencePass == 5 || AAmbiencePass == 15) play_trackset(5);
        if (AAmbienceRandomRange > 5) AAmbienceRandomRange = 1;
        else ++AAmbienceRandomRange;
        unsigned int track = timeGetTime() % static_cast<unsigned int>(AAmbienceRandomRange);
        if (track == 3 || (track == 8 && AAmbiencePhase3Cents >= 0)) ++track;
        if (!is_trackset_playing(track)) play_trackset(track);
    }
finish_phase:
    if (AAmbiencePass < 40) return;
    if (AAmbiencePhase == 1) {
        stop_trackset(13);
        stop_trackset(15);
        stop_trackset(16);
        AAmbiencePhase = 2;
        sub_4c7f70(14, 0);
        AAmbiencePass = 0;
        return;
    }
    if (AAmbiencePhase == 2) {
        stop_trackset(18);
        stop_trackset(19);
        stop_trackset(20);
        stop_trackset(21);
        stop_trackset(22);
        stop_trackset(23);
        stop_trackset(24);
        stop_trackset(25);
        stop_trackset(26);
        stop_trackset(27);
        sub_4c7f70(14, 1);
        play_trackset(3);
        AAmbiencePhase = 3;
        AAmbiencePass = 0;
        return;
    }
    if (AAmbiencePhase == 3) {
        stop_trackset(1);
        stop_trackset(2);
        stop_trackset(3);
        stop_trackset(4);
        stop_trackset(5);
        stop_trackset(6);
        stop_trackset(7);
        stop_trackset(8);
        AAmbiencePhase = 1;
    }
    AAmbiencePass = 0;
}

/*
Purpose: Increment the dword field at 0x58.
         Emitted by tools/generate_field_accessors.py from

             inc dword ptr [ecx + 0x58] / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// ORIGINAL: 0x00448280 ?tech@AAmbience@@QAEXXZ 0x00448280-0x00448284 BYTE_EXACT
// symbol    ?tech@AAmbience@@UAEXXZ
// size      4 bytes
// prototype void (__thiscall ?tech@AAmbience@@QAEXXZ)(AAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AAmbience::tech() {
    ++field_58_;
}


/*
Purpose: Destroy the contained Effect objects in reverse construction order.
// ORIGINAL: 0x00448290 ??1FX@@QAE@XZ 0x00448290-0x004482A0 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??1FX@@QAE@XZ)(FX* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FX::~FX() {
}

// The implicit Effect constructor at 0x004482D0 installs the derived table;
// its Wave tail includes FX::init's virtual pitch target at slot 38.
// RECOVERY_GENERATED: v1 vtable 0x0066B7A4 ? ??_7Effect@@6B@ 0x004482D0

/*
Purpose: Construct the 97 contained Effect objects, including compiler-owned
         cleanup of already-constructed elements if construction throws.
// ORIGINAL: 0x004482A0 ??0FX@@QAE@XZ 0x004482A0-0x004482BB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ??0FX@@QAE@XZ)(FX* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006457C2
Return Value: n/a (the constructor ABI returns the object pointer in eax)
Status: Complete
*/
FX::FX() {
}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004482D0 ??0Effect@@QAE@XZ 0x004482D0-0x004482E2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original FX construction at0x004482A0 passes0x004482D0 as its per-element constructor for97elements of0x6Cbytes.", "Existing seedsrc/recovered/units/004482d0.cpp identifies span0x004482D0-0x004482E2; original instructions call Wave::Wave at0x004C66E0 then store the Effect virtual table at0x0066B7A4."], "reason": "Implicit Effect constructor emitted for the real FX Effect array; Wave base construction and derived virtual-table installation are compiler-owned."}


// Original family tables share slot 1 at 0x004482F0. Unlike set_fade,
// this changes the remembered interval without notifying the wrapped device.
// The behavioral name is provisional; zero refuses without changing state.
// ORIGINAL: 0x004482F0 sub_4482f0 0x004482F0-0x0044830C BYTE_EXACT
// symbol    ?set_fade_interval@SoundFadeState@@UAEHK@Z
int SoundFadeState::set_fade_interval(unsigned long interval) {
    if (interval == 0) {
        return 10;
    }
    fade_38_ = interval;
    return 0;
}


// Purpose: Return the next Sound in the common chain (family slot 25).
// Ownership is also witnessed by attach/detach's typed chain manipulation.
// ORIGINAL: 0x00448310 sub_448310 0x00448310-0x00448314 BYTE_EXACT
// symbol    ?get_next@Sound@@UAEPAV1@XZ
Sound *Sound::get_next() {
    return chain_next_;
}

// Purpose: Return the previous Sound in the common chain (family slot 26).
// ORIGINAL: 0x00448320 sub_448320 0x00448320-0x00448324 BYTE_EXACT
// symbol    ?get_prev@Sound@@UAEPAV1@XZ
Sound *Sound::get_prev() {
    return chain_prev_;
}

// Purpose: Return the type field written by Sound::set_type (family slot 30).
// ORIGINAL: 0x00448330 sub_448330 0x00448330-0x00448334 BYTE_EXACT
// symbol    ?get_type@Sound@@UAEIXZ
uint32_t Sound::get_type() {
    return type_;
}

// The Windows multimedia clock is a system import, not storage in this build.
// RECOVERY_IMPORT: v1 __imp__timeGetTime@0 WINMM.dll name:timeGetTime


// Historical seed catalog (inactive): // prior-original: 0x00448340 sub_448340 0x00448340-0x0044834F historical-byte-exact
// Historical seed catalog (inactive): // symbol    ?Method@S_00448340@@QAEHXZ
// Historical seed catalog (inactive): // size      15 bytes
// Historical seed catalog (inactive): // prototype
// Historical seed catalog (inactive): // callers   0   call targets   0
// Historical seed catalog (inactive): // kind      game
// Historical seed catalog (inactive): // flags     hidden;sp_ready;purged_ok
// Historical seed catalog (inactive): // calls     (none)
// Historical seed catalog (inactive): // 0x00448340  sub_448340  ->  _sub_448340
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// Historical seed catalog (inactive): // OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// Historical seed catalog (inactive): // and rewriting it in the tree's own style is a later phase. See README.md
// Historical seed catalog (inactive): // beside this file. Re-verified in bulk by byte_match_fanout.py --collect.
// Historical seed catalog (inactive): address=0x00448340
// Historical seed catalog (inactive): mode=body; state=implemented
// Historical seed catalog (inactive): name=sub_448340; symbol=?Method@S_00448340@@QAEHXZ
// Historical seed catalog (inactive): spans=((4490048, 4490063),)
// Historical seed catalog (inactive): kind='game'; exclusion=''
// Historical seed catalog (inactive): byte_exact=True; semantic=False
// Historical seed catalog (inactive): recipe=census
// Historical seed catalog (inactive): levers=()
// Historical seed catalog (inactive): ruled_out=()
// Historical seed catalog (inactive): unrecoverable=()
// Historical seed catalog (inactive): deferred=()
// ORIGINAL: 0x00448340 sub_448340 0x00448340-0x0044834F BYTE_EXACT
// symbol    ?unk_slot24@Wave@@UAEHXZ
int Wave::unk_slot24() {
    if (device_ != 0) {
        return device_->unk_slot24();
    }
    return 0;
}

// Historical seed catalog (inactive): /*
// Historical seed catalog (inactive): Purpose: Sign-extend bit 0 of field 0x40: 0 or -1.
// Historical seed catalog (inactive):          Emitted by tools/generate_field_accessors.py from
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):              mov eax, dword ptr [ecx + 0x40] / shl eax, 0x1f / sar eax, 0x1f / ret
// Historical seed catalog (inactive):
// Historical seed catalog (inactive):          The name is the ADDRESS, not an invention, and `self` is a void
// Historical seed catalog (inactive):          pointer because this body needs `this` to be nothing more than a
// Historical seed catalog (inactive):          pointer and an offset - the class it belongs to is not established.
// Historical seed catalog (inactive):          The adapter declares 0 stack dword(s) so it
// Historical seed catalog (inactive):          cleans 0 bytes, taken from the body's own `ret` -
// Historical seed catalog (inactive):          the only statement of this function's arity that exists, since it has
// Historical seed catalog (inactive):          no mangled name. Declaring fewer would leave them on the caller's
// Historical seed catalog (inactive):          stack.
// Historical seed catalog (inactive): // prior-original: 0x00448380 sub_448380 0x00448380-0x0044838A historical-byte-exact
// Historical seed catalog (inactive): // symbol    ?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): // size      10 bytes
// Historical seed catalog (inactive): // prototype
// Historical seed catalog (inactive): // callers   0   call targets   0
// Historical seed catalog (inactive): // kind      game
// Historical seed catalog (inactive): // flags     hidden;sp_ready;purged_ok
// Historical seed catalog (inactive): // calls     (none)
// Historical seed catalog (inactive): Return Value: the value described above
// Historical seed catalog (inactive): Status: Complete
// Historical seed catalog (inactive): */
// Historical seed catalog (inactive): address=0x00448380
// Historical seed catalog (inactive): mode=body; state=implemented
// Historical seed catalog (inactive): name=sub_448380; symbol=?field_accessor_00448380_redirect@@YIIPAX0@Z
// Historical seed catalog (inactive): spans=((4490112, 4490122),)
// Historical seed catalog (inactive): kind='game'; exclusion=''
// Historical seed catalog (inactive): byte_exact=True; semantic=False
// Historical seed catalog (inactive): recipe=census
// Historical seed catalog (inactive): levers=()
// Historical seed catalog (inactive): ruled_out=()
// Historical seed catalog (inactive): unrecoverable=()
// Historical seed catalog (inactive): deferred=()
// ORIGINAL: 0x00448380 sub_448380 0x00448380-0x0044838A BYTE_EXACT
// symbol    ?unk_slot24@Sound@@UAEHXZ
// The original Sound table selects 0x00448380 at slot 24. This
// inherited integer contract sign-extends flags_40_ bit zero.
int Sound::unk_slot24() {
    return static_cast<int>(flags_40_ << 31) >> 31;
}

