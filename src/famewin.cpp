// ORIGINAL: 0x0044B330 ??_GFameWin@@WEEE@AEPAXI@Z 0x0044B330-0x0044B33B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GFameWin@@WEEE@AEPAXI@Z at 0x0044b330 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object FameWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned FameWin constructor at 0x0044b2db stores Buffer-view vftable 0x0066c260 at this+0x444; the source owner is the typed FameWin GraphicWin/Buffer multiple-inheritance view.", "Current src/famewin.cpp COFF Buffer-view vftable first slot selects generated ??_EFameWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current FameWin COFF weak external ??_EFameWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GFameWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls FameWin::~FameWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_fame_win@@YIPAXPAX0I@Z and ?scalar_delete_fame_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for FameWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EFameWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x0044B300 ??_GFameWin@@UAEPAXI@Z 0x0044B300-0x0044B321 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0044B300-0x0044B321", "Pinned class constructor ??0FameWin@@QAE@XZ at 0x0044B200 stores original class/view table VA 0x66c268 at 0x0044B2D5 (mov dword ptr [esi], 0x66c268); the current genuine owner vftable ??_7FameWin@@6BWin@@@ selects generated ??_GFameWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EFameWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GFameWin@@UAEPAXI@Z at 0x0044B300 calls the source-mapped ??1FameWin@@QAE@XZ at 0x0044B100, then tests byte ptr [ebp + 8], 1; je 0x44b31a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/famewin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original FameWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GFameWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004483D0 _$E21 0x004483D0-0x004483E6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820ac selects initializer0x4483d0..0x4483e6, registering exit0x4483f0..0x4483fa. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74daf8 bound to?g_FAMEWIN@@3VFameWin@@A.", "Original startup instruction evidence: [[\"0x4483d0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483d5\", \"call\", \"0x44b200\"], [\"0x4483da\", \"push\", \"0x4483f0\"], [\"0x4483df\", \"call\", \"0x645398\"], [\"0x4483e4\", \"pop\", \"ecx\"], [\"0x4483e5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4483f0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483f5\", \"jmp\", \"0x44b100\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/famewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??0FameWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??1FameWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f248; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_FAMEWIN@@3VFameWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x004483F0 _$E19 0x004483F0-0x004483FA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6820ac selects initializer0x4483d0..0x4483e6, registering exit0x4483f0..0x4483fa. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x74daf8 bound to?g_FAMEWIN@@3VFameWin@@A.", "Original startup instruction evidence: [[\"0x4483d0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483d5\", \"call\", \"0x44b200\"], [\"0x4483da\", \"push\", \"0x4483f0\"], [\"0x4483df\", \"call\", \"0x645398\"], [\"0x4483e4\", \"pop\", \"ecx\"], [\"0x4483e5\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x4483f0\", \"mov\", \"ecx, 0x74daf8\"], [\"0x4483f5\", \"jmp\", \"0x44b100\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/famewin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??0FameWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_FAMEWIN@@3VFameWin@@A\", \"??1FameWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f248; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_FAMEWIN@@3VFameWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
#define WIN_RECT_EXTENT_INLINE  // stand-in: see rect_width in win.h
#define BUFFER_WRITE_RIGHT_L3  // stand-in: see write_right_l in buffer.h
#include "stdafx.h"
#include "famewin.h"
#include "rectangle_operations.h"
#include "font_metrics.h"
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

// The pinned entry and its read/write helpers reuse these two distinct
// filename objects. Their static lifetime and 13-byte payload are retained;
// the original allocation capacities and source names remain unproved.
// RECOVERY_OBJECT: v1 0x00684610 _HallOfFameWriteFilename
static char HallOfFameWriteFilename[] = "hallfame.jhh";

// RECOVERY_OBJECT: v1 0x00684624 _HallOfFameReadFilename
static char HallOfFameReadFilename[] = "hallfame.jhh";

