// ORIGINAL: 0x00477E00 ??_GMonuWin@@WEEE@AEPAXI@Z 0x00477E00-0x00477E0B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original ??_GMonuWin@@WEEE@AEPAXI@Z at 0x00477e00 is an 11-byte secondary scalar deleting adjustor; it subtracts 0x444 from ECX and tail-jumps to the complete-object MonuWin scalar deleting entry without touching stack or flags.", "Historical seed correction: the earlier `??3` deallocation interpretation was refuted; the original is a `??_G` scalar-deleting adjustor. `WEEE` demangles to adjustor{1092}, equal to 0x444.", "Pinned MonuWin constructor at 0x00477dac stores Buffer-view vftable 0x0066c878 at this+0x444; the source owner is the typed MonuWin GraphicWin/Buffer multiple-inheritance view.", "Current src/monuwin.cpp COFF Buffer-view vftable first slot selects generated ??_EMonuWin@@WEEE@AEPAXI@Z; its body performs the same 0x444 adjustment and preserves the flag argument.", "Current MonuWin COFF weak external ??_EMonuWin@@UAEPAXI@Z records search mode 2 (library search) and fallback target ??_GMonuWin@@UAEPAXI@Z; this linked image resolves both primary labels to the same physical class deleting body. The object fallback and linked address are per-build evidence, not a general E/G equivalence or owner-selection rule.", "That measured current primary body calls MonuWin::~MonuWin, tests flags bit 0, calls operator delete only when bit 0 is set, returns the saved object pointer, and uses ret 4; this is the per-class measured E-to-G scalar path, not a general vector/scalar equivalence.", "The removed transitional C++ names were ?adjust_this_monu_win@@YIPAXPAX0I@Z and ?scalar_delete_monu_win@@YIPAXPAX0I@Z; no callers exist outside deleting_thunks.cpp/.h, and the current generated Buffer vftable uses the class-owned E adjustor instead.", "Historical seed metadata noted flags hidden;sp_ready;purged_ok, zero direct callers, and no direct call targets. These remain historical observations only; they do not establish virtual-dispatch completeness or acceptance.", "The old secondary record had no BYTE_EXACT or SEMANTIC claim. Its original identity, full span, kind and parsed history are moved unchanged; no matching claim is granted."], "reason": "Bind the pinned secondary scalar deleting identity for MonuWin to its measured genuine Buffer-view generated adjustor. Keep the original G identity and point its current symbol fact at the owner-emitted E adjustor; the per-object weak fallback and bit-0 path above establish why this exact current implementation preserves the original operation."}
// symbol    ??_EMonuWin@@WEEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x00477DD0 ??_GMonuWin@@UAEPAXI@Z 0x00477DD0-0x00477DF1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00477DD0-0x00477DF1", "Pinned class constructor ??0MonuWin@@QAE@XZ at 0x00477C80 stores original class/view table VA 0x66c880 at 0x00477DA6 (mov dword ptr [esi], 0x66c880); the current genuine owner vftable ??_7MonuWin@@6BWin@@@ selects generated ??_GMonuWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EMonuWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GMonuWin@@UAEPAXI@Z at 0x00477DD0 calls the source-mapped ??1MonuWin@@QAE@XZ at 0x00477B10, then tests byte ptr [ebp + 8], 1; je 0x477dea bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/monuwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original MonuWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GMonuWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x00472200 _$E21 0x00472200-0x00472216 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x68254c selects initializer0x472200..0x472216, registering exit0x472220..0x47222a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7f9f58 bound to?MonumentWindowGlobal@@3VMonuWin@@A.", "Original startup instruction evidence: [[\"0x472200\", \"mov\", \"ecx, 0x7f9f58\"], [\"0x472205\", \"call\", \"0x477c80\"], [\"0x47220a\", \"push\", \"0x472220\"], [\"0x47220f\", \"call\", \"0x645398\"], [\"0x472214\", \"pop\", \"ecx\"], [\"0x472215\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x472220\", \"mov\", \"ecx, 0x7f9f58\"], [\"0x472225\", \"jmp\", \"0x477b10\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/monuwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?MonumentWindowGlobal@@3VMonuWin@@A\", \"??0MonuWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?MonumentWindowGlobal@@3VMonuWin@@A\", \"??1MonuWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f398; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?MonumentWindowGlobal@@3VMonuWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E16
// ORIGINAL: 0x00472220 _$E19 0x00472220-0x0047222A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x68254c selects initializer0x472200..0x472216, registering exit0x472220..0x47222a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x7f9f58 bound to?MonumentWindowGlobal@@3VMonuWin@@A.", "Original startup instruction evidence: [[\"0x472200\", \"mov\", \"ecx, 0x7f9f58\"], [\"0x472205\", \"call\", \"0x477c80\"], [\"0x47220a\", \"push\", \"0x472220\"], [\"0x47220f\", \"call\", \"0x645398\"], [\"0x472214\", \"pop\", \"ecx\"], [\"0x472215\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x472220\", \"mov\", \"ecx, 0x7f9f58\"], [\"0x472225\", \"jmp\", \"0x477b10\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/monuwin.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?MonumentWindowGlobal@@3VMonuWin@@A\", \"??0MonuWin@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?MonumentWindowGlobal@@3VMonuWin@@A\", \"??1MonuWin@@UAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f398; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?MonumentWindowGlobal@@3VMonuWin@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E14
#include "monuwin.h"
#include "console.h"
#include "game.h"
#include "game_data_state_window.h"
#include "netdaemon.h"
#include "flic.h"
#include "reportif.h"
#include "win.h"
#include "faction.h"
#include "strings.h"
#include "alpha.h"
#include "tutwin.h"
#include "general.h"
#include "stringstruct.h"
#include "technology.h"
#include "base.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

// The source-owned object at 0x7FD634 is set by the monument trigger before
// init and read here; it is not a member of the MonuWin object.
// Primary entries remain compiler-owned; complete original extent is unproved.
// RECOVERY_GENERATED: v1 vtable 0x0066C880 ? ??_7MonuWin@@6BWin@@@ 0x00477C80
// RECOVERY_GENERATED: v1 vtable 0x0066C878 0x0066C880 ??_7MonuWin@@6BBuffer@@@ 0x00477C80

