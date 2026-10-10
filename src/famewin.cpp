// ORIGINAL: 0x0044B330 ??_GFameWin@@WEEE@AEPAXI@Z 0x0044B330-0x0044B33B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GFameWin@@WEEE@AEPAXI@Z at 0x0044b330 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object FameWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned FameWin constructor at 0x0044b2db stores Buffer-view vftable 0x0066c260 at this+0x444; the source owner is the typed FameWin GraphicWin/Buffer multiple-inheritance view.", "Current src/famewin.cpp COFF Buffer-view vftable first slot selects generated ??_EFameWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current FameWin COFF weak external ??_EFameWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GFameWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls FameWin::~FameWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_fame_win@@YIPAXPAX0I@Z and ?scalar_delete_fame_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for FameWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EFameWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x0044B300 ??_GFameWin@@UAEPAXI@Z 0x0044B300-0x0044B321 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0044B300-0x0044B321", "Pinned class constructor ??0FameWin@@QAE@XZ at 0x0044B200 stores original class/view table VA 0x66c268 at 0x0044B2D5 (mov dword ptr [esi], 0x66c268); the current genuine owner vftable ??_7FameWin@@6BWin@@@ selects generated ??_GFameWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EFameWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GFameWin@@UAEPAXI@Z at 0x0044B300 calls the source-mapped ??1FameWin@@QAE@XZ at 0x0044B100, then tests byte ptr [ebp + 8], 1; je 0x44b31a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/famewin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original FameWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GFameWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004483D0 _$E21 0x004483D0-0x004483E6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820ac selects initializer0x4483d0..0x4483e6, registering exit0x4483f0..0x4483fa. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74daf8 bound to?g_FAMEWIN@@3VFameWin@@A.", "Original startup instruction evidence: [[\"0x4483d0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483d5\", \"call\", \"0x44b200\"], [\"0x4483da\", \"push\", \"0x4483f0\"], [\"0x4483df\", \"call\", \"0x645398\"], [\"0x4483e4\", \"pop\", \"ecx\"], [\"0x4483e5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4483f0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483f5\", \"jmp\", \"0x44b100\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/famewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??0FameWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??1FameWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f248; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_FAMEWIN@@3VFameWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// ORIGINAL: 0x004483F0 _$E19 0x004483F0-0x004483FA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820ac selects initializer0x4483d0..0x4483e6, registering exit0x4483f0..0x4483fa. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74daf8 bound to?g_FAMEWIN@@3VFameWin@@A.", "Original startup instruction evidence: [[\"0x4483d0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483d5\", \"call\", \"0x44b200\"], [\"0x4483da\", \"push\", \"0x4483f0\"], [\"0x4483df\", \"call\", \"0x645398\"], [\"0x4483e4\", \"pop\", \"ecx\"], [\"0x4483e5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4483f0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483f5\", \"jmp\", \"0x44b100\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/famewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??0FameWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??1FameWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f248; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_FAMEWIN@@3VFameWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
#include "stdafx.h"
#include "famewin.h"
#include "console.h"
#include "halloffame.h"
#include "alpha.h"
#include "faction.h"
#include "general.h"
#include "netdaemon.h"
#include "game.h"
#include "base.h"
#include "map.h"
#include "strings.h"
#include "worldwin.h"
#include "basewin.h"
#include "reportif.h"
#include "tutwin.h"

// The two original call sites pass distinct, zero-filled BSS addresses as
// window captions. No writer or external bounded extent is known; retain
// separate source-owned empty buffers rather than aliasing executable data.
static char FameWinStandardCaption[4];
static char FameWinWideCaption[4];

// ORIGINAL: 0x0044AF70 sub_44af70 0x0044AF70-0x0044AF8D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native44AE50 reads/writes hallfame.jhh with rb/wb and mode759194;35records of8C bytes begin9A3840;44AF70 clears only their present bytes.", "Native44AEE6 owned ButtonGroup receiver753468 is FameWin74DAF8+5970;44AF04 virtual slot57 is Win::set_modal with callbackpop_wait5398E0.", "Root-only Console retirement dependency closure authorized in task history; retain git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0044ae50.cpp and git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0044af70.cpp seed interpretations.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0044af70.cpp"], "reason": "Recover real FameWin entry and record-presence cleanup through existing typed HallOfFame records and owned button group, standard CRT I/O and inherited virtual modal API. Retain historical exact seed obligations and original spans; current binding grants no match or acceptance claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?clear_hall_of_fame@@YAXXZ
void __cdecl clear_hall_of_fame() {
  for (int record = 0; record < HallOfFameRecordCount; ++record) {
    HallOfFameRecords[record].present = 0;
  }
}