// ORIGINAL: 0x0044AE50 ?hall_of_fame@@YAXH@Z 0x0044AE50-0x0044AF6D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native44AE50 reads/writes hallfame.jhh with rb/wb and mode759194;35records of8C bytes begin9A3840;44AF70 clears only their present bytes.", "Native44AEE6 owned ButtonGroup receiver753468 is FameWin74DAF8+5970;44AF04 virtual slot57 is Win::set_modal with callbackpop_wait5398E0.", "Root-only Console retirement dependency closure authorized in task history; retain git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/0044ae50.cpp and git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/0044af70.cpp seed interpretations.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/0044ae50.cpp"], "reason": "Recover real FameWin entry and record-presence cleanup through existing typed HallOfFame records and owned button group, standard CRT I/O and inherited virtual modal API. Retain historical exact seed obligations and original spans; current binding grants no match or acceptance claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
int __cdecl hof_load();
int __cdecl hof_save();
// WORKAROUND(inline): dead budget probe. The image's hall_of_fame
// (0x0044AE50) expands hof_load and hof_save with their sub_44af90 bodies,
// keeps hof_load's inner clear_hall_of_fame as the only out-of-line call to
// 0x0044AF70 and expands hof_save's; the standalone hof_load/hof_save/
// sub_44af90 copies expand it everywhere. Two zero-code expansions after the
// set_modal call reproduce that split (without them every clear_hall_of_fame
// expands: 10 calls against the image's 11); no natural spelling found yet.
static int hof_probe(int x) {
  if (x == 1) x = 3;
  if (x == 2) x = 5;
  return x;
}
void __cdecl hall_of_fame(int add_current_player) {
  hof_load();
  if (g_FAMEWIN.init() != 0) return;
  if (add_current_player != 0) g_FAMEWIN.try_to_add_me();
  g_FAMEWIN.buttonGroup_.set(ConsoleGlobal.faction_id_, 1);
  Win &modal_window = g_FAMEWIN;
  modal_window.set_modal(0, pop_wait, 0);
  int pr = 0;
  pr += hof_probe(0);
  pr += hof_probe(0);
  if (pr == 7) return;
  hof_save();
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
*/
// The pinned 0x0064603F/0x00646178 targets are CRT fwrite/fread wrappers.
// The retained LEVER describes the seed's integer-address spelling; this
// typed loop expresses the fixed 35-record table as seven groups of five.
// symbol    ?sub_44af90@@YAHPAU_iobuf@@H@Z
int __cdecl sub_44af90(FILE *a1, int a2) {
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
        clear_hall_of_fame();
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
*/
// The pinned fopen/fclose targets are 0x00645646/0x00645598; the data at
// 0x00684610 spells "hallfame.jhh", not the former product filename.
int __cdecl hof_save() {
    // The pinned hall_of_fame and hof_save fopen calls both reference the
    // initial "wb\0" bytes at 0x68460C; this writable-data correspondence does
    // not claim runtime immutability or linker selection.
    // RECOVERY_LITERAL: v1 0x68460C 0x68460F $SG58898 0x44AE50
    FILE *file = fopen(HallOfFameWriteFilename, "wb");
    if (file == 0) {
        return 1;
    }
    sub_44af90(file, 1);
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
*/
// Pinned data at 0x00684620/0x00684624 is "rb"/"hallfame.jhh".
int __cdecl hof_load() {
    // The pinned hall_of_fame and hof_load fopen calls both reference the
    // initial "rb\0" bytes at 0x684620; this writable-data correspondence does
    // not claim runtime immutability or linker selection.
    // RECOVERY_LITERAL: v1 0x684620 0x684623 $SG58903 0x44AE50
    FILE *f = fopen(HallOfFameReadFilename, "rb");
    if (f == 0) {
        clear_hall_of_fame();
        return 1;
    }
    sub_44af90(f, 0);
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
// ORIGINAL: 0x0044B100 ??1FameWin@@QAE@XZ 0x0044B100-0x0044B1FF;0x00654D60-0x00654DF6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0044b100.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// kind game
// symbol    ??1FameWin@@UAE@XZ
// Member destructors expand here in reverse declaration order. In particular,
// FlatButton's inline destructor performs close() before its BaseButton base
// teardown, and ButtonGroup's destructor performs its own close().
// The implicit virtual destructor follows the typed member/base lifetime.

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
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00448400.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
// TRIED: full transcription landed (MISMATCH #1, no SEH frame reproduced). Used a plain local `Buffer localBuf;` since ~Buffer() is declared here (unlike the sibling StringStruct cases) - RAII reproduces the real destructor call on every return path, matching the disassembly exactly. Did not use the now-retired hypothesis umbrella's FameWin field names per the brief's warning; every FameWin/BaseButton field access is a raw self+offset cast, cross-checked against the raw bytes (not Ghidra) for both BaseButton::init 8-arg calls.
int FameWin::init() {
  Buffer icon_sheet;
  field_A1C_ = 800;
  field_A20_ = 600;
  spot_.init(0x40);

  field_A48_ = ReportIfGlobal.field_1E230_;
  if (field_A48_ == 0) {
    field_A14_ = 0;
    field_A18_ = 0;
    set_rectangle_dimensions(&frame_rect_, 0, 0, field_A1C_, field_A20_);
    GraphicWin::init(0, 0, 800, 600, FameWinStandardCaption, 0x1000000, 0, 0, 0);
  } else {
    field_A14_ = 0x77;
    field_A18_ = 0x50;
    set_rectangle_dimensions(&frame_rect_, 0x77, 0x50, field_A1C_, field_A20_);
    int y = WinScreenHeight - 0x300;
    int x = (WinScreenWidth - 0x400) / 2;
    GraphicWin::init(x, y, 0x400, 0x300, FameWinWideCaption, 0x1000000, 0, 0, 0);
  }

  Font *font = ReportIfGlobal.field_1E190_;
  field_A44_ = font_line_advance(*font);
  if (buffer1_.load_pcx("frame.pcx", 0, 0xA, 0xEC) != 0) goto failed;
  if (buffer2_.load_pcx("hof.pcx", 0, 0xA, 0xEC) != 0) goto failed;

  set_rectangle_dimensions(&ranks_rect_, 0x1E, 0x10E, 0x105, 0x39);
  set_rectangle_dimensions(&portrait_rect_, 0x12F, 0x10D, 0xEE, 0x76);
  set_rectangle_dimensions(&details_rect_, 0x228, 0x10D, 0xD2, 0x112);
  set_rectangle_dimensions(&factions_rect_, 0x15, 0x7B, 0x2EE, 0x72);
  set_rectangle_dimensions(&awards_rect_, 0x4C, 0x27, 0x288, 0x50);
  set_rectangle_dimensions(&landscape_rect_, 0x128, 0x187, 0xFC, 0xC1);

  offset_rect(&ranks_rect_, frame_rect_.left, frame_rect_.top);
  offset_rect(&portrait_rect_, frame_rect_.left, frame_rect_.top);
  offset_rect(&details_rect_, frame_rect_.left, frame_rect_.top);
  offset_rect(&factions_rect_, frame_rect_.left, frame_rect_.top);
  offset_rect(&awards_rect_, frame_rect_.left, frame_rect_.top);
  offset_rect(&landscape_rect_, frame_rect_.left, frame_rect_.top);

  if (icon_sheet.load_pcx("hoficons.pcx", 0, 0xA, 0xEC) != 0) {
failed:
    return 1;
  }

  field_A14_ = 1;
  Sprite *sprite = sprites_;
  for (int column = 0; column < 6; ++column) {
    field_A18_ = 1;
    for (int row = 0; row < 2; ++row) {
      sprite++->extract(&icon_sheet, WinFillColour, field_A14_,
                                       field_A18_, 0x4F, 0x4F, 0);
      field_A18_ += 0x50;
    }
    field_A14_ += 0x50;
  }

  field_A1C_ = rect_width(&awards_rect_) / 6;
  field_A14_ = ((int)field_A1C_ - 0x4F) / 2 + awards_rect_.left;
  field_A18_ = (rect_height(&awards_rect_) - 0x4F) / 2 + awards_rect_.top;

  for (int award = 0; award < 6; ++award) {
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(974 + award));
    flatButtons2_[award].init(0, award + 0x8AE, award * static_cast<int>(field_A1C_) + field_A14_,
                field_A18_, 0x4F, 0x4F, this, 0);
    flatButtons2_[award].set_report_icons(&sprites_[award * 2 + 1], &sprites_[award * 2],
                            &sprites_[award * 2]);
    flatButtons2_[award].set_bubble_text(StringTemp);
    flatButtons2_[award].Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    flatButtons2_[award].redraw();
    flatButtons2_[award].redraw();
    flatButtons2_[award].hide();
  }

  field_A1C_ = rect_width(&factions_rect_) / 7;
  field_A14_ = ((int)field_A1C_ - 0x46) / 2 + factions_rect_.left;
  field_A18_ = (rect_height(&factions_rect_) - 0x46) / 2 + factions_rect_.top;
  buttonGroup_.init(0x15, 0);
  selectedRecord_ = static_cast<uint32_t>(-1);

  for (int i = 0; i < MaxPlayerNum - 1; ++i) {
    const int faction = i + 1;
    StringTemp[0] = '\0';
    parse_set(Players[faction].is_leader_female, FALSE);
    strcat(StringTemp, Players[faction].title_leader);
    strcat(StringTemp, BufferJoinSpace);
    parse_set(Players[faction].is_leader_female, FALSE);
    strcat(StringTemp, Players[faction].name_leader);
    strcat(StringTemp, BufferJoinSpace);
    strcat(StringTemp, label_get(180));
    strcat(StringTemp, BufferJoinSpace);
    parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
    strcat(StringTemp, Players[faction].noun_faction);

    FlatButton &button = flatButtons1_[i];
    set_rectangle_dimensions(&cell_rect_,
                  i * static_cast<int>(field_A1C_) + field_A14_,
                  field_A18_, 0x46, 0x46);
    button.init(0, faction, cell_rect_.left, cell_rect_.top, cell_rect_.right - cell_rect_.left,
                cell_rect_.bottom - cell_rect_.top, this, 0);
    button.set_report_icons(&FactionsArt[faction].report_logo[1],
                            &FactionsArt[faction].report_logo[0],
                            &FactionsArt[faction].report_logo[0]);
    button.set_bubble_text(StringTemp);
    button.Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    button.redraw();
    button.hide();
    buttonGroup_.add(&button);
  }

  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(665));
  set_rectangle_dimensions(&cell_rect_,
                frame_rect_.left + 0x22A, frame_rect_.top + 0x22C, 0xCE, 0x16);
  {
    FlatButton &button = flatButton3_;
    button.init(StringTemp, 0x7B8, cell_rect_.left, cell_rect_.top,
                      cell_rect_.right - cell_rect_.left, cell_rect_.bottom - cell_rect_.top, this, 0);
    button.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46],
                                   &InterfaceBoxStyles[45]);
    button.Buffer::set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
    button.redraw();
    button.hide();
  }
  fill(0);
  return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044A640 ?on_group_clicked@FameWin@@QAEXHHH@Z 0x0044A640-0x0044A655 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/0044a640.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_group_clicked@FameWin@@UAEXHHH@Z
void FameWin::on_group_clicked(int, int, int) {
  selectedRecord_ = 0;
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044A570 ?on_button_clicked@FameWin@@QAEXH@Z 0x0044A570-0x0044A640 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0044a570.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// symbol    ?on_button_clicked@FameWin@@UAEXH@Z
void FameWin::on_button_clicked(int button_id) {
  if (button_id == 0x7B8) {
    buffer1_.close();
    release_modal();
    GraphicWin::close();
    return;
  }
  button_id -= 0x8AE;
  if (button_id < 0 || button_id > 5) return;
  for (int row = 0; row < 7; ++row) {
    for (int column = 0; column < 5; ++column) {
      const HallOfFameRecord &entry = HallOfFameRecords[row * 5 + column];
      if ((entry.present & 1) && (entry.award_flags & (1 << button_id))) {
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
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00448d90.cpp"], "reason": "Product definition uses the typed FameWin ownership model while retaining the historical seed and its recovery obligations."}
// TRIED: literal register-machine transliteration (pseudo-regs eax..edi, an EBP(n)-mapped scratch buffer for the small ebp-relative locals, a real local `Buffer localBuf` for the Buffer ctor/load_pcx/copy/dtor, and `canvas = (Buffer*)(self+0x444)` for FameWin's embedded canvas Buffer, goto per basic block). VCall::slot001 retyped to take an int (matches its `push 0` call site) - only its signature changed, per the emitter's own note that this is free to set. Repeated write_l/write_right_l/wrap_cent+wrap_height call sites collapsed the register shuffle to the four underlying FameWin fields (self+0xa14/0xa18/0xa1c/0xa44) directly, since the two are provably the same value at the call - verified against several call sites' actual push order before simplifying. One `fill(...)` call (population-graph highlight box) has approximated arguments: its inputs are two text_width() results left on the stack by a deferred-cleanup (no `add esp` between calls) sequence that a later `fill`'s five thiscall args partly reuse, which was not fully reconstructed. Covers the full control-flow graph and call order otherwise (4843 rebuilt vs 5917 original bytes; sim 0.50 best flag set).
// symbol    ?on_redraw@FameWin@@UAEXXZ
void FameWin::on_redraw() {
  int banner_drawn = 0;
  Buffer portrait;
  restore_clip();
  if (field_A48_ != 0) {
    buffer1_.copy(this, 0, 0, 0, 0, 0x400, 0x300);
  }
  buffer2_.copy(this, 0, 0, frame_rect_.left, frame_rect_.top, 0x320, 0x258);
  set_clip(&frame_rect_);
  set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
  set_text_color(0xE6, -1, 1, 1);

  for (int award_hide = 0; award_hide < 6; ++award_hide) {
    flatButtons2_[award_hide].show(0);
  }
  for (int faction_hide = 0; faction_hide < 7; ++faction_hide) {
    flatButtons1_[faction_hide].show(0);
  }
  flatButton3_.show(0);

  int faction = static_cast<int>(buttonGroup_.field_84_);
  int selected = static_cast<int>(selectedRecord_);
  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  say_num(faction);
  strcat(StringTemp, "_");
  say_num(selected + 1);
  strcat(StringTemp, ".pcx");
  HallOfFameRecord *record = &HallOfFameRecords[faction * 5 - 5 + selected];

  if ((record->present & 1) != 0) {
    if (portrait.load_pcx(StringTemp, 0, 0, 0xEC) == 0) {
      portrait.copy(this, 0, 0, portrait_rect_.left, portrait_rect_.top,
                    rect_width(&portrait_rect_), rect_height(&portrait_rect_));
    }
    for (int award = 0; award < 6; ++award) {
      if ((record->award_flags & (1 << award)) != 0) {
        flatButtons2_[award].set_report_page_state(2);
      } else {
        flatButtons2_[award].set_report_page_state(-1);
      }
      flatButtons2_[award].redraw();
    }

    const int landscape_y = landscape_rect_.top + 10;
    const int landscape_x = landscape_rect_.left + 9;
    FactionsArt[faction].diplomacy_landscape.draw(
        this, landscape_x, landscape_y);
    RECT *details = &details_rect_;
    set_clip(details);
    field_A14_ = details->left + 3;
    field_A18_ = details->top + 3;
    field_A1C_ = details->right - details->left - 6;
    field_A20_ = details->bottom - details->top - 6;

    if ((record->award_flags & 1) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(974));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }
    if ((record->award_flags & 2) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(975));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }
    if ((record->award_flags & 4) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(976));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }
    if ((record->award_flags & 8) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(977));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }
    if ((record->award_flags & 0x10) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(978));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }
    if ((record->award_flags & 0x20) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(979));
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }

    if ((record->present & 4) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp,
             StringTable.get(Facility[FAC_ASCENT_TO_TRANSCENDENCE].name));
      CharUpperA(StringTemp);
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    } else if ((record->present & 8) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(524));
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, label_get(900));
      CharUpperA(StringTemp);
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    } else if ((record->present & 0x10) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(930));
      CharUpperA(StringTemp);
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    } else if ((record->present & 0x20) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(924));
      CharUpperA(StringTemp);
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    } else if ((record->present & 0x40) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(1029));
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, label_get(900));
      CharUpperA(StringTemp);
      wrap_cent(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += wrap_height(StringTemp, field_A1C_);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(327));
    CharUpperA(StringTemp);
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->field_14);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Labels.strings_ptr[76]));
    CharUpperA(StringTemp);
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->field_1C);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(57));
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->leader_two);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(685));
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->field_18);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(1064));
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(Difficulty[record->field_20]));
    CharUpperA(StringTemp);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    if (record->field_21 != 0) {
      StringTemp[0] = '\0';
      if (!(record->field_21 == 2)) {
        strcat(StringTemp, label_get(577));
        CharUpperA(StringTemp);
        write_l(StringTemp, field_A14_, field_A18_);
        StringTemp[0] = '\0';
        strcat(StringTemp, "-25%");
        write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
      } else {
        strcat(StringTemp, label_get(578));
        CharUpperA(StringTemp);
        write_l(StringTemp, field_A14_, field_A18_);
        StringTemp[0] = '\0';
        strcat(StringTemp, "+25%");
        write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
      }
      field_A18_ += field_A44_;
    }

    if ((record->present & 2) != 0) {
      StringTemp[0] = '\0';
      strcat(StringTemp, label_get(1041));
      CharUpperA(StringTemp);
      write_l(StringTemp, field_A14_, field_A18_);
      StringTemp[0] = '\0';
      strcat(StringTemp, "+100%");
      write_right_l(StringTemp, field_A14_ + field_A1C_, field_A18_);
      field_A18_ += field_A44_;
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(432));
    CharUpperA(StringTemp);
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->leader_one);
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(1060));
    CharUpperA(StringTemp);
    write_l(StringTemp, field_A14_, field_A18_);
    StringTemp[0] = '\0';
    say_num(record->rating);
    strcat(StringTemp, "%");
    write_right(StringTemp, field_A14_, field_A18_, field_A1C_);
    field_A18_ += field_A44_;

    restore_clip();
    set_font(ReportIfGlobal.field_1E1B4_, 0, 0, 0);
    set_text_color(0xF4, -1, 1, 1);
    for (int banner = 0; banner < 6; ++banner) {
      if (banner_drawn == 0 && (record->award_flags & (1 << banner)) != 0) {
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(974 + banner));
        fill(frame_rect_.left - text_width(StringTemp) / 2 + 0x186, frame_rect_.top + 0xF,
             text_width(StringTemp) + 0x14, 0x15, 0x41);
        write_cent_text(StringTemp, frame_rect_.left, frame_rect_.top + 0x10, 0x320);
        banner_drawn = 1;
      }
    }
    set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
    set_text_color(0xE6, -1, 1, 1);
  } else {
    for (int award = 0; award < 6; ++award) {
      flatButtons2_[award].set_report_page_state(-1);
      flatButtons2_[award].redraw();
    }
  }

  set_font(ReportIfGlobal.field_1E19C_, 0, 0, 0);
  set_text_color(0xF4, -1, 1, 1);
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(1063));
  write_cent_text(StringTemp, frame_rect_.left + 0x1C, frame_rect_.top + 0xF7, 0x109);
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(306));
  CharUpperA(StringTemp);
  write_cent_text(StringTemp, frame_rect_.left + 0x12E, frame_rect_.top + 0xF7, 0xF0);
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(964));
  write_cent_text(StringTemp, frame_rect_.left + 0x227, frame_rect_.top + 0xF7, 0xD4);

  set_font(ReportIfGlobal.field_1E190_, 0, 0, 0);
  set_text_color(0xE6, -1, 1, 1);
  spot_.kill_type(0);
  RECT *cell = &cell_rect_;
  *cell = ranks_rect_;
  for (int rank = 0; rank < 5; ++rank) {
    HallOfFameRecord *entry = &HallOfFameRecords[faction * 5 - 5 + rank];
    field_A14_ = cell->left;
    field_A18_ = cell->top + 3;
    field_A1C_ = cell->right - cell->left;
    if ((entry->present & 1) != 0) {
      if (rank == static_cast<int>(selectedRecord_)) set_text_color(0x9B, -1, 1, 1);
      else set_text_color(0xE6, -1, 1, 1);
      StringTemp[0] = '\0';
      strcat(StringTemp, entry->leader_title);
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, entry->leader_name);
      write_cent_text(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += field_A44_;
      StringTemp[0] = '\0';
      strcat(StringTemp, entry->faction_noun);
      write_cent_text(StringTemp, field_A14_, field_A18_, field_A1C_);
      field_A18_ += field_A44_;
      StringTemp[0] = '\0';
      say_num(entry->rating);
      strcat(StringTemp, "%");
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, BufferJoinSpace);
      strcat(StringTemp, label_get(730));
      say_num(entry->year);
      write_cent_text(StringTemp, field_A14_, field_A18_, field_A1C_);
      spot_.add(rank, 0, cell->left, cell->top, cell->right - cell->left,
                cell->bottom - cell->top);
    }
    offset_rect(cell, 0, rect_height(&ranks_rect_) + 6);
  }
}