// Main initialization pass.  The coordinate tables and sprite source
// rectangles below are the observed constructor-owned UI data; all storage
// is addressed through its recovered members rather than through self+offset.
// ORIGINAL: 0x00472230 ?init@MonuWin@@QAEHXZ 0x00472230-0x004736DA;0x00656160-0x006561EE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00472230; retired seed provenance git:670a036f:src/unrecovered/00472230.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?init@MonuWin@@QAEHXZ
// TRIED: two of the ~30 unrolled RECT-translate blocks (24 + 6, roughly 15% of the function's bytes) rewritten as a loop over an offset table instead of being unrolled per-field - semantically identical, not byte-exact for that region. Flic's embedded Buffer/Palette sub-object teardown (explicit dtor calls seen at the two cleanup tails) is not modelled since Flic is opaque in this TU; only its close() call is reproduced. Everything else (branches, GraphicWin::init/Buffer::init/Flic::init/decode_frame loop/load_pcx calls/all Sprite::extract calls/both BaseButton caption-button blocks) is a direct field-offset transcription.
int MonuWin::init() {
  Buffer temporary;
  Flic animation;
  int i;
  window_width_ = 0x320;
  window_height_ = 0x258;
  spot_.init(0x40);

  field_A48_ = ReportIfGlobal.field_1E230_;
  const bool fullscreen = field_A48_ != 0;
  if (fullscreen) {
    origin_x_ = (0x400 - window_width_) / 2;
    origin_y_ = (0x300 - window_height_) / 2;
    client_left_ = origin_x_;
    client_top_ = origin_y_;
    client_right_ = origin_x_ + window_width_;
    client_bottom_ = origin_y_ + window_height_;
    GraphicWin::init((WinScreenWidth - 0x400) / 2, WinScreenHeight - 0x300,
                     0x400, 0x300, "", 0x1000000, 0, 0,
                     0);
  } else {
    origin_x_ = origin_y_ = client_left_ = client_top_ = 0;
    client_right_ = window_width_;
    client_bottom_ = window_height_;
    GraphicWin::init(0, 0, window_width_, window_height_, "",
                     0x1000000, 0, 0, 0);
  }
  frame_or_page_ = ReportIfGlobal.field_1E184_->line_advance();
  buffer_first_.init(0x400, 0x300, 0, 0);

  // The animation and PCX resources are short-lived decode sources.  Their
  // embedded Buffer/Palette and the temporary Buffer are ordinary C++
  // subobjects and release through their destructors on every return path.
  if (animation.init(&buffer_first_, const_cast<char *>("monument.flc"),
                     0, 0, 0) != 0) {
    return 1;
  }
  animation.set_palette_offset(10);
  const int monument = MonumentWindowCount;
  const int frame_count = monument == 0xE ? 0xD : monument;
  for (int frame = 0; frame < frame_count; ++frame) {
    animation.decode_stored_frame();
  }
  if (buffer_second_.load_pcx(monument < 5 ? "monzoom1.pcx" : "monzoom2.pcx",
                           0, 0xA, 0xEC) != 0) {
    return 1;
  }

  // The original initializes a fixed set of rectangles.  These first grid
  // hitboxes are shared by all type panels and are translated by the client
  // origin before Spot::add is used by the redraw switch.
  static const RECT source_rects[17] = {
      {0x13f,0xca,0x1df,0x12b}, {0x90,0x2f,0x293,0x5b},
      {0x65,0x76,0x2bd,0x1de}, {0x12d,0x1a9,0x159,0x1d3},
      {0x1c7,0x1a9,0x1f3,0x1d3}, {0x12d,0x175,0x159,0x19f},
      {0x1c7,0x175,0x1f3,0x19f}, {0xa3,0x1a9,0xcf,0x1d3},
      {0x24e,0x1a9,0x27a,0x1d3}, {0xb7,0x175,0xe3,0x19f},
      {0x23a,0x175,0x266,0x19f}, {0,0x1a9,0x2c,0x1d3},
      {0x2ee,0x1a9,0x31a,0x1d3}, {0x25,0x175,0x51,0x19f},
      {0x2cc,0x175,0x2f8,0x19f}, {0x45,0x141,0x71,0x16b},
      {0x2aa,0x141,0x2d6,0x16b}};
  for (i = 0; i < 17; ++i) {
    rects_3134_[i] = source_rects[i];
    rects_3134_[i].left += client_left_;
    rects_3134_[i].right += client_left_;
    rects_3134_[i].top += client_top_;
    rects_3134_[i].bottom += client_top_;
  }
  static const RECT large_hit_rects[7] = {
      {0xFE,0x1FF,0x138,0x231}, {0x1E0,0x1FF,0x21A,0x231},
      {0xC0,0x1FF,0xFA,0x231}, {0x21E,0x1FF,0x258,0x231},
      {0x81,0x1FF,0xBB,0x231}, {0x25D,0x1FF,0x297,0x231},
      {0x16C,0x6F,0x1B3,0xAF}};
  for (i = 0; i < 7; ++i) {
    rects_366C_[i] = large_hit_rects[i];
  }
  if (monument == 0xE) {
    rects_366C_[6].left = 0x148;
    rects_366C_[6].top = 6;
    rects_366C_[6].right = 0x1D6;
    rects_366C_[6].bottom = 0x6B;
  }
  for (i = 0; i < 7; ++i) {
    rects_366C_[i].left += client_left_;
    rects_366C_[i].right += client_left_;
    rects_366C_[i].top += client_top_;
    rects_366C_[i].bottom += client_top_;
  }

  // Load the player portrait sheet for each faction, extracting the selected
  // faction's large portrait and the small/large faction marks into the two
  // seven-element typed runs.  This follows Players[1..7]'s actual filename
  // ownership; no pointer arithmetic across adjacent Sprite arrays is used.
  for (int player = 1; player < MaxPlayerNum; ++player) {
    StringTemp[0] = '\0';
    strcat(StringTemp, Players[player].filename);
    strcat(StringTemp, "3.pcx");
    if (temporary.load_pcx(StringTemp, 0, 0xA, 0xEC) != 0) {
      return 1;
    }
    const int transparent = WinFillColour;
    if (player == ConsoleGlobal.faction_id_) {
      sprite_first_.extract(&temporary, transparent, 0xA4, 0xB,
                               0xA0, 0x61, 0);
      if (monument == 0xE) {
        sprite_second_.extract(&temporary, transparent, 0xA, 0xB,
                                 0x8E, 0x65, 0);
      } else {
        sprite_second_.extract(&temporary, transparent, 0xA, 0x71,
                                 0x47, 0x40, 0);
      }
    }
    const int slot = player - 1;
    sprites_34E0_[slot].extract(&temporary, transparent, 0xA4, 0x6D,
                                0xA0, 0x61, 0);
    sprites_33AC_[slot].extract(&temporary, transparent, 0xA, 0xB2,
                                0x3A, 0x32, 0);
  }

  if (temporary.load_pcx("monicons.pcx", 0, 0xA, 0xEC) != 0) {
    return 1;
  }
  origin_x_ = 2;
  origin_y_ = 0x226;
  for (i = 0; i < 15; ++i) {
    sprites_2C0C_[i].extract(&temporary, WinFillColour, origin_x_, origin_y_,
                             0x2C, 0x2A, 0);
    if (i == 7) {
      origin_x_ = 2;
      origin_y_ += 0x2B;
    } else {
      origin_x_ += 0x2D;
    }
  }
  origin_x_ = 1;
  origin_y_ = 0x89;
  for (i = 0; i < 15; ++i) {
    sprites_2EA0_[i].extract(&temporary, WinFillColour, origin_x_, origin_y_,
                             0xA0, 0x61, 0);
    if ((i + 1) % 5 == 0) {
      origin_x_ = 1;
      origin_y_ += 0x62;
    } else {
      origin_x_ += 0xA1;
    }
  }
  sprites_3244_[0].extract(&temporary, WinFillColour, 0x116, 0x41,
                           0x4E, 0x3F, 0);
  const RECT icon_rects[6] = {
      {0xFD,0x1F4,0x14B,0x233}, {0x1DC,0x1F4,0x225,0x233},
      {0xBE,0x1F4,0x14B,0x233}, {0x1DC,0x1F4,0x262,0x233},
      {0x7F,0x1F4,0x14B,0x233}, {0x1DC,0x1F4,0x2B1,0x233}};
  for (i = 0; i < 6; ++i) {
    icon_rects_[i] = icon_rects[i];
    icon_rects_[i].left += client_left_;
    icon_rects_[i].right += client_left_;
    icon_rects_[i].top += client_top_;
    icon_rects_[i].bottom += client_top_;
  }
  sprites_3244_[1].extract(&temporary, WinFillColour, 0x165, 0x41,
                           0x49, 0x3F, 0);
  sprites_3244_[2].extract(&temporary, WinFillColour, 1, 0x41,
                           0x8D, 0x3F, 0);
  sprites_3244_[3].extract(&temporary, WinFillColour, 0x8F, 0x41,
                           0x86, 0x3F, 0);
  sprites_3244_[4].extract(&temporary, WinFillColour, 1, 1,
                           0xCC, 0x3F, 0);
  sprites_3244_[5].extract(&temporary, WinFillColour, 0xCE, 1,
                           0xD5, 0x3F, 0);

  // The task's existing Label owner supplies both caption string indices;
  // StringTable is the canonical parser-populated string owner.
  static const int label_offsets[2] = {0xA64, 0xB60};
  for (i = 0; i < 2; ++i) {
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(label_offsets[i] / 4));

    if (field_A48_ != 0) {
      field_A34_ = 0x338;
      field_A38_ = 0x2EB;
      field_A3C_ = 0x400;
      field_A40_ = 0x300;
    } else {
      field_A34_ = 0x258;
      field_A38_ = 0x243;
      field_A3C_ = 0x320;
      field_A40_ = 0x258;
    }
    FlatButton &button = i == 0 ? button_first_ : button_second_;
    button.init(StringTemp, 0x3E7, field_A34_, field_A38_,
                field_A3C_ - field_A34_, field_A40_ - field_A38_, this, 0);
    button.set_border_styles(&InterfaceBoxStyles[47],
                             &InterfaceBoxStyles[46],
                             &InterfaceBoxStyles[45]);
    static_cast<Buffer *>(&button)->set_font(&TutWinBaseWindow.font1_,
                                             0, 0, 0);
    button.redraw();
    button.hide();
  }

  // Remaining extracted art and faction portraits stay in typed arrays;
  // the two action buttons are separate members. The switch cases in on_redraw
  // populate the live Spot list.  This pass also establishes the initial
  // preview/selection state after all resources have loaded.
  fill(0);
  selection_pending_ = 0;
  preview_faction_or_page_ = ConsoleGlobal.faction_id_;
  return 0;
}