// ORIGINAL: 0x0044AE50 ?hall_of_fame@@YAXH@Z 0x0044AE50-0x0044AF6D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native44AE50 reads/writes hallfame.jhh with rb/wb and mode759194;35records of8C bytes begin9A3840;44AF70 clears only their present bytes.", "Native44AEE6 owned ButtonGroup receiver753468 is FameWin74DAF8+5970;44AF04 virtual slot57 is Win::set_modal with callbackpop_wait5398E0.", "Root-only Console retirement dependency closure authorized in task history; retain git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0044ae50.cpp and git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/0044af70.cpp seed interpretations.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0044ae50.cpp"], "reason": "Recover real FameWin entry and record-presence cleanup through existing typed HallOfFame records and owned button group, standard CRT I/O and inherited virtual modal API. Retain historical exact seed obligations and original spans; current binding grants no match or acceptance claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl hall_of_fame(int add_current_player) {
  FILE *file = fopen("hallfame.jhh", "rb");
  if (file == 0) {
    clear_hall_of_fame();
  } else {
    HallOfFameIOMode = 0;
    if (fread(HallOfFameRecords, sizeof(HallOfFameRecords), 1, file) == 0) {
      clear_hall_of_fame();
    }
    fclose(file);
  }

  if (g_FAMEWIN.init() != 0) return;
  if (add_current_player != 0) g_FAMEWIN.try_to_add_me();
  g_FAMEWIN.buttonGroup_.set(ConsoleGlobal.faction_id_, 1);
  g_FAMEWIN.set_modal(0, pop_wait, 0);

  file = fopen("hallfame.jhh", "wb");
  if (file != 0) {
    HallOfFameIOMode = 1;
    if (fwrite(HallOfFameRecords, sizeof(HallOfFameRecords), 1, file) == 0) {
      clear_hall_of_fame();
    }
    fclose(file);
  }
}

/*
Purpose: Hall-of-fame I/O: store the read/write mode, then transfer the
         0x1324-byte record block out in one shot; on a failed write,
         zero the five 0x8C-stride fields of every record and return.

// ORIGINAL: 0x0044AF90 sub_44af90 0x0044AF90-0x0044AFF2 FILE BYTE_EXACT
// LEVER: write-branch-first if/else polarity (matches the original's `je` target) plus a signed (int) pointer-vs-address comparison instead of unsigned, in the zero-fill loop's exit test
// size      98 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064603F 0x00646178
Status: Structural migration to source-owned HallOfFameRecords and
        HallOfFameIOMode. The historical byte claim remains an obligation;
        this current spelling measures MISMATCH. The image's 0x0064603F and
        0x00646178 are CRT fwrite/fread wrappers.
*/
extern "C" int __cdecl sub_44af90(FILE *a1, int a2) {
    HallOfFameIOMode = a2;
    int result;
    if (a2 != 0) {
        result = static_cast<int>(
            fwrite(HallOfFameRecords, sizeof(HallOfFameRecords), 1, a1));
    } else {
        result = static_cast<int>(
            fread(HallOfFameRecords, sizeof(HallOfFameRecords), 1, a1));
    }
    if (result == 0) {
        for (int i = 0; i < HallOfFameRecordCount; ++i) {
            HallOfFameRecords[i].present = 0;
        }
    }
    return 0;
}

/*
Purpose: Hall-of-fame save: open hallfame.jhh for writing, set the I/O mode,
         write the whole record block, clear entry-present flags on failure,
         close, and report the open failure.

// ORIGINAL: 0x0044B000 ?hof_save@@YAHXZ 0x0044B000-0x0044B06C FILE BYTE_EXACT
// size      108 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598 0x00645646 0x0064603F
Status: Structural migration to source-owned hall-of-fame storage. The
        historical byte claim remains an obligation; current spelling
        measures MISMATCH. The image's fopen is 0x00645646 and fclose is
        0x00645598; the pinned .data bytes at 0x00684610 spell
        "hallfame.jhh", correcting the previous product filename.
*/
int __cdecl hof_save() {
    FILE *file = fopen("hallfame.jhh", "wb");
    if (file == 0) {
        return 1;
    }
    HallOfFameIOMode = 1;
    int written = static_cast<int>(
        fwrite(HallOfFameRecords, sizeof(HallOfFameRecords), 1, file));
    if (written == 0) {
        for (int i = 0; i < HallOfFameRecordCount; ++i) {
            HallOfFameRecords[i].present = 0;
        }
    }
    fclose(file);
    return 0;
}

/*
Purpose: Hall-of-fame load: open hallfame.jhh for reading, clear entry-present
         flags and fail when the open misses, set the I/O mode, read the whole
         record block (clearing flags on a short read), then close.

// ORIGINAL: 0x0044B070 ?hof_load@@YAHXZ 0x0044B070-0x0044B0F8 FILE BYTE_EXACT
// size      136 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598 0x00645646 0x00646178
Status: Structural migration to source-owned hall-of-fame storage. The
        historical byte claim remains an obligation; current spelling
        measures MISMATCH. The pinned .data bytes at 0x00684620/24 are
        "rb"/"hallfame.jhh".
*/
int __cdecl hof_load() {
    FILE *f = fopen("hallfame.jhh", "rb");
    if (f == 0) {
        for (int i = 0; i < HallOfFameRecordCount; ++i) {
            HallOfFameRecords[i].present = 0;
        }
        return 1;
    }
    HallOfFameIOMode = 0;
    int n = static_cast<int>(
        fread(HallOfFameRecords, sizeof(HallOfFameRecords), 1, f));
    if (n == 0) {
        for (int i = 0; i < HallOfFameRecordCount; ++i) {
            HallOfFameRecords[i].present = 0;
        }
    }
    fclose(f);
    return 0;
}


// hall_of_fame's receiver at 0x0074DAF8 and the original g_FAMEWIN
// initializer/finalizer at 0x004483D0/0x004483F0 identify this as a
// process-lifetime object. Its end is the adjacent HallOfFameIOMode slot.
// RECOVERY_OBJECT: v2 0x0074DAF8 0x00759194 ?g_FAMEWIN@@3VFameWin@@A 0x004483D0
FameWin g_FAMEWIN;