// ORIGINAL: 0x0044A500 ?on_left_click@FameWin@@QAEXHH@Z 0x0044A500-0x0044A563 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/0044a500.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?on_left_click@FameWin@@UAEXHH@Z
// kind      game
void FameWin::on_left_click(int x, int y) {
  if ((NetDaemonNet.primary_state_.trailing_words_[6] &
       (1u << ConsoleGlobal.faction_id_)) == 0 &&
      NetDaemonNet.field_1BB4_ == 0 &&
      NetDaemonNet.primary_state_.leading_words_[2] == 0) {
    // The original increments Spot::check's -1-or-index result and branches
    // on zero; the index range keeps that increment representable.
    if (spot_.check(x, y, &y, &x) + 1 != 0) selectedRecord_ = y;
    redraw();
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0044AC10 ?demote@FameWin@@QAEXH@Z 0x0044AC10-0x0044AE42;0x00654EBA-0x00654ECF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0044ac10.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
void FameWin::demote(int rank) {
  Buffer buffer;
  if (rank >= 4) return;

  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  say_num(factionID_);
  strcat(StringTemp, "_");
  say_num(rank + 1);
  strcat(StringTemp, ".pcx");
  buffer.load_pcx(StringTemp, 0, 0, 0xEC);

  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  say_num(factionID_);
  strcat(StringTemp, "_");
  say_num(rank + 2);
  strcat(StringTemp, ".pcx");
  buffer.write_pcx(StringTemp);

  const HallOfFameRecord &source =
      HallOfFameRecords[static_cast<int>(factionID_) * 5 + rank - 5];
  HallOfFameRecord &destination =
      HallOfFameRecords[static_cast<int>(factionID_) * 5 + rank - 4];
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
  for (int i = 0; i < 8; i++) {
    destination.unknown_6C[i] = source.unknown_6C[i];
  }
}

// ORIGINAL: 0x0044A660 ?try_to_add_me@FameWin@@QAEXXZ 0x0044A660-0x0044AC05 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0044a660.cpp", "Pinned original owner review: original clip resets and absent-record branches, CharUpperA import, difficulty-label capitalization and exact demote tail copying."], "reason": "Recover FameWin owner behavior through existing GraphicWin/Buffer bases, owned buttons and sprites, and typed HallOfFame/Player records; retain historical seed identity and extents without a measurement claim."}
// The image re-reads factionID_ for every lookup and runs the award scans as
// three separate passes sharing one counter. Each name copy reads factionID_
// once into a block-local index (the image recomputes the row per block).
void FameWin::try_to_add_me() {
  // Native 0x44A666/0x44A922 test game_state_ (0x9A64C0) throughout.
  if ((GameDataStateStorage.fields_.game_state_ & 0x20) != 0) return;

  factionID_ = ConsoleGlobal.faction_id_;
  compute_score(factionID_, nullptr, nullptr, 0);

  const int leader_score = PlayersData[factionID_].unk_107;
  const int rating = get_rating(factionID_, leader_score);
  int rank = 0x11A;
  for (int candidate = 4; candidate >= 0; --candidate) {
    const HallOfFameRecord &entry =
        HallOfFameRecords[factionID_ * 5 - 5 + candidate];
    if (rating >= entry.rating || (entry.present & 1) == 0) rank = candidate;
  }
  if (rank > 4) return;

  if (rank < 4) demote(3);
  if (rank < 3) demote(2);
  if (rank < 2) demote(1);
  if (rank < 1) demote(0);

  selectedRecord_ = static_cast<uint32_t>(rank);
  HallOfFameRecord &record = HallOfFameRecords[factionID_ * 5 + rank - 5];
  record.rating = rating;
  record.leader_one = leader_score;
  record.leader_two = PlayersData[factionID_].energy_reserves;
  record.award_flags = 0;
  record.present = 1;
  record.year = game_year(GameDataStateStorage.fields_.turn_current_num_);
  record.field_1C = PlayersData[factionID_].score_items_[2] +
                    PlayersData[factionID_].score_items_[3];
  record.field_14 = PlayersData[factionID_].score_items_[4];
  record.field_18 = PlayersData[factionID_].score_items_[0];
  if (PlayersData[factionID_].score_items_[5] != 0) {
    if (MapNativeLifeForms == 0) {
      record.field_21 = 1;
    } else {
      record.field_21 = 2;
    }
  } else {
    record.field_21 = 0;
  }

  {
    const int faction = factionID_;
    parse_set(Players[faction].is_leader_female, FALSE);
    strncpy(record.leader_title, Players[faction].title_leader,
            sizeof(record.leader_title));
  }
  {
    const int faction = factionID_;
    parse_set(Players[faction].is_leader_female, FALSE);
    strncpy(record.leader_name, Players[faction].name_leader,
            sizeof(record.leader_name));
  }
  {
    const int faction = factionID_;
    parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
    strncpy(record.faction_noun, Players[faction].noun_faction,
            sizeof(record.faction_noun));
  }

  record.field_20 = static_cast<uint8_t>(PlayersData[factionID_].diff_level);
  if ((GameDataStateStorage.fields_.game_rules_ & 0x400) != 0) record.present |= 2;
  if ((GameDataStateStorage.fields_.game_state_ & 0x2000) != 0) {
    record.present |= 8;
  } else if ((GameDataStateStorage.fields_.game_state_ & 0x400000) != 0) {
    record.present |= 0x10;
  } else if ((GameDataStateStorage.fields_.game_state_ & 0x200000) != 0) {
    record.present |= 0x20;
  } else if (PlayersData[factionID_].score_items_[6] != 0) {
    record.present |= 4;
  } else if ((GameDataStateStorage.fields_.game_state_ & 0x10) != 0) {
    record.present |= 0x40;
  }

  // One counter and one faction index serve all three record scans.
  int count = 0;
  int faction_index;
  int rank_index;
  for (faction_index = 0; faction_index < 7; ++faction_index) {
    for (rank_index = 0; rank_index < 5; ++rank_index) {
      if (faction_index == static_cast<int>(factionID_) - 1 && rank_index == rank) continue;
      HallOfFameRecord &other = HallOfFameRecords[faction_index * 5 + rank_index];
      if ((other.present & 1) == 0) continue;
      ++count;
      if ((other.award_flags & 1) && record.rating >= other.rating) {
        record.award_flags |= 1;
        other.award_flags &= ~1;
      }
      if ((other.award_flags & 2) && record.field_1C >= other.field_1C) {
        record.award_flags |= 2;
        other.award_flags &= ~2;
      }
      if ((other.award_flags & 4) && record.leader_two >= other.leader_two) {
        record.award_flags |= 4;
        other.award_flags &= ~4;
      }
      if ((other.award_flags & 8) && record.field_18 >= other.field_18) {
        record.award_flags |= 8;
        other.award_flags &= ~8;
      }
    }
  }
  if (count == 0) record.award_flags = 0x0F;

  count = 0;
  for (faction_index = 0; faction_index < 7; ++faction_index) {
    for (rank_index = 0; rank_index < 5; ++rank_index) {
      if (faction_index == static_cast<int>(factionID_) - 1 && rank_index == rank) continue;
      HallOfFameRecord &other = HallOfFameRecords[faction_index * 5 + rank_index];
      if ((other.present & 1) == 0 || (other.present & 8) == 0) continue;
      ++count;
      if ((other.award_flags & 0x10) && (record.present & 8) &&
          record.year <= other.year) {
        record.award_flags |= 0x10;
        other.award_flags &= ~0x10;
      }
    }
  }
  if (count == 0 && (record.present & 8) != 0) record.award_flags |= 0x10;

  count = 0;
  for (faction_index = 0; faction_index < 7; ++faction_index) {
    for (rank_index = 0; rank_index < 5; ++rank_index) {
      if (faction_index == static_cast<int>(factionID_) - 1 && rank_index == rank) continue;
      HallOfFameRecord &other = HallOfFameRecords[faction_index * 5 + rank_index];
      if ((other.present & 1) == 0 || (other.present & 4) == 0) continue;
      ++count;
      if ((other.award_flags & 0x20) && (record.present & 4) &&
          record.year <= other.year) {
        record.award_flags |= 0x20;
        other.award_flags &= ~0x20;
      }
    }
  }
  if (count == 0 && (record.present & 4) != 0) record.award_flags |= 0x20;

  StringTemp[0] = '\0';
  strcat(StringTemp, "hof_");
  say_num(factionID_);
  strcat(StringTemp, "_");
  say_num(rank + 1);
  strcat(StringTemp, ".pcx");
  WorldClimateWorldWin.write_pcx(StringTemp);
}

// Native constructor 0x0044B200 installs the Win and Buffer views at
// complete-object +0 and +0x444. Table slots select the mapped FameWin
// overrides and secondary adjustor; original table extents remain unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066C268 ? ??_7FameWin@@6BWin@@@ 0x0044B200
// RECOVERY_GENERATED: v1 vtable 0x0066C260 ? ??_7FameWin@@6BBuffer@@@ 0x0044B200

// Record cleanup: hof_load, hof_save and sub_44af90 expand it; only the
// hof_load expansion inside hall_of_fame keeps a call (inline budget).
// Original filenames are unknown.

// ORIGINAL: 0x0044AF70 sub_44af70 0x0044AF70-0x0044AF8D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native44AE50 reads/writes hallfame.jhh with rb/wb and mode759194;35records of8C bytes begin9A3840;44AF70 clears only their present bytes.", "Native44AEE6 owned ButtonGroup receiver753468 is FameWin74DAF8+5970;44AF04 virtual slot57 is Win::set_modal with callbackpop_wait5398E0.", "Root-only Console retirement dependency closure authorized in task history; retain git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/0044ae50.cpp and git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/0044af70.cpp seed interpretations.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/0044af70.cpp"], "reason": "Recover real FameWin entry and record-presence cleanup through existing typed HallOfFame records and owned button group, standard CRT I/O and inherited virtual modal API. Retain historical exact seed obligations and original spans; current binding grants no match or acceptance claim. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?clear_hall_of_fame@@YAXXZ
void __cdecl clear_hall_of_fame() {
    for (int group = 0; group < HallOfFameRecordCount / 5; ++group) {
        for (int rank = 0; rank < 5; ++rank) {
            HallOfFameRecords[group * 5 + rank].present = 0;
        }
    }
}