// ORIGINAL: 0x004736E0 ?on_redraw@MonuWin@@QAEXXZ 0x004736E0-0x00476704;0x006561EE-0x0065622D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004736E0; retired seed provenance git:670a036f:src/unrecovered/004736e0.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_redraw@MonuWin@@UAEXXZ
void MonuWin::on_redraw() {
  // StringList's StringAllocationBase captures and clears the shared heap
  // during construction, then restores it during destruction.
  StringList description;
  // The common pre-branch setup targets GraphicWin's secondary Buffer.
  Buffer *const canvas = static_cast<Buffer *>(this);
  canvas->set_font(ReportIfGlobal.field_1E184_, 0, 0, 0);
  canvas->set_text_color(0xF2, -1, 1, 1);
  canvas->restore_clip();
  const int faction = ConsoleGlobal.faction_id_;
  preview_faction_or_page_ = faction;
  int i, j, row, type;
  if (selection_pending_ != 0) {
    Buffer panel;
    if (field_A48_ != 0) {
      buffer_second_.copy(canvas, 0, 0, 0, 0, 0x400, 0x300);
    } else {
      buffer_second_.copy(canvas, 0x70, 0x54, 0, 0, 0x320, 0x258);
    }
    // Native path begins with the selected record's localized description.
    // Keep the complete native parser setup and case routing explicit rather
    // than painting a representative icon for unrelated monument types.
    const int type = FactionMonuments[faction].latest_type_;
    const char *description_section = 0;
    if (static_cast<unsigned int>(type) <= 20u) {
    const MonumentRecord &record = FactionMonuments[faction].records_[type];
    switch (type) {
    case 0: {
      parse_num(0, record.year_);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, FactionMonuments[faction].founding_base_name_, -1, -1);
      if (record.word_14_ != 0) {
      description_section = "FOUNDCOLONY1ST";
      } else {
      parse_says(3, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(4, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(5, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(6, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[0].year_);
      description_section = "FOUNDCOLONY2ND";
      }
      break;
    }
    case 1: {
      parse_num(0, record.year_);
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      parse_says(1, tech_name(record.word_8_, 0), -1, -1);
      if (record.word_14_ != 0) {
      description_section = "TECHDISCOVER1ST";
      } else {
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(2, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[1].year_);
      description_section = "TECHDISCOVER2ND";
      }
      break;
    }
    case 2: {
      parse_num(0, record.year_);
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      if (record.word_14_ != 0) {
      if (record.word_8_ == -1) {
      description_section = "SECRETS1STA";
      } else {
      parse_num(1, record.word_8_);
      if (record.word_4_ == -1) {
      description_section = "SECRETS1STB";
      } else {
      parse_num(2, record.word_4_);
      description_section = "SECRETS1STC";
      }
      }
      } else {
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(1, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[2].year_);
      if (record.word_8_ == -1) {
      description_section = "SECRETS2NDA";
      } else {
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      parse_num(2, record.word_8_);
      if (record.word_4_ == -1) {
      description_section = "SECRETS2NDB";
      } else {
      parse_num(3, record.word_4_);
      description_section = "SECRETS2NDC";
      }
      }
      }
      break;
    }
    case 3: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      description_section = "PROTO1ST";
      } else {
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(3, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[3].year_);
      description_section = "PROTO2ND";
      }
      break;
    }
    case 4: {
      parse_num(0, record.year_);
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(3, FactionMonuments[faction].facility_base_name_, -1, -1);
      description_section = "FACILITY1ST";
      } else {
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(3, Players[record.owner_faction_].adj_name_faction, -1, -1);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[4].year_);
      description_section = "FACILITY2ND";
      }
      break;
    }
    case 5: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, StringTable.get(
                     Facility[FacilitySPStart + record.word_8_].name), -1, -1);
      description_section = "PROJECT1ST";
      } else {
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      parse_says(1, StringTable.get(
                     Facility[FacilitySPStart + record.word_8_].name), -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(3, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(4, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[5].year_);
      description_section = "PROJECT2ND";
      }
      break;
    }
    case 6: {
      parse_num(0, record.year_);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      if (record.word_14_ != 0) {
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.word_8_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(3, Players[record.word_8_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(4, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "BATTLEWIN1ST";
      } else {
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(3, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(4, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(5, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[6].year_);
      description_section = "BATTLEWIN2ND";
      }
      break;
    }
    case 7: {
      parse_num(0, record.year_);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(2, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_says(3, FactionMonuments[faction].conquered_base_name_, -1, -1);
      description_section = "BASECAP1ST";
      } else {
      parse_says(2, FactionMonuments[faction].conquered_base_name_, -1, -1);
      parse_says(3, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(4, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(5, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(6, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[7].year_);
      description_section = "BASECAP2ND";
      }
      break;
    }
    case 8: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(2, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "NAVY1ST";
      } else {
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(3, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[8].year_);
      description_section = "NAVY2ND";
      }
      break;
    }
    case 9: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(2, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "AIR1ST";
      } else {
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(3, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[9].year_);
      description_section = "AIR2ND";
      }
      break;
    }
    case 10: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      description_section = "NATIVE1ST";
      } else {
      parse_says(0, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(3, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[10].year_);
      description_section = "NATIVE2ND";
      }
      break;
    }
    case 11: {
      parse_num(0, record.year_);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(0, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      description_section = "SPACE1ST";
      } else {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].noun_gender;
      PluralityDefault = Players[record.owner_faction_].is_noun_plural;
      parse_says(3, Players[record.owner_faction_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(4, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(5, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[11].year_);
      description_section = "SPACE2ND";
      }
      break;
    }
    case 12: {
      parse_num(0, record.year_);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      if (record.word_14_ != 0) {
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(2, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "PRESERVE1ST";
      } else {
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(3, Players[record.owner_faction_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.owner_faction_].is_leader_female;
      PluralityDefault = 0;
      parse_says(4, Players[record.owner_faction_].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_num(1, FactionMonuments[record.owner_faction_].records_[12].year_);
      description_section = "PRESERVE2ND";
      }
      break;
    }
    case 13: {
      parse_num(0, record.year_);
      if (record.word_8_ != -1) {
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[record.word_8_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.word_8_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(2, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(3, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "WINCO-OPUNIFY";
      } else {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      parse_says(2, Players[faction].adj_name_faction, -1, -1);
      description_section = "WINUNIFY";
      }
      break;
    }
    case 14: {
      parse_num(0, record.year_);
      if (record.word_8_ != -1) {
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[record.word_8_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[record.word_8_].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(2, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(3, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "WINCO-OPASCENT";
      } else {
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[faction].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[faction].name_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(2, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      description_section = "WINTRANSCEND";
      }
      break;
    }
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20: {
      parse_num(0, record.year_);
      GenderDefault = Players[faction].noun_gender;
      PluralityDefault = Players[faction].is_noun_plural;
      parse_says(0, Players[faction].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[record.word_8_].noun_gender;
      PluralityDefault = Players[record.word_8_].is_noun_plural;
      parse_says(1, Players[record.word_8_].noun_faction,
                 Players[faction].noun_gender, Players[faction].is_noun_plural);
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(2, Players[record.word_8_].title_leader,
                 Players[faction].is_leader_female, -1);
      GenderDefault = Players[record.word_8_].is_leader_female;
      PluralityDefault = 0;
      parse_says(3, Players[record.word_8_].name_leader,
                 Players[faction].is_leader_female, -1);
      description_section = "KILLFACTION";
      break;
    }
    }
    }
    if (description_section != 0) {
      description.load(const_cast<LPSTR>("monument"),
                       const_cast<LPSTR>(description_section), 1, 0);
    }
    canvas->set_clip(&rects_3134_[1]);
    canvas->write_strings(&description, rects_3134_[1].left,
                          rects_3134_[1].top,
                          rects_3134_[1].right - rects_3134_[1].left, 0);
    canvas->restore_clip();
    if (type == 0xD) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction].filename);
        strcat(StringTemp, "3.pcx");
        panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
        panel.copy(canvas, 0x155, 0x0B, rects_3134_[2].left,
                   rects_3134_[2].top, 0x258, 0x168);
    } else if (type >= 0xF) {
        int counterpart;
        if (static_cast<unsigned int>(type) <= 20u) {
          counterpart = FactionMonuments[faction].records_[type].word_8_;
        } else {
          // The original common painter reads +8 at this selector-derived
          // address even after the switch has routed an out-of-range type to
          // 0x475E58. Preserve that object-representation read without forming
          // a reference beyond the 21-element C++ array.
          const char *const owner_bytes =
              reinterpret_cast<const char *>(&FactionMonuments[faction]);
          memcpy(&counterpart,
                 owner_bytes + type * sizeof(MonumentRecord) +
                     offsetof(MonumentRecord, word_8_),
                 sizeof(counterpart));
        }
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[counterpart].filename);
        strcat(StringTemp, "3.pcx");
        panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
        panel.copy(canvas, 0x155, 0x174, rects_3134_[2].left,
                   rects_3134_[2].top, 0x258, 0x168);
    } else {
        char number[12];
        StringTemp[0] = '\0';
        strcat(StringTemp, "monbig");
        _itoa(type, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, ".pcx");
        panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
        panel.copy(canvas, 0, 0, rects_3134_[2].left,
                   rects_3134_[2].top, 0x258, 0x168);
    }
    button_first_.hide();
    button_second_.show(0);
  } else {
    // Clear transient hit regions, paint the owned decoded animation onto
    // the inherited canvas, and order the first fifteen monument records
    // by year before painting their event icons.
    spot_.kill_type(0);
    const bool fullscreen = field_A48_ != 0;
    if (fullscreen) {
      buffer_first_.copy(static_cast<Buffer *>(this), 0, 0, 0, 0,
                       0x400, 0x300);
    } else {
      buffer_first_.copy(static_cast<Buffer *>(this), 0x70, 0x54, 0, 0,
                       0x320, 0x258);
    }

    int order[15];
    const MonumentFaction &monuments = FactionMonuments[preview_faction_or_page_];
    for (i = 0; i != 15; ++i) {
      order[i] = monuments.records_[i].achieved_ != 0 ? i : -1;
    }
    for (i = 1; i != 15; ++i) {
      for (j = i; j != 15; ++j) {
        if (order[j] == -1) {
          continue;
        }
        if (order[i - 1] == -1 ||
            monuments.records_[order[i - 1]].year_ >
            monuments.records_[order[j]].year_) {
          swap(&order[i - 1], &order[j]);
        }
      }
    }

    RECT row_rect;
    for (row = 0; row != 15; ++row) {
      const int type = order[row];
      if (type == -1 || monuments.records_[type].achieved_ == 0) {
        continue;
      }
      if (row != 14) {
        row_rect = rects_3134_[row + 3];
      } else {
        // Native 0x47621C advances a 16-byte RECT source from +0x3164 for
        // all fifteen sorted entries. Its final source is +0x3244, exactly
        // Sprite[0], even though init only establishes RECT entries through
        // +0x3234. Preserve that object-representation read safely; this is
        // the native anomalous final-row geometry, not a declared RECT view.
        memcpy(&row_rect, &sprites_3244_[0], sizeof(row_rect));
      }
      field_A34_ = row_rect.left;
      field_A38_ = row_rect.top;
      field_A3C_ = row_rect.right;
      field_A40_ = row_rect.bottom;
      sprites_2C0C_[type].draw(
          static_cast<Buffer *>(this),
          sprites_2C0C_[type].transparent_index(), field_A34_, field_A38_);
      spot_.add(type, 0, field_A34_, field_A38_,
                field_A3C_ - field_A34_, field_A40_ - field_A38_);
    }

    selected_monument_type_ = monuments.latest_type_;
    const RECT &primary = rects_3134_[0];
    spot_.add(0x45, 0, primary.left, primary.top,
              primary.right - primary.left,
              primary.bottom - primary.top);

    Sprite *selected_sprite;
    if (selected_monument_type_ == 0xD) {
      selected_sprite = &sprite_first_;
    } else if (selected_monument_type_ < 0xF) {
      selected_sprite = &sprites_2EA0_[selected_monument_type_];
    } else {
      const int counterpart =
          monuments.records_[selected_monument_type_].word_8_;
      selected_sprite = counterpart == 0
          ? &sprites_33AC_[6]
          : &sprites_34E0_[counterpart - 1];
    }
    selected_sprite->draw(static_cast<Buffer *>(this),
                          selected_sprite->transparent_index(),
                          primary.left, primary.top);

    int victory_count = 0;
    for (type = 15; type != 21; ++type) {
      if (monuments.records_[type].achieved_ != 0) {
        ++victory_count;
      }
    }
    if (victory_count != 0) {
      const int even_icon = (victory_count - 1) / 2;
      const RECT &even_rect = icon_rects_[even_icon * 2];
      field_A34_ = even_rect.left;
      field_A38_ = even_rect.top;
      field_A3C_ = even_rect.right;
      field_A40_ = even_rect.bottom;
      sprites_3244_[even_icon * 2].draw(
          static_cast<Buffer *>(this),
          sprites_3244_[even_icon * 2].transparent_index(),
          field_A34_, field_A38_);

      if (victory_count > 1) {
        const int odd_icon = (victory_count - 2) / 2;
        const RECT &odd_rect = icon_rects_[odd_icon * 2 + 1];
        field_A34_ = odd_rect.left;
        field_A38_ = odd_rect.top;
        field_A3C_ = odd_rect.right;
        field_A40_ = odd_rect.bottom;
        sprites_3244_[odd_icon * 2 + 1].draw(
            static_cast<Buffer *>(this),
            sprites_3244_[odd_icon * 2 + 1].transparent_index(),
            field_A34_, field_A38_);
      }

      int record_type = 15;
      for (i = 0; i != victory_count; ++i) {
        const MonumentRecord &record = monuments.records_[record_type];
        const RECT &row = rects_366C_[i];
        if (record.achieved_ == 0) {
          // Native 4764DD advances the absent selector here and again in
          // the common loop increment. One rectangle slot is consumed.
          record_type += 2;
          continue;
        }
        const int counterpart = record.word_8_;
        // The native per-faction lookup uses a 1-based faction id from the
        // record at +0x08; source-owned mark extraction stores player 1
        // at sprites_33AC_[0].
        Sprite &mark = sprites_33AC_[counterpart - 1];
        mark.draw(static_cast<Buffer *>(this),
                  mark.transparent_index(), row.left, row.top);
        spot_.add(record_type, 0, row.left, row.top,
                  row.right - row.left, row.bottom - row.top);
        ++record_type;
      }
    }

    // Native final art is Sprite at +0x3640 and uses the left/top words of
    // the seventh independent hit rectangle at +0x36CC/+0x36D0.
    sprite_second_.draw(static_cast<Buffer *>(this),
                        sprite_second_.transparent_index(),
                          rects_366C_[6].left,
                          rects_366C_[6].top);
    button_second_.hide();
    button_first_.show(0);
  }
}

// ORIGINAL: 0x00476760 ?close@MonuWin@@QAEXXZ 0x00476760-0x00476774 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476760; retired seed provenance git:670a036f:src/recovered/00476760.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?close@MonuWin@@QAEXXZ
void MonuWin::close() {
  release_modal();
  GraphicWin::close();
}

// ORIGINAL: 0x00476780 ?on_left_click@MonuWin@@QAEXHH@Z 0x00476780-0x00476841 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476780; retired seed provenance git:670a036f:src/unrecovered/00476780.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_left_click@MonuWin@@UAEXHH@Z
// TRIED: fixed const-pointer global for 0x94cea0 (indexed table base, address itself scales - own raw cast used). First divergence #1 mov/push (prologue reg alloc), 214 vs 193 B.
void MonuWin::on_left_click(int x, int y) {
  const int faction = ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.primary_state_.trailing_words_[6] & (1u << faction)) != 0 ||
      NetDaemonNet.channels_[1].payload_.flags != 0 ||
      NetDaemonNet.primary_state_.leading_words_[2] != 0) {
    return;
  }
  if (selection_pending_ != 0) {
    selection_pending_ = 0;
    redraw();
    return;
  }

  if (spot_.check(x, y, &y, &x) + 1 == 0) {
    return;
  }
  if (y == 0x45) {
    y = selected_monument_type_;
  }
  FactionMonuments[ConsoleGlobal.faction_id_].latest_type_ = y;
  selected_monument_type_ = y;
  selection_pending_ = 1;
  redraw();
}

// ORIGINAL: 0x00476850 ?on_mouse_move@MonuWin@@QAEXHH@Z 0x00476850-0x004769A9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476850; retired seed provenance git:670a036f:src/recovered/units/00476850.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_mouse_move@MonuWin@@UAEXHH@Z
#pragma optimize("y", off)
void MonuWin::on_mouse_move(int x, int y) {
  if (selection_pending_ != 0) {
    return;
  }
  const int faction = ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.primary_state_.trailing_words_[6] & (1u << faction)) != 0 ||
      NetDaemonNet.channels_[1].payload_.flags != 0 ||
      NetDaemonNet.primary_state_.leading_words_[2] != 0) {
    return;
  }

  if (spot_.check(x, y, &y, &x) + 1 == 0 || y == 0x45) {
    return;
  }

  if (y == 0xD) {
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprite_first_;
    sprite.draw(canvas, sprite.transparent_index(), rects_3134_[0].left,
                rects_3134_[0].top);
  } else if (y < 0xF) {
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprites_2EA0_[y];
    sprite.draw(canvas, sprite.transparent_index(), rects_3134_[0].left,
                rects_3134_[0].top);
  } else {
    // These victory/defeat records store the counterpart faction (0..7) and
    // index one run that starts at the last sprite of the preceding array.
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprites_33AC_[6 + FactionMonuments[preview_faction_or_page_]
                                           .records_[y].word_8_];
    sprite.draw(canvas, sprite.transparent_index(), rects_3134_[0].left,
                rects_3134_[0].top);
  }
  selected_monument_type_ = y;
  soft_update();
}
#pragma optimize("", on)

// ORIGINAL: 0x004769B0 ?on_button_clicked@MonuWin@@QAEXH@Z 0x004769B0-0x004769F7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004769B0; retired seed provenance git:670a036f:src/recovered/units/004769b0.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_button_clicked@MonuWin@@UAEXH@Z
void MonuWin::on_button_clicked(int id) {
  if (id != 999) {
    return;
  }
  if (selection_pending_ != 0) {
    selection_pending_ = 0;
    redraw();
    return;
  }
  release_modal();
  GraphicWin::close();
}

// ORIGINAL: 0x00476A00 ?on_key_click@MonuWin@@QAEHHH@Z 0x00476A00-0x00476A42 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476A00; retired seed provenance git:670a036f:src/recovered/units/00476a00.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_key_click@MonuWin@@UAEHHH@Z
int MonuWin::on_key_click(int key, int state) {
  (void)key;
  (void)state;
  if (selection_pending_ != 0) {
    selection_pending_ = 0;
    redraw();
  } else {
    release_modal();
    GraphicWin::close();
  }
  return 1;
}

// Trigger476AE1 supplies this complete singleton to init. The constructor
// and destructor establish36DC bytes; the adjacent count is independent.
// RECOVERY_OBJECT: v2 0x007F9F58 0x007FD634 ?MonumentWindowGlobal@@3VMonuWin@@A 0x00476A50
MonuWin MonumentWindowGlobal;
// RECOVERY_OBJECT: v2 0x007FD634 0x007FD638 ?MonumentWindowCount@@3HA 0x00476A50
int MonumentWindowCount;

// Native476A50: Player flags use59C-byte stride, records use4F4-byte stride.
// Both tables have real source owners; the old int-pointer scaling was wrong.
// ORIGINAL: 0x00476A50 ?monument@@YAXH@Z 0x00476A50-0x00476B22 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476A50; retired seed provenance source:src/stringstruct.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Native trigger uses the source-owned singleton, real Player rule flags and MonumentFaction records, and inherited modal/GraphicWin close operations; raw int-pointer scaling in the prior transcription multiplied Player stride by four."}
// symbol    ?monument@@YAXH@Z
// TRIED: literal 0 store to the count slot before the loop and the vtable-cast placement are what reach MNEMONIC_ONLY; the rest is register allocation not reproduced.
void __cdecl monument(int from_load) {
    const int faction = ConsoleGlobal.faction_id_;
    if ((Players[faction].rule_flags & 0x80) != 0 || IsMultiplayerNet != 0)
        return;
    if (from_load != 0 &&
        ((GameDataStateStorage.fields_.game_more_preferences_ & 0x1000000) != 0 ||
         (GameDataStateStorage.fields_.game_state_ & 0x10) != 0 ||
         ((GameDataStateStorage.fields_.game_state_ & 1) != 0 && (GameDataStateStorage.fields_.game_state_ & 8) != 0)))
        return;

    MonumentWindowCount = 0;
    for (int event = 0; event < 15; ++event)
        if (FactionMonuments[faction].records_[event].achieved_ != 0)
            ++MonumentWindowCount;
    if (MonumentWindowCount == 0) return;
    const int result = MonumentWindowGlobal.init();
    if (result != 0) return;
    MonumentWindowGlobal.selection_pending_ = from_load != 0;
    Win &modal_window = MonumentWindowGlobal;
    modal_window.set_modal(result, 0, 0);
    MonumentWindowGlobal.GraphicWin::close();
    do_all_draws();
}

// Eight faction blocks begin at94CA08, with21records at38 stride and
// latest-type at+498. Founding/secrets/victory consumers establish the
// natural container; reset's previous94CA18 view began inside record0.
// RECOVERY_OBJECT: v2 0x0094CA08 0x0094F1A8 ?FactionMonuments@@3PAUMonumentFaction@@A 0x00476DA0
MonumentFaction FactionMonuments[MaxPlayerNum];

// ORIGINAL: 0x00476B30 ?clear_monuments@@YAXXZ 0x00476B30-0x00476B61 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned476B30 starts its writes at94CA18 with38stride in8faction rows4F4apart; founding476B70 and secrets476DA0 establish containing-record base94CA08", "recovery/tests/test_setup_dependencies_behavior.py checks everybyte against the exact original22word mutation footprint", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/00476b30.cpp"], "reason": "Reset the shared natural monument container: clear21achieved words atrecord+10 andlatest_type atrow+498, preserving all other bytes. Historical reset-only shifted interpretation remains in Git/task history; original seed and restoration obligations are intact. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl clear_monuments() {
    intptr_t latest = reinterpret_cast<intptr_t>(FactionMonuments)
                    + offsetof(MonumentFaction, latest_type_);
    const intptr_t end = latest + sizeof(FactionMonuments);
    const uintptr_t achieved_offset =
        offsetof(MonumentFaction, latest_type_)
        - offsetof(MonumentFaction, records_)
        - offsetof(MonumentRecord, achieved_);
    do {
        uintptr_t achieved = static_cast<uintptr_t>(latest) - achieved_offset;
        int slots = 21;
        do {
            *reinterpret_cast<int *>(achieved) = 0;
            achieved += sizeof(MonumentRecord);
        } while (--slots);
        *reinterpret_cast<int *>(latest) = -1;
        latest += sizeof(MonumentFaction);
    } while (latest < end);
}

// First-achiever producers: when a peer already holds the record, the image
// rereads the faction parameter from its stack home ([ebp+8]) as the default
// owner instead of reusing the register copy every other use shares. A
// volatile read of the parameter reproduces that single memory load (and the
// register assignment it leaves); a plain read lets VC6 coalesce it.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00476B70 ?mon_colony_founded@@YAXHPAD@Z 0x00476B70-0x00476C84 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:terranx_original.exe:0x00476B70-0x00476C84", "source:src/recovered/units/00476b70.cpp", "Original faction stride4F4, record0 flag/first-owner words and name copy to4AC determine existing type ownership; preserve24byte strncpy behavior."], "reason": "Recover first colony founding in the shared genuine FactionMonuments owner, including first-achiever selection, the bounded faction-owned base-name field and live local notification."}
void __cdecl mon_colony_founded(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[0];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[0].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[0];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native strncpy copies exactly 24 bytes; a full-length name is not
    // forcibly terminated here. The field remains owned by this faction.
    strncpy(FactionMonuments[faction_id].founding_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].founding_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 0;
    if (faction_id == local_faction) {
        monument(1);
    }
}

// ORIGINAL: 0x00476C90 ?mon_tech_discovered@@YAXHH@Z 0x00476C90-0x00476D93
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00476C90 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00476c90.cpp; record1 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_tech_discovered(int faction_id, int tech_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[1];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[1].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[1];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = tech_id;
    FactionMonuments[faction_id].latest_type_ = 1;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00476DA0 ?mon_secrets_of_tech@@YAXH@Z 0x00476DA0-0x00476EDD
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00476da0.cpp preserved original identity/extent; pinned instructions and independent family review establish317byte body and fields; actual-source milestone/reset controls verify shared ownership and native branches"], "reason": "Record first/repeated technology secrets in genuine shared monument record2, retain first-achiever peer interpretation and notify the live local faction."}
void __cdecl mon_secrets_of_tech(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[2];
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
        return;

    if (record.achieved_ != 0) {
        if (record.word_8_ + 1 != 0) {
            if (record.word_4_ + 1 == 0)
                record.word_4_ = year;
        } else {
            record.word_8_ = year;
        }
    } else {
        record.word_14_ = 0;
        int peer;
        for (peer = 0; peer < MaxPlayerNum; ++peer) {
            if (peer != faction_id &&
                FactionMonuments[peer].records_[2].achieved_ != 0)
                ++record.word_14_;
        }

        int first_faction = faction_id;
        if (record.word_14_ != 0) {
            record.word_14_ = 0;
            for (peer = 0; peer < MaxPlayerNum; ++peer) {
                const MonumentRecord &other = FactionMonuments[peer].records_[2];
                if (peer != faction_id && other.achieved_ != 0 &&
                    other.word_14_ != 0)
                    first_faction = peer;
            }
        } else {
            record.word_14_ = 1;
        }
        record.owner_faction_ = first_faction;
        record.year_ = year;
        record.achieved_ = 1;
        record.word_8_ = -1;
        record.word_4_ = -1;
    }
    FactionMonuments[faction_id].latest_type_ = 2;
    if (faction_id == ConsoleGlobal.faction_id_)
        monument(1);
}

// These first-built triggers select individual records in FactionMonuments.
// Their latest-type word belongs to the faction block, not to a fabricated
// per-type overlay extending across unrelated records.
// The same event owner also receives victory and faction-loss notifications.

/*
Purpose: Record the local faction's first built prototype (the mon type-3
         trigger): latch the year, count the rival factions that already
         hold this monument, decide the builder, then open the monument
         window when it was the local faction's own build.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00476ee0.cpp, plain FILE). The seed's TRIED wall
        stands: the original reserves the year local with `push ecx` ahead
        of the callee-save pushes, and the C forms tried all save
        ebx/esi first. rule_flags is read as the typed dword here where
        the image tests the byte at 0x00946F58 + f*0x59C; bit 0x80 is the
        same test. game_year expands from its MEASURED inline in game.h
        where the image calls 0x005C89A0 - the accepted mon_winning_*
        house form above.
*/
// ORIGINAL: 0x00476EE0 ?mon_prototype_built@@YAXH@Z 0x00476EE0-0x00476FDE
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00476EE0 and its complete recorded span identify mon_prototype_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_prototype_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[3];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[3].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[3];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 3;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00476FE0 ?mon_facility_built@@YAXHPAD@Z 0x00476FE0-0x004770F2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00476FE0 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00476fe0.cpp; faction stride4F4, record4, name field4C4 and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_facility_built(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[4];

    if (record.achieved_ != 0 || (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[4].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[4];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    // Native copies exactly 24 bytes, including strncpy's zero padding but
    // without forcing a terminator for a full-length name.
    strncpy(FactionMonuments[faction_id].facility_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].facility_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 4;
    if (faction_id == local_faction) {
        monument(1);
    }
}

// ORIGINAL: 0x00477100 ?mon_secret_project@@YAXHH@Z 0x00477100-0x00477207
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477100 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/recovered/units/00477100.cpp; record5 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
void __cdecl mon_secret_project(int faction_id, int project_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[5];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[5].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[5];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = project_id;
    FactionMonuments[faction_id].latest_type_ = 5;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00477210 ?mon_enemy_destroyed@@YAXHH@Z 0x00477210-0x00477320 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:monument-enemy-destroyed-owner original identity, spans and historical claim retained from git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00477210.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:477210..477320 complete body; record6 stores at 94CB58/60/64/68/6C, latest_type at 94CEA0 and notification after updates.", "source:src/game.h MonumentRecord stride38, MonumentFaction stride4F4 and source bindings; game_year before all guards; Players rule_flags bit80 is alien, not unrelated game state.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00477210.cpp"], "reason": "Recover complete enemy-destruction milestone with genuine FactionMonuments, Players and live ConsoleGlobal ownership. Preserve target-faction fallback when prior achievements lack a first flag; later qualifying peer ties win. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?mon_enemy_destroyed@@YAXHH@Z
// kind      game
void __cdecl mon_enemy_destroyed(int faction_id, int target_faction) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[6];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 ||
        record.achieved_ != 0 || target_faction == 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[6].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = target_faction;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[6];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = owner_faction;
    record.word_8_ = target_faction;
    FactionMonuments[faction_id].latest_type_ = 6;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00477320 ?mon_conquer_base@@YAXHPAD@Z 0x00477320-0x00477434
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00477320 and seed git:fc8112d1ca80fbe743dd051511ad7e0af8454e94:src/recovered/units/00477320.cpp; faction stride4F4, record7, name field4DC and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
void __cdecl mon_conquer_base(int faction_id, char *base_name) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[7];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[7].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[7];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.owner_faction_ = owner_faction;
    record.achieved_ = 1;
    record.year_ = year;
    strncpy(FactionMonuments[faction_id].conquered_base_name_, base_name,
            sizeof(FactionMonuments[faction_id].conquered_base_name_));
    const int local_faction = ConsoleGlobal.faction_id_;
    FactionMonuments[faction_id].latest_type_ = 7;
    if (faction_id == local_faction) {
        monument(1);
    }
}

/*
Purpose: The type-8 trigger: the faction's first naval unit built.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477440.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the naval record table.
*/
// ORIGINAL: 0x00477440 ?mon_naval_unit_built@@YAXH@Z 0x00477440-0x0047753E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477440 and its complete recorded span identify mon_naval_unit_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_naval_unit_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[8];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[8].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[8];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 8;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-9 trigger: the faction's first air unit built.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477540.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the air record table.
*/
// ORIGINAL: 0x00477540 ?mon_air_unit_built@@YAXH@Z 0x00477540-0x0047763E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477540 and its complete recorded span identify mon_air_unit_built.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_air_unit_built(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[9];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[9].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[9];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 9;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-10 trigger: the faction's first native life bred.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477640.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the native-life record table.
*/
// ORIGINAL: 0x00477640 ?mon_native_life_bred@@YAXH@Z 0x00477640-0x0047773E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477640 and its complete recorded span identify mon_native_life_bred.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_native_life_bred(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[10];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[10].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[10];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 10;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The type-11 trigger: the faction's first craft in space.
// size      254 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00477740.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the space record table.
*/
// ORIGINAL: 0x00477740 ?mon_first_in_space@@YAXH@Z 0x00477740-0x0047783E
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477740 and its complete recorded span identify mon_first_in_space.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: MonRecord struct + reinterpret_cast<MonRecord*>(base)+a1 array indexing reproduces every instruction from the flag/active check onward byte-for-byte; only the prologue diverges at MISMATCH #6 (original reserves the `year` local with `push ecx` ahead of the ebx/esi callee-save pushes - same wall as 0x00477840's sibling, and every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_first_in_space(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[11];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[11].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[11];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 11;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// ORIGINAL: 0x00477840 ?mon_built_preserve@@YAXH@Z 0x00477840-0x0047793E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477840 and seed git:91b86557299ab66a61f1184540338d569e780e7e:src/unrecovered/00477840.cpp; record12 at factionstride4F4, year0, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
// TRIED: single `if(!(flag)&&active==0)` short-circuit form vs hoisting `winner`/`count=0` before the scan loop - both land the same MISMATCH #3 (original reserves the stack slot for `year` with `push ecx` ahead of the ebx/esi pushes; every C form tried here has the compiler push ebx/esi first instead).
void __cdecl mon_built_preserve(int faction_id) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[12];

    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 || record.achieved_ != 0) {
        return;
    }

    record.word_14_ = 0;
    for (int other = 0; other < 8; ++other) {
        if (other != faction_id &&
            FactionMonuments[other].records_[12].achieved_ != 0) {
            ++record.word_14_;
        }
    }

    int owner_faction;
    if (record.word_14_ != 0) {
        record.word_14_ = 0;
        owner_faction = *(volatile int *)&faction_id;
        for (int other = 0; other < 8; ++other) {
            const MonumentRecord &other_record =
                FactionMonuments[other].records_[12];
            if (other != faction_id && other_record.achieved_ != 0 &&
                other_record.word_14_ != 0) {
                owner_faction = other;
            }
        }
    } else {
        record.word_14_ = 1;
        owner_faction = faction_id;
    }

    record.achieved_ = 1;
    record.owner_faction_ = owner_faction;
    record.year_ = year;
    FactionMonuments[faction_id].latest_type_ = 12;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: Record the unification victory monument for one faction: skip
         factions already flagged, take the faction's unify record slot
         when its live word is clear, stamp the year/counterpart/faction
         and the two live words, publish 0xD as the faction's latest
         monument type, and open the monument window for the local faction.

// size      135 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x00477940 ?mon_winning_unify@@YAXHH@Z 0x00477940-0x004779C7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477940 and its complete recorded span identify mon_winning_unify.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// kind game
void __cdecl mon_winning_unify(int faction_id, int counterpart) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[13];
    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        return;
    }
    if (record.achieved_ != 0) {
        return;
    }
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = faction_id;
    record.word_8_ = counterpart;
    FactionMonuments[faction_id].latest_type_ = 0xD;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: The transcendence twin of mon_winning_unify: same gates and
         record shape over the faction's trans record slot, publishing
         0xE as the latest monument type instead.

// size      135 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x004779D0 ?mon_winning_trans@@YAXHH@Z 0x004779D0-0x00477A57 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x004779D0 and its complete recorded span identify mon_winning_trans.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: `extern "C"` on the already-declared plain-linkage game_year/monument callees conflicts as C2732; a byte-table extern for 0x94CD18 compiles but the `lea` vs `sub` MISMATCH stayed open.
// kind game
void __cdecl mon_winning_trans(int faction_id, int counterpart) {
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    MonumentRecord &record = FactionMonuments[faction_id].records_[14];
    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        return;
    }
    if (record.achieved_ != 0) {
        return;
    }
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.year_ = year;
    record.owner_faction_ = faction_id;
    record.word_8_ = counterpart;
    FactionMonuments[faction_id].latest_type_ = 0xE;
    if (faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

/*
Purpose: Place a faction-killed faction's shared monument: scan the six
         free 0x38-byte slots for the first clear achieved word, take the
         matching 0xF+i index, stamp the finished-record fields, publish
         the chosen index as the latest monument type, and open the
         monument window for the unflagged local faction.

// size      175 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C89A0 0x00476A50
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
// ORIGINAL: 0x00477A60 ?mon_killed_faction@@YAXHH@Z 0x00477A60-0x00477B0F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned original entry 0x00477A60 and its complete recorded span identify mon_killed_faction.","src/game.h defines MonumentRecord/MonumentFaction and the shared FactionMonuments[8] object; the body uses those typed record/faction owners rather than a second table view.","Existing src/game.cpp milestone producers already own neighboring records 0,1,2,4,5,6,7,12; this event family fills record types 3,8,9,10,11,13,14,15..20 on the same owner.","Actual callers remain in src/base.cpp (build/turn events), src/faction.cpp (faction death), and src/game.cpp (unification); notification calls resolve through the existing monument UI API.","Historical seed comments and unclaimed/non-byte-exact status are retained; no new byte or semantic claim is made."],"reason":"Keep this milestone producer with the existing source-owned FactionMonuments event family. Retain the exact branch order, record writes, unchecked faction indices, current-local-faction notification timing, and original matching/restoration obligations."}
// TRIED: the flag byte needs its own extern beside the scaffold's int-typed binding (C2373), and the three table bases need masked relocations; the spill of `year` to an extra frame slot stayed a 0.86-similarity MISMATCH.
// kind game
void __cdecl mon_killed_faction(int faction_id, int counterpart) {
    // The image tests the -1 sentinel as `chosen + 1` (lea/test).
    int chosen = -1;
    const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
    for (int index = 0; index < 6; ++index) {
        if (FactionMonuments[faction_id].records_[index + 0xF].achieved_ == 0
            && chosen + 1 == 0) {
            chosen = index + 0xF;
        }
    }
    if (chosen + 1 == 0) {
        return;
    }
    MonumentRecord &record = FactionMonuments[faction_id].records_[chosen];
    record.year_ = year;
    record.word_8_ = counterpart;
    record.word_14_ = 1;
    record.achieved_ = 1;
    record.owner_faction_ = faction_id;
    FactionMonuments[faction_id].latest_type_ = chosen;
    if ((Players[faction_id].rule_flags & 0x80) == 0
        && faction_id == ConsoleGlobal.faction_id_) {
        monument(1);
    }
}

// Native teardown is the reverse member order (including the two scalar
// Sprites); typed member/base destruction supplies it without placement calls.
// ORIGINAL: 0x00477B10 ??1MonuWin@@QAE@XZ 0x00477B10-0x00477C79;0x00655F90-0x0065607E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00477B10; retired seed provenance git:670a036f:src/recovered/units/00477b10.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ??1MonuWin@@UAE@XZ

// The native constructor constructs GraphicWin, both FlatButtons, Spot,
// Buffers, the five Sprite arrays and two scalar Sprites in this declaration
// order. These are genuine typed members, so C++ performs exactly that
// lifetime sequence; the explicit native calls are the compiler's expansion
// of these default constructions, not a second user body to duplicate.
// ORIGINAL: 0x00477C80 ??0MonuWin@@QAE@XZ 0x00477C80-0x00477DC7;0x00656080-0x0065615E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00477C80; retired seed provenance git:670a036f:src/recovered/units/00477c80.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ??0MonuWin@@QAE@XZ
MonuWin::MonuWin() : GraphicWin() {}