// C++ member lifetime follows the constructor and destructor traces: the
// GraphicWin base is first/last; the typed members follow their declaration
// order. This body is intentionally empty so the compiler owns that work.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044B200 ??0FameWin@@QAE@XZ 0x0044B200-0x0044B2F3;0x00654E00-0x00654E8E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044b200.cpp"], "reason": "Typed C++ member lifetime follows the original constructor call sequence; the seed remains the historical body interpretation."}
FameWin::FameWin() {}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044B100 ??1FameWin@@QAE@XZ 0x0044B100-0x0044B1FF;0x00654D60-0x00654DF6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044b100.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ??1FameWin@@UAE@XZ
// Member destructors expand here in reverse declaration order. In particular,
// FlatButton's inline destructor performs close() before its BaseButton base
// teardown, and ButtonGroup's destructor performs its own close().
FameWin::~FameWin() {}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00448D60 ?close@FameWin@@QAEXXZ 0x00448D60-0x00448D81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/00448d60.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
void FameWin::close() {
  buffer1_.close();
  release_modal();
  GraphicWin::close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044A4B0 ?on_key_click@FameWin@@QAEHHH@Z 0x0044A4B0-0x0044A4F3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044a4b0.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_key_click@FameWin@@UAEHHH@Z
int FameWin::on_key_click(int, int code) {
  if ((code > 0x2F && code < 0x5B) || code == 0x1B || code == 0xD) {
    buffer1_.close();
    release_modal();
    GraphicWin::close();
  }
  return 1;
}

// ORIGINAL: 0x00448400 ?init@FameWin@@QAEHXZ 0x00448400-0x00448D60;0x00654E90-0x00654EA5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00448400.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
// TRIED: full transcription landed (MISMATCH #1, no SEH frame reproduced). Used a plain local `Buffer localBuf;` since ~Buffer() is declared here (unlike the sibling StringStruct cases) - RAII reproduces the real destructor call on every return path, matching the disassembly exactly. Did not use the now-retired hypothesis umbrella's FameWin field names per the brief's warning; every FameWin/BaseButton field access is a raw self+offset cast, cross-checked against the raw bytes (not Ghidra) for both BaseButton::init 8-arg calls.
int FameWin::init() {
  Buffer icon_sheet;
  field_A1C_ = 800;
  field_A20_ = 600;
  spot_.init(0x40);

  const int monitor_count = ReportIfGlobal.field_1E230_;
  field_A48_ = static_cast<uint32_t>(monitor_count);
  int x, y, width, height;
  char *caption;
  if (monitor_count == 0) {
    field_A14_ = field_A18_ = field_A24_ = field_A28_ = 0;
    field_A2C_ = field_A1C_;
    field_A30_ = field_A20_;
    caption = FameWinStandardCaption;
    x = y = 0;
    width = 800;
    height = 600;
  } else {
    field_A14_ = field_A24_ = 0x77;
    field_A18_ = field_A28_ = 0x50;
    field_A2C_ = field_A1C_ + 0x77;
    field_A30_ = field_A20_ + 0x50;
    caption = FameWinWideCaption;
    x = (WinScreenWidth - 0x400) / 2;
    y = WinScreenHeight - 0x300;
    width = 0x400;
    height = 0x300;
  }
  GraphicWin::init(x, y, width, height, caption, 0x1000000, 0, 0, 0);

  field_A44_ = static_cast<uint32_t>(ReportIfGlobal.field_1E190_->line_advance());
  int result = 1;
  if (buffer1_.load_pcx("frame.pcx", 0, 0xA, 0xEC) == 0 &&
      buffer2_.load_pcx("hof.pcx", 0, 0xA, 0xEC) == 0) {
    field_B42C_ = 0x1E;  field_B430_ = 0x10E;
    field_B434_ = 0x123; field_B438_ = 0x147;
    field_B43C_ = 0x12F; field_B440_ = 0x10D;
    field_B444_ = 0x21D; field_B448_ = 0x183;
    field_B44C_ = 0x228; field_B450_ = 0x10D;
    field_B454_ = 0x2FA; field_B458_ = 0x21F;
    field_B45C_ = 0x15;  field_B460_ = 0x7B;
    field_B464_ = 0x303; field_B468_ = 0xED;
    field_B46C_ = 0x4C;  field_B470_ = 0x27;
    field_B474_ = 0x2D4; field_B478_ = 0x77;
    field_B47C_ = 0x128; field_B480_ = 0x187;
    field_B484_ = 0x224; field_B488_ = 0x248;

    field_B42C_ += field_A24_; field_B434_ += field_A24_;
    field_B430_ += field_A28_; field_B438_ += field_A28_;
    field_B43C_ += field_A24_; field_B444_ += field_A24_;
    field_B440_ += field_A28_; field_B448_ += field_A28_;
    field_B44C_ += field_A24_; field_B454_ += field_A24_;
    field_B450_ += field_A28_; field_B458_ += field_A28_;
    field_B45C_ += field_A24_; field_B464_ += field_A24_;
    field_B460_ += field_A28_; field_B468_ += field_A28_;
    field_B46C_ += field_A24_; field_B474_ += field_A24_;
    field_B470_ += field_A28_; field_B478_ += field_A28_;
    field_B47C_ += field_A24_; field_B484_ += field_A24_;
    field_B480_ += field_A28_; field_B488_ += field_A28_;

    if (icon_sheet.load_pcx("hoficons.pcx", 0, 0xA, 0xEC) == 0) {
      for (int column = 0; column < 6; ++column) {
        const int icon_x = column * 0x50 + 1;
        for (int row = 0; row < 2; ++row) {
          sprites_[column * 2 + row].extract(
              &icon_sheet, WinFillColour, icon_x, row * 0x50 + 1,
              0x4F, 0x4F, 0);
        }
      }

      const int cell_width =
          (static_cast<int>(field_B474_) - static_cast<int>(field_B46C_)) / 6;
      field_A1C_ = static_cast<uint32_t>(cell_width);
      field_A14_ = static_cast<uint32_t>((cell_width - 0x4F) / 2 + field_B46C_);
      field_A18_ = static_cast<uint32_t>(
          ((static_cast<int>(field_B478_) - static_cast<int>(field_B470_) - 0x4F) / 2) +
          field_B470_);

      for (int award_button_index = 0; award_button_index < 6;
           ++award_button_index) {
        FlatButton &button = flatButtons2_[award_button_index];
        const int label_id = 974 + award_button_index;
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(label_id));
        button.init(0, award_button_index + 0x8AE,
                    award_button_index * static_cast<int>(field_A1C_) + field_A14_,
                    field_A18_, 0x4F, 0x4F, this, 0);
          button.set_report_icons(&sprites_[award_button_index * 2 + 1],
                                &sprites_[award_button_index * 2],
                                &sprites_[award_button_index * 2]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw();
        button.redraw();
        button.hide();
      }

      const int cell_height =
          (static_cast<int>(field_B464_) - static_cast<int>(field_B45C_)) / 7;
      field_A1C_ = static_cast<uint32_t>(cell_height);
      field_A14_ = static_cast<uint32_t>((cell_height - 0x46) / 2 + field_B45C_);
      field_A18_ = static_cast<uint32_t>(
          ((static_cast<int>(field_B468_) - static_cast<int>(field_B460_) - 0x46) / 2) +
          field_B460_);
      buttonGroup_.init(0x15, 0);
      selectedRecord_ = static_cast<uint32_t>(-1);

      for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction].title_leader);
        strcat(StringTemp, " ");
        strcat(StringTemp, Players[faction].name_leader);
        strcat(StringTemp, " ");
        strcat(StringTemp, label_get(180));
        strcat(StringTemp, " ");
        strcat(StringTemp, Players[faction].noun_faction);

        const int index = faction - 1;
        const int button_y = index * static_cast<int>(field_A1C_) + field_A14_;
        field_A38_ = field_A18_;
        field_A34_ = static_cast<uint32_t>(button_y);
        field_A3C_ = static_cast<uint32_t>(button_y + 0x46);
        field_A40_ = field_A38_ + 0x46;
        FlatButton &button = flatButtons1_[index];
        button.init(0, faction, field_A34_, field_A38_,
                    field_A3C_ - field_A34_, field_A40_ - field_A38_, this, 0);
        button.set_report_icons(&FactionsArt[faction].report_logo[1],
                                &FactionsArt[faction].report_logo[0],
                                &FactionsArt[faction].report_logo[0]);
        button.set_bubble_text(StringTemp);
        static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
        button.redraw();
        button.redraw();
        button.hide();
        buttonGroup_.add(&button);
      }

      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(665));
      field_A34_ = field_A24_ + 0x22A;
      field_A38_ = field_A28_ + 0x22C;
      field_A3C_ = field_A24_ + 0x2F8;
      field_A40_ = field_A28_ + 0x242;
      flatButton3_.init(StringTemp, 0x7B8, field_A34_, field_A38_,
                        field_A3C_ - field_A34_, field_A40_ - field_A38_, this, 0);
      flatButton3_.set_border_styles(&InterfaceBoxStyles[47],
                                     &InterfaceBoxStyles[46],
                                     &InterfaceBoxStyles[45]);
      static_cast<Buffer *>(&flatButton3_)->set_font(
          &TutWinBaseWindow.font1_, 0, 0, 0);
      flatButton3_.redraw();
      flatButton3_.hide();
      fill(0);
      result = 0;
    }
  }
  return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044A640 ?on_group_clicked@FameWin@@QAEXHHH@Z 0x0044A640-0x0044A655 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0044a640.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_group_clicked@FameWin@@UAEXHHH@Z
void FameWin::on_group_clicked(int, int, int) {
  selectedRecord_ = 0;
  redraw();
}

static void famewin_clear_text() { StringTemp[0] = '\0'; }

static void famewin_append_label(int index) {
  strcat(StringTemp, label_get(index));
}

static void famewin_append_number(int value) {
  char number[32];
  _itoa(value, number, 10);
  strcat(StringTemp, number);
}

static void famewin_write_left(Buffer *canvas, int x, int y) {
  const int length = static_cast<int>(strlen(StringTemp));
  if (length != 0) canvas->write_l(StringTemp, x, y, length);
}

static void famewin_write_right(Buffer *canvas, int x, int y, int width) {
  const int length = static_cast<int>(strlen(StringTemp));
  if (length != 0) canvas->write_right_l(StringTemp, x, y, width, length);
}

static void famewin_write_wrapped(Buffer *canvas, int x, int y, int width) {
  canvas->wrap_cent(StringTemp, x, y, width);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044A570 ?on_button_clicked@FameWin@@QAEXH@Z 0x0044A570-0x0044A640
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0044a570.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_button_clicked@FameWin@@UAEXH@Z
void FameWin::on_button_clicked(int button_id) {
  if (button_id == 0x7B8) {
    buffer1_.close();
    release_modal();
    GraphicWin::close();
    return;
  }
  int award = button_id - 0x8AE;
  if (award < 0 || award > 5) return;
  for (int row = 0; row < 7; ++row) {
    for (int column = 0; column < 5; ++column) {
      const HallOfFameRecord &entry = HallOfFameRecords[row * 5 + column];
      if ((entry.present & 1) && (entry.award_flags & (1 << award))) {
        buttonGroup_.set(row + 1, 1);
        selectedRecord_ = column;
        redraw();
        return;
      }
    }
  }
  redraw();
}

// ORIGINAL: 0x00448D90 ?on_redraw@FameWin@@QAEXXZ 0x00448D90-0x0044A4AD;0x00654EA5-0x00654EBA
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00448d90.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// TRIED: literal register-machine transliteration (pseudo-regs eax..edi, an EBP(n)-mapped scratch buffer for the small ebp-relative locals, a real local `Buffer localBuf` for the Buffer ctor/load_pcx/copy/dtor, and `canvas = (Buffer*)(self+0x444)` for FameWin's embedded canvas Buffer, goto per basic block). VCall::slot001 retyped to take an int (matches its `push 0` call site) - only its signature changed, per the emitter's own note that this is free to set. Repeated write_l/write_right_l/wrap_cent+wrap_height call sites collapsed the register shuffle to the four underlying FameWin fields (self+0xa14/0xa18/0xa1c/0xa44) directly, since the two are provably the same value at the call - verified against several call sites' actual push order before simplifying. One `fill(...)` call (population-graph highlight box) has approximated arguments: its inputs are two text_width() results left on the stack by a deferred-cleanup (no `add esp` between calls) sequence that a later `fill`'s five thiscall args partly reuse, which was not fully reconstructed. Covers the full control-flow graph and call order otherwise (4843 rebuilt vs 5917 original bytes; sim 0.50 best flag set).
// symbol    ?on_redraw@FameWin@@UAEXXZ
void FameWin::on_redraw() {
  Buffer local_buffer;
  Buffer *canvas = static_cast<Buffer *>(this);
  canvas->restore_clip();
  const int faction = static_cast<int>(buttonGroup_.field_84_);
  const int selected_rank = static_cast<int>(selectedRecord_);
  HallOfFameRecord *record =
      &HallOfFameRecords[faction * 5 - 5 + selected_rank];

  if (field_A48_ != 0) {
    buffer1_.copy(canvas, 0, 0, 0, 0, 0x400, 0x300);
  }
  buffer2_.copy(canvas, 0, 0, field_A24_, field_A28_, 0x320, 0x258);
  RECT margins = {static_cast<LONG>(field_A24_), static_cast<LONG>(field_A28_),
                  static_cast<LONG>(field_A2C_), static_cast<LONG>(field_A30_)};
  canvas->set_clip(&margins);
  canvas->set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
  canvas->set_text_color(0xE6, -1, 1, 1);

  for (int award_hide_index = 0; award_hide_index < 6; ++award_hide_index)
    flatButtons2_[award_hide_index].hide();
  for (int faction_hide_index = 0; faction_hide_index < 7;
       ++faction_hide_index)
    flatButtons1_[faction_hide_index].hide();
  flatButton3_.hide();

  if ((record->present & 1) != 0) {
    famewin_clear_text();
    strcat(StringTemp, "hof_");
    famewin_append_number(faction);
    strcat(StringTemp, "_");
    famewin_append_number(selected_rank + 1);
    strcat(StringTemp, ".pcx");
    if (local_buffer.load_pcx(StringTemp, 0, 0, 0xEC) == 0) {
      local_buffer.copy(canvas, 0, 0, field_B43C_, field_B440_,
                        field_B444_ - field_B43C_, field_B448_ - field_B440_);
    }
  }

  if ((record->present & 1) != 0) {
    for (int page_award_index = 0; page_award_index < 6; ++page_award_index) {
      const int bit = 1 << page_award_index;
      flatButtons2_[page_award_index].set_report_page_state(
          (record->award_flags & bit) ? 2 : -1);
      flatButtons2_[page_award_index].redraw();
    }
  } else {
    for (int hidden_award_index = 0; hidden_award_index < 6;
         ++hidden_award_index) {
      flatButtons2_[hidden_award_index].set_report_page_state(-1);
      flatButtons2_[hidden_award_index].redraw();
    }
  }

  if ((record->present & 1) != 0) {
    FactionsArt[faction].diplomacy_landscape.draw(
        canvas, FactionsArt[faction].diplomacy_landscape.transparent_index(),
        field_B47C_ + 9, field_B480_ + 10);
    RECT details = {static_cast<LONG>(field_B44C_),
                    static_cast<LONG>(field_B450_),
                    static_cast<LONG>(field_B454_),
                    static_cast<LONG>(field_B458_)};
    canvas->set_clip(&details);
    field_A14_ = field_B44C_ + 3;
    field_A18_ = field_B450_ + 3;
    field_A1C_ = field_B454_ - field_B44C_ - 6;
    field_A20_ = field_B458_ - field_B450_ - 6;

    const int award_labels[6] = {0xF38 / 4, 0xF3C / 4, 0xF40 / 4,
                                 0xF44 / 4, 0xF48 / 4, 0xF4C / 4};
    for (int wrapped_award_index = 0; wrapped_award_index < 6;
         ++wrapped_award_index) {
      if ((record->award_flags & (1 << wrapped_award_index)) == 0) continue;
      famewin_clear_text();
      famewin_append_label(award_labels[wrapped_award_index]);
      famewin_write_wrapped(canvas, field_A14_, field_A18_, field_A1C_);
      field_A18_ += static_cast<uint32_t>(canvas->wrap_height(
          StringTemp, static_cast<int>(field_A1C_)));
    }

    // The original presents one primary special-record description according
    // to its ordered flag chain, then the record's score fields.
    famewin_clear_text();
    if ((record->present & 4) != 0) {
      strcat(StringTemp, StringTable.get(
          Facility[FAC_ASCENT_TO_TRANSCENDENCE].name));
    } else if ((record->present & 8) != 0) {
      famewin_append_label(0x830 / 4);
      strcat(StringTemp, " ");
      famewin_append_label(0xE10 / 4);
    } else if ((record->present & 0x10) != 0) {
      famewin_append_label(0xE88 / 4);
    } else if ((record->present & 0x20) != 0) {
      famewin_append_label(0xE70 / 4);
    } else if ((record->present & 0x40) != 0) {
      famewin_append_label(0x1014 / 4);
      strcat(StringTemp, " ");
      famewin_append_label(0xE10 / 4);
    }
    if (StringTemp[0] != '\0') {
      CharUpperA(StringTemp);
      famewin_write_wrapped(canvas, field_A14_, field_A18_, field_A1C_);
      field_A18_ += static_cast<uint32_t>(canvas->wrap_height(
          StringTemp, static_cast<int>(field_A1C_)));
    }

    famewin_clear_text();
    famewin_append_label(0x51C / 4);
    CharUpperA(StringTemp);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->field_14);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    famewin_clear_text();
    famewin_append_label(0x130 / 4);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->field_1C);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    famewin_clear_text();
    famewin_append_label(0xE4 / 4);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->leader_two);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    famewin_clear_text();
    famewin_append_label(0xAB4 / 4);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->field_18);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    famewin_clear_text();
    famewin_append_label(0x10A0 / 4);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    strcat(StringTemp, StringTable.get(Difficulty[record->field_20]));
    CharUpperA(StringTemp);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    if (record->field_21 != 0) {
      famewin_clear_text();
      famewin_append_label((record->field_21 == 2 ? 0x908 : 0x904) / 4);
      CharUpperA(StringTemp);
      famewin_write_left(canvas, field_A14_, field_A18_);
      famewin_clear_text();
      if (record->field_21 == 2) strcat(StringTemp, "+25%");
      else strcat(StringTemp, "-25%");
      famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
      field_A18_ += field_A44_;
    }

    if ((record->present & 2) != 0) {
      famewin_clear_text();
      famewin_append_label(0x1044 / 4);
      CharUpperA(StringTemp);
      famewin_write_left(canvas, field_A14_, field_A18_);
      famewin_clear_text();
      strcat(StringTemp, "+100%");
      famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
      field_A18_ += field_A44_;
    }

    famewin_clear_text();
    famewin_append_label(0x6C0 / 4);
    CharUpperA(StringTemp);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->leader_one);
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    famewin_clear_text();
    famewin_append_label(0x1090 / 4);
    CharUpperA(StringTemp);
    famewin_write_left(canvas, field_A14_, field_A18_);
    famewin_clear_text();
    famewin_append_number(record->rating);
    strcat(StringTemp, "%");
    famewin_write_right(canvas, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    canvas->restore_clip();
    canvas->set_font(ReportIfGlobal.field_1E1B4_, 0, 0, 0);
    canvas->set_text_color(0xF4, -1, 1, 1);
    for (int banner_award_index = 0; banner_award_index < 6;
         ++banner_award_index) {
      if ((record->award_flags & (1 << banner_award_index)) == 0) continue;
      famewin_clear_text();
      famewin_append_label(award_labels[banner_award_index]);
      const int fill_extent = canvas->text_width(StringTemp) + 0x14;
      const int fill_y = static_cast<int>(field_A28_) + 0x0F;
      const int text_width = canvas->text_width(StringTemp);
      const int fill_x =
          static_cast<int>(field_A24_) - text_width / 2 + 0x186;
      fill(fill_x, fill_y, fill_extent, 0x15, 0x41);

      const int text_y = static_cast<int>(field_A28_) + 0x10;
      const int text_x = static_cast<int>(field_A24_);
      const int text_length = static_cast<int>(strlen(StringTemp));
      canvas->write_cent_l(StringTemp, text_x, text_y, 0x320, text_length);
      break;
    }
    canvas->set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
    canvas->set_text_color(0xE6, -1, 1, 1);
  }

  canvas->set_font(ReportIfGlobal.field_1E19C_, 0, 0, 0);
  canvas->set_text_color(0xF4, -1, 1, 1);
  famewin_clear_text();
  famewin_append_label(0x109C / 4);
  canvas->write_cent_l(StringTemp, field_A24_ + 0x1C,
                       field_A28_ + 0xF7, 0x109,
                       static_cast<int>(strlen(StringTemp)));
  famewin_clear_text();
  famewin_append_label(0x4C8 / 4);
  CharUpperA(StringTemp);
  canvas->write_cent_l(StringTemp, field_A24_ + 0x12E,
                       field_A28_ + 0xF7, 0xF0,
                       static_cast<int>(strlen(StringTemp)));
  famewin_clear_text();
  famewin_append_label(0xF10 / 4);
  canvas->write_cent_l(StringTemp, field_A24_ + 0x227,
                       field_A28_ + 0xF7, 0xD4,
                       static_cast<int>(strlen(StringTemp)));

  canvas->set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
  canvas->set_text_color(0xE6, -1, 1, 1);
  spot_.kill_type(0);
  RECT row = {static_cast<LONG>(field_B42C_),
              static_cast<LONG>(field_B430_),
              static_cast<LONG>(field_B434_),
              static_cast<LONG>(field_B438_)};
  for (int rank = 0; rank < 5; ++rank) {
      HallOfFameRecord &entry = HallOfFameRecords[faction * 5 - 5 + rank];
      const int x = row.left;
      const int y = row.top + 3;
      const int width = row.right - row.left;
      if ((entry.present & 1) != 0) {
        canvas->set_text_color(rank == selected_rank ? 0x9B : 0xE6, -1, 1, 1);
        famewin_clear_text();
        strcat(StringTemp, entry.leader_title);
        strcat(StringTemp, " ");
        strcat(StringTemp, entry.leader_name);
        canvas->write_cent_l(StringTemp, x, y, width,
                             static_cast<int>(strlen(StringTemp)));
        famewin_clear_text();
        strcat(StringTemp, entry.faction_noun);
        canvas->write_cent_l(StringTemp, x, y + field_A44_, width,
                             static_cast<int>(strlen(StringTemp)));
        famewin_clear_text();
        famewin_append_number(entry.rating);
        strcat(StringTemp, "  ");
        famewin_append_label(0xB68 / 4);
        canvas->write_cent_l(StringTemp, x, y + 2 * field_A44_, width,
                             static_cast<int>(strlen(StringTemp)));
        famewin_clear_text();
        famewin_append_number(entry.year);
        canvas->write_cent_l(StringTemp, x, y + 3 * field_A44_, width,
                             static_cast<int>(strlen(StringTemp)));
      }
      if ((entry.present & 1) != 0) {
        spot_.add(rank, 0, row.left, row.top, width, row.bottom - row.top);
      }
      const int row_step = static_cast<int>(field_B438_ - field_B430_) + 6;
      row.top += row_step;
      row.bottom += row_step;
  }
}

// ORIGINAL: 0x0044A500 ?on_left_click@FameWin@@QAEXHH@Z 0x0044A500-0x0044A563 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/0044a500.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void FameWin::on_left_click(int x, int y) {
  if ((NetDaemonNet.lock_state_[0].trailing_words_[7] &
       (1u << ConsoleGlobal.faction_id_)) == 0 &&
      NetDaemonNet.lock_state_[2].payload_.flags == 0 &&
      NetDaemonNet.lock_state_[0].leading_words_[3] == 0) {
    if (spot_.check(x, y, &y, &x) != -1) selectedRecord_ = y;
    redraw();
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044AC10 ?demote@FameWin@@QAEXH@Z 0x0044AC10-0x0044AE42;0x00654EBA-0x00654ECF FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0044ac10.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
void FameWin::demote(int rank) {
  Buffer buffer;
  if (rank >= 4) return;

  char number[16];
  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  _itoa(static_cast<int>(factionID_), number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, "_");
  _itoa(rank + 1, number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, ".pcx");
  buffer.load_pcx(StringTemp, 0, 0, 0xEC);

  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  _itoa(static_cast<int>(factionID_), number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, "_");
  _itoa(rank + 2, number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, ".pcx");
  buffer.write_pcx(StringTemp);

  const int old_index = static_cast<int>(factionID_) * 5 - 5 + rank;
  const HallOfFameRecord &source = HallOfFameRecords[old_index];
  HallOfFameRecord &destination = HallOfFameRecords[old_index + 1];
  destination.present = source.present;
  destination.award_flags = source.award_flags;
  destination.year = source.year;
  destination.leader_one = source.leader_one;
  destination.leader_two = source.leader_two;
  destination.rating = source.rating;
  destination.field_14 = source.field_14;
  destination.field_18 = source.field_18;
  destination.field_1C = source.field_1C;
  destination.field_20 = source.field_20;
  destination.field_21 = source.field_21;
  strncpy(destination.leader_title, source.leader_title,
          sizeof(destination.leader_title));
  strncpy(destination.leader_name, source.leader_name,
          sizeof(destination.leader_name));
  strncpy(destination.faction_noun, source.faction_noun,
          sizeof(destination.faction_noun));
  memcpy(destination.unknown_6A + 2, source.unknown_6A + 2, 0x20);
}

// ORIGINAL: 0x0044A660 ?try_to_add_me@FameWin@@QAEXXZ 0x0044A660-0x0044AC05 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0044a660.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
// TRIED: MISMATCH #14, register-allocation-level divergence in the compute_score call setup; full body (score/rank tables, demote chain, 3 tie-break loops, name copies, pcx message) transcribed from the disassembly with byte-offset field access.
void FameWin::try_to_add_me() {
  if ((GameDataStateStorage.fields_.game_preferences_ & 0x20) != 0) return;

  const int player_id = ConsoleGlobal.faction_id_;
  factionID_ = static_cast<uint32_t>(player_id);
  compute_score(player_id, nullptr, nullptr, 0);

  const int leader_score = PlayersData[player_id].unk_107;
  const int rating = get_rating(player_id, leader_score);
  int rank = 0x11A;
  for (int candidate = 4; candidate >= 0; --candidate) {
    const HallOfFameRecord &entry =
        HallOfFameRecords[player_id * 5 - 5 + candidate];
    if (rating >= entry.rating || (entry.present & 1) == 0) rank = candidate;
  }
  if (rank > 4) return;

  if (rank < 4) demote(3);
  if (rank < 3) demote(2);
  if (rank < 2) demote(1);
  if (rank < 1) demote(0);

  selectedRecord_ = static_cast<uint32_t>(rank);
  HallOfFameRecord &record = HallOfFameRecords[player_id * 5 - 5 + rank];
  PlayerData &player_data = PlayersData[player_id];
  const Player &player = Players[player_id];
  record.rating = rating;
  record.leader_one = leader_score;
  record.leader_two = player_data.energy_reserves;
  record.award_flags = 0;
  record.present = 1;
  record.year = game_year(GameDataStateStorage.fields_.turn_current_num_);
  record.field_1C = player_data.score_items_[3] + player_data.score_items_[2];
  record.field_14 = player_data.score_items_[4];
  record.field_18 = player_data.score_items_[0];
  record.field_21 = player_data.score_items_[5] == 0
                        ? 0
                        : (MapNativeLifeForms == 0 ? 1 : 2);

  parse_set(player.is_leader_female, FALSE);
  strncpy(record.leader_title, player.title_leader,
          sizeof(record.leader_title));
  parse_set(player.is_leader_female, FALSE);
  strncpy(record.leader_name, player.name_leader,
          sizeof(record.leader_name));
  parse_set(player.noun_gender, player.is_noun_plural);
  strncpy(record.faction_noun, player.noun_faction,
          sizeof(record.faction_noun));

  record.field_20 = static_cast<uint8_t>(player_data.diff_level);
  if ((GameDataStateStorage.fields_.game_rules_ & 0x400) != 0) record.present |= 2;
  if ((GameDataStateStorage.fields_.game_preferences_ & 0x2000) != 0) {
    record.present |= 8;
  } else if ((GameDataStateStorage.fields_.game_preferences_ & 0x400000) != 0) {
    record.present |= 0x10;
  } else if ((GameDataStateStorage.fields_.game_preferences_ & 0x200000) != 0) {
    record.present |= 0x20;
  } else if (player_data.unk_109 != 0) {
    record.present |= 4;
  } else if ((GameDataStateStorage.fields_.game_preferences_ & 0x10) != 0) {
    record.present |= 0x40;
  }

  int valid_count = 0;
  for (int orbital_faction_index = 0; orbital_faction_index < 7;
       ++orbital_faction_index) {
    for (int orbital_rank_index = 0; orbital_rank_index < 5;
         ++orbital_rank_index) {
      if (orbital_faction_index == player_id - 1 && orbital_rank_index == rank) continue;
      HallOfFameRecord &other = HallOfFameRecords[orbital_faction_index * 5 + orbital_rank_index];
      if ((other.present & 1) == 0) continue;
      ++valid_count;
      if ((other.award_flags & 1) && record.rating >= other.rating) {
        record.award_flags |= 1;
        other.award_flags &= static_cast<uint8_t>(~1u);
      }
      if ((other.award_flags & 2) && record.field_1C >= other.field_1C) {
        record.award_flags |= 2;
        other.award_flags &= static_cast<uint8_t>(~2u);
      }
      if ((other.award_flags & 4) && record.leader_two >= other.leader_two) {
        record.award_flags |= 4;
        other.award_flags &= static_cast<uint8_t>(~4u);
      }
      if ((other.award_flags & 8) && record.field_18 >= other.field_18) {
        record.award_flags |= 8;
        other.award_flags &= static_cast<uint8_t>(~8u);
      }
    }
  }
  if (valid_count == 0) record.award_flags = 0x0F;

  int orbital_records = 0;
  int artifact_records = 0;
  for (int award_faction_index = 0; award_faction_index < 7;
       ++award_faction_index) {
    for (int award_rank_index = 0; award_rank_index < 5;
         ++award_rank_index) {
      if (award_faction_index == player_id - 1 && award_rank_index == rank) continue;
      HallOfFameRecord &other = HallOfFameRecords[award_faction_index * 5 + award_rank_index];
      if ((other.present & 1) == 0) continue;
      if ((other.present & 8) != 0) {
        ++orbital_records;
        if ((other.award_flags & 0x10) && (record.present & 8) &&
            record.year <= other.year) {
          record.award_flags |= 0x10;
          other.award_flags &= static_cast<uint8_t>(~0x10u);
        }
      }
      if ((other.present & 4) != 0) {
        ++artifact_records;
        if ((other.award_flags & 0x20) && (record.present & 4) &&
            record.year <= other.year) {
          record.award_flags |= 0x20;
          other.award_flags &= static_cast<uint8_t>(~0x20u);
        }
      }
    }
  }
  if (orbital_records == 0 && (record.present & 8) != 0)
    record.award_flags |= 0x10;
  if (artifact_records == 0 && (record.present & 4) != 0)
    record.award_flags |= 0x20;

  char number[32];
  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  _itoa(player_id, number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, "_");
  _itoa(rank + 1, number, 10);
  strcat(StringTemp, number);
  strcat(StringTemp, ".pcx");
  static_cast<Buffer *>(&WorldClimateWorldWin)->write_pcx(StringTemp);
}
