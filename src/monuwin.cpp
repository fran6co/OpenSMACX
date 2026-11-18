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
// WORKAROUND(noinline): on_redraw's StringList local calls
// StringStruct::StringStruct (0x00401000) out of line (0x00473733), giving
// its three-state EH layout; see stringstruct.h.
#define FONT_LINE_ADVANCE_INLINE
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
#define SPOT_ADD_RECT_INLINE  // stand-in: see Spot::add(int, int, RECT *) in spot.h
#define WIN_RECT_EXTENT_INLINE  // rect_width/rect_height (win.h): on_redraw reads its extents through the RECT
#include "monuwin.h"
#include "rectangle_operations.h"
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

// The window captions the two GraphicWin::init calls pass; each is
// referenced only by MonuWin::init (0x4722BB, 0x472361) and lies in the
// uninitialised .data tail after MonumentWindowCount.
// RECOVERY_OBJECT: v2 0x007FD63C 0x007FD640 ?MonuWinWindowedTitle@@3PADA 0x00472230
char MonuWinWindowedTitle[4];
// RECOVERY_OBJECT: v2 0x007FD640 0x007FD644 ?MonuWinFullscreenTitle@@3PADA 0x00472230
char MonuWinFullscreenTitle[4];

// Main initialization pass.  The coordinate tables and sprite source
// rectangles below are the observed constructor-owned UI data; all storage
// is addressed through its recovered members rather than through self+offset.
// ORIGINAL: 0x00472230 ?init@MonuWin@@QAEHXZ 0x00472230-0x004736DA;0x00656160-0x006561EE
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00472230; retired seed provenance git:d8200f17:src/unrecovered/00472230.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?init@MonuWin@@QAEHXZ
// TRIED: loops over static RECT tables (superseded 2026-11-12 by the image's
// unrolled stores and offset_rect calls). Font::line_advance is the font.h
// header inline the image expands at 0x47238B.
int MonuWin::init() {
  Buffer temporary;
  Flic animation;
  int i;
  window_width_ = 0x320;
  window_height_ = 0x258;
  spot_.init(0x40);
  field_A48_ = ReportIfGlobal.field_1E230_;
  if (!field_A48_) {
    const int width = window_width_;
    const int height = window_height_;
    origin_x_ = 0;
    origin_y_ = 0;
    client_rect_.left = 0;
    client_rect_.top = 0;
    client_rect_.right = width;
    client_rect_.bottom = height;
    GraphicWin::init(0, 0, 0x320, 0x258, MonuWinWindowedTitle, 0x1000000, 0, 0, 0);
  } else {
    const int width = window_width_;
    const int height = window_height_;
    const int x = (0x400 - width) / 2;
    const int y = (0x300 - height) / 2;
    origin_x_ = x;
    origin_y_ = y;
    client_rect_.left = x;
    client_rect_.top = y;
    client_rect_.right = x + width;
    client_rect_.bottom = y + height;
    GraphicWin::init((WinScreenWidth - 0x400) / 2, WinScreenHeight - 0x300, 0x400, 0x300,
                     MonuWinFullscreenTitle, 0x1000000, 0, 0, 0);
  }
  frame_or_page_ = ReportIfGlobal.field_1E184_->line_advance();
  buffer_first_.init(0x400, 0x300, 0, 0);
  if (animation.init(&buffer_first_, "monument.flc", 0, 0, 0)) {
    return 1;
  }
  animation.set_palette_offset(10);
  i = 0;
  while (1) {
    if (i >= (MonumentWindowCount == 0xE ? 0xD : MonumentWindowCount)) {
      break;
    }
    animation.decode_stored_frame();
    i++;
  }
  if (buffer_second_.load_pcx(MonumentWindowCount < 5 ? "monzoom1.pcx" : "monzoom2.pcx",
                              0, 0xA, 0xEC)) {
    return 1;
  }
  rects_3134_[0].left = 0x13F;
  rects_3134_[0].top = 0xCA;
  rects_3134_[0].right = 0x1DF;
  rects_3134_[0].bottom = 0x12B;
  rects_3134_[3].left = 0x12D;
  rects_3134_[3].top = 0x1A9;
  rects_3134_[3].right = 0x159;
  rects_3134_[3].bottom = 0x1D3;
  rects_3134_[4].left = 0x1C7;
  rects_3134_[4].top = 0x1A9;
  rects_3134_[4].right = 0x1F3;
  rects_3134_[4].bottom = 0x1D3;
  rects_3134_[5].left = 0x12D;
  rects_3134_[5].right = 0x159;
  rects_3134_[5].top = 0x175;
  rects_3134_[5].bottom = 0x19F;
  rects_3134_[6].left = 0x1C7;
  rects_3134_[6].top = 0x175;
  rects_3134_[6].right = 0x1F3;
  rects_3134_[6].bottom = 0x19F;
  rects_3134_[7].left = 0xA3;
  rects_3134_[7].top = 0x1A9;
  rects_3134_[7].right = 0xCF;
  rects_3134_[7].bottom = 0x1D3;
  rects_3134_[8].left = 0x24E;
  rects_3134_[8].top = 0x1A9;
  rects_3134_[8].right = 0x27A;
  rects_3134_[8].bottom = 0x1D3;
  rects_3134_[9].left = 0xB7;
  rects_3134_[9].top = 0x175;
  rects_3134_[9].right = 0xE3;
  rects_3134_[9].bottom = 0x19F;
  rects_3134_[10].left = 0x23A;
  rects_3134_[10].top = 0x175;
  rects_3134_[10].right = 0x266;
  rects_3134_[10].bottom = 0x19F;
  rects_3134_[11].left = 0;
  rects_3134_[11].top = 0x1A9;
  rects_3134_[11].right = 0x2C;
  rects_3134_[11].bottom = 0x1D3;
  rects_3134_[12].left = 0x2EE;
  rects_3134_[12].top = 0x1A9;
  rects_3134_[12].right = 0x31A;
  rects_3134_[12].bottom = 0x1D3;
  rects_3134_[13].left = 0x25;
  rects_3134_[13].top = 0x175;
  rects_3134_[13].right = 0x51;
  rects_3134_[13].bottom = 0x19F;
  rects_3134_[14].left = 0x2CC;
  rects_3134_[14].top = 0x175;
  rects_3134_[14].right = 0x2F8;
  rects_3134_[14].bottom = 0x19F;
  rects_3134_[15].left = 0x45;
  rects_3134_[15].top = 0x141;
  rects_3134_[15].right = 0x71;
  rects_3134_[15].bottom = 0x16B;
  rects_3134_[16].left = 0x2AA;
  rects_3134_[16].top = 0x141;
  rects_3134_[16].right = 0x2D6;
  rects_3134_[16].bottom = 0x16B;
  rects_3134_[1].left = 0x90;
  rects_3134_[1].top = 0x2F;
  rects_3134_[1].right = 0x293;
  rects_3134_[1].bottom = 0x5B;
  rects_3134_[2].left = 0x65;
  rects_3134_[2].top = 0x76;
  rects_3134_[2].right = 0x2BD;
  rects_3134_[2].bottom = 0x1DE;
  if (MonumentWindowCount == 0xE) {
    rects_366C_[6].left = 0x148;
    rects_366C_[6].top = 6;
    rects_366C_[6].right = 0x1D6;
    rects_366C_[6].bottom = 0x6B;
  } else {
    rects_366C_[6].left = 0x16C;
    rects_366C_[6].top = 0x6F;
    rects_366C_[6].right = 0x1B3;
    rects_366C_[6].bottom = 0xAF;
  }
  rects_366C_[0].left = 0xFE;
  rects_366C_[0].top = 0x1FF;
  rects_366C_[0].right = 0x138;
  rects_366C_[0].bottom = 0x231;
  rects_366C_[1].left = 0x1E0;
  rects_366C_[1].top = 0x1FF;
  rects_366C_[1].right = 0x21A;
  rects_366C_[1].bottom = 0x231;
  rects_366C_[2].left = 0xC0;
  rects_366C_[2].top = 0x1FF;
  rects_366C_[2].right = 0xFA;
  rects_366C_[2].bottom = 0x231;
  rects_366C_[3].left = 0x21E;
  rects_366C_[3].top = 0x1FF;
  rects_366C_[3].right = 0x258;
  rects_366C_[3].bottom = 0x231;
  rects_366C_[4].left = 0x81;
  rects_366C_[4].top = 0x1FF;
  rects_366C_[4].right = 0xBB;
  rects_366C_[4].bottom = 0x231;
  rects_366C_[5].left = 0x25D;
  rects_366C_[5].top = 0x1FF;
  rects_366C_[5].right = 0x297;
  rects_366C_[5].bottom = 0x231;
  offset_rect(&rects_3134_[0], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[3], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[4], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[5], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[6], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[7], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[8], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[9], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[10], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[11], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[12], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[13], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[14], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[15], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[16], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[1], client_rect_.left, client_rect_.top);
  offset_rect(&rects_3134_[2], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[6], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[0], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[1], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[2], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[3], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[4], client_rect_.left, client_rect_.top);
  offset_rect(&rects_366C_[5], client_rect_.left, client_rect_.top);
  for (i = 1; i < MaxPlayerNum; i++) {
    StringTemp[0] = '\0';
    strcat(StringTemp, Players[i].filename);
    strcat(StringTemp, "3.pcx");
    if (temporary.load_pcx(StringTemp, 0, 0xA, 0xEC)) {
      return 1;
    }
    if (i == ConsoleGlobal.faction_id_) {
      sprite_first_.extract(&temporary, WinFillColour, 0xA4, 0xB, 0xA0, 0x61, 0);
      if (MonumentWindowCount == 0xE) {
        sprite_second_.extract(&temporary, WinFillColour, 0xA, 0xB, 0x8E, 0x65, 0);
      } else {
        sprite_second_.extract(&temporary, WinFillColour, 0xA, 0x71, 0x47, 0x40, 0);
      }
    }
    sprites_34E0_[i - 1].extract(&temporary, WinFillColour, 0xA4, 0x6D, 0xA0, 0x61, 0);
    sprites_33AC_[i - 1].extract(&temporary, WinFillColour, 0xA, 0xB2, 0x3A, 0x32, 0);
  }
  if (temporary.load_pcx("monicons.pcx", 0, 0xA, 0xEC)) {
    return 1;
  }
  origin_x_ = 2;
  origin_y_ = 0x226;
  for (i = 0; i < 15; i++) {
    sprites_2C0C_[i].extract(&temporary, WinFillColour, origin_x_, origin_y_, 0x2C, 0x2A, 0);
    if (i == 7) {
      origin_x_ = 2;
      origin_y_ += 0x2B;
    } else {
      origin_x_ += 0x2D;
    }
  }
  origin_x_ = 1;
  origin_y_ = 0x89;
  for (i = 0; i < 15; i++) {
    sprites_2EA0_[i].extract(&temporary, WinFillColour, origin_x_, origin_y_, 0xA0, 0x61, 0);
    if ((i + 1) % 5) {
      origin_x_ += 0xA1;
    } else {
      origin_x_ = 1;
      origin_y_ += 0x62;
    }
  }
  sprites_3244_[0].extract(&temporary, WinFillColour, 0x116, 0x41, 0x4E, 0x3F, 0);
  set_rect(&icon_rects_[0], 0xFD, 0x1F4, 0x14B, 0x233);
  sprites_3244_[1].extract(&temporary, WinFillColour, 0x165, 0x41, 0x49, 0x3F, 0);
  set_rect(&icon_rects_[1], 0x1DC, 0x1F4, 0x225, 0x233);
  sprites_3244_[2].extract(&temporary, WinFillColour, 1, 0x41, 0x8D, 0x3F, 0);
  set_rect(&icon_rects_[2], 0xBE, 0x1F4, 0x14B, 0x233);
  sprites_3244_[3].extract(&temporary, WinFillColour, 0x8F, 0x41, 0x86, 0x3F, 0);
  set_rect(&icon_rects_[3], 0x1DC, 0x1F4, 0x262, 0x233);
  sprites_3244_[4].extract(&temporary, WinFillColour, 1, 1, 0xCC, 0x3F, 0);
  set_rect(&icon_rects_[4], 0x7F, 0x1F4, 0x14B, 0x233);
  sprites_3244_[5].extract(&temporary, WinFillColour, 0xCE, 1, 0xD5, 0x3F, 0);
  set_rect(&icon_rects_[5], 0x1DC, 0x1F4, 0x2B1, 0x233);
  offset_rect(&icon_rects_[0], client_rect_.left, client_rect_.top);
  offset_rect(&icon_rects_[1], client_rect_.left, client_rect_.top);
  offset_rect(&icon_rects_[2], client_rect_.left, client_rect_.top);
  offset_rect(&icon_rects_[3], client_rect_.left, client_rect_.top);
  offset_rect(&icon_rects_[4], client_rect_.left, client_rect_.top);
  offset_rect(&icon_rects_[5], client_rect_.left, client_rect_.top);
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0xA64 / 4));
  if (field_A48_) {
    scratch_rect_.left = 0x338;
    scratch_rect_.top = 0x2EB;
    scratch_rect_.right = 0x400;
    scratch_rect_.bottom = 0x300;
  } else {
    scratch_rect_.left = 0x258;
    scratch_rect_.top = 0x243;
    scratch_rect_.right = 0x320;
    scratch_rect_.bottom = 0x258;
  }
  button_first_.init(StringTemp, 0x3E7, scratch_rect_.left, scratch_rect_.top, scratch_rect_.right - scratch_rect_.left,
                     scratch_rect_.bottom - scratch_rect_.top, this, 0);
  button_first_.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46],
                                  &InterfaceBoxStyles[45]);
  button_first_.set_font_redraw(&TutWinBaseWindow.font1_);
  button_first_.hide();
  StringTemp[0] = '\0';
  strcat(StringTemp, label_get(0xB60 / 4));
  if (field_A48_) {
    scratch_rect_.left = 0x338;
    scratch_rect_.top = 0x2EB;
    scratch_rect_.right = 0x400;
    scratch_rect_.bottom = 0x300;
  } else {
    scratch_rect_.left = 0x258;
    scratch_rect_.top = 0x243;
    scratch_rect_.right = 0x320;
    scratch_rect_.bottom = 0x258;
  }
  button_second_.init(StringTemp, 0x3E7, scratch_rect_.left, scratch_rect_.top, scratch_rect_.right - scratch_rect_.left,
                      scratch_rect_.bottom - scratch_rect_.top, this, 0);
  button_second_.set_border_styles(&InterfaceBoxStyles[47], &InterfaceBoxStyles[46],
                                   &InterfaceBoxStyles[45]);
  button_second_.set_font_redraw(&TutWinBaseWindow.font1_);
  button_second_.hide();
  fill(0);
  selection_pending_ = 0;
  preview_faction_or_page_ = ConsoleGlobal.faction_id_;
  return 0;
}

// ORIGINAL: 0x004736E0 ?on_redraw@MonuWin@@QAEXXZ 0x004736E0-0x00476704;0x006561EE-0x0065622D
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004736E0; retired seed provenance git:d8200f17:src/unrecovered/004736e0.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_redraw@MonuWin@@UAEXXZ
void MonuWin::on_redraw() {
  StringList description;
  char number[80];
  int order[15];
  int i;
  int j;
  int row;
  int type;
  Buffer::set_font(ReportIfGlobal.field_1E184_, 0, 0, 0);
  Buffer::set_text_color(0xF2, -1, 1, 1);
  preview_faction_or_page_ = ConsoleGlobal.faction_id_;
  Buffer::restore_clip();
  if (selection_pending_ != 0) {
    Buffer panel;
    if (field_A48_ != 0) {
      buffer_second_.copy(this, 0, 0, 0, 0, 0x400, 0x300);
    } else {
      buffer_second_.copy(this, 0x70, 0x54, 0, 0, 0x320, 0x258);
    }
    StringTemp[0] = '\0';
    int faction = preview_faction_or_page_;
    type = FactionMonuments[faction].latest_type_;
    MonumentRecord *rec = &FactionMonuments[faction].records_[type];
    switch (type) {
    case 0:
        parse_num(0, rec->year_);
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, FactionMonuments[preview_faction_or_page_].founding_base_name_, -1, -1);
        if (rec->word_14_) {
            description.load("monument", "FOUNDCOLONY1ST", 1, 0);   // 0x686404 / 0x6863F4 -> 0x475E50
        } else {
            parse_says(3, get_adjective(preview_faction_or_page_), -1, -1);
            parse_says(4, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(5, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(6, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "FOUNDCOLONY2ND", 1, 0);   // 0x686420 / 0x686410 -> 0x475E50
        }
        break;

    case 1:                                                          // 0x473A8B
        parse_num(0, rec->year_);
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(1, tech_name(rec->word_8_, 0), -1, -1);
        if (rec->word_14_) {
            description.load("monument", "TECHDISCOVER1ST", 1, 0);  // 0x68643C / 0x68642C -> 0x475E50
        } else {                                                     // 0x473B00
            parse_says(2, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "TECHDISCOVER2ND", 1, 0);  // 0x686458 / 0x686448 -> 0x475E50
        }
        break;

    case 2:                                                          // 0x473B94
        parse_num(0, rec->year_);
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        if (rec->word_14_) {
            // 0x473BD5: word_8_ loaded once into eax; `lea ecx,[eax+1]; test; je`
            if (rec->word_8_ + 1 != 0) {
                parse_num(1, rec->word_8_);
                if (rec->word_4_ + 1 != 0) {
                    parse_num(2, rec->word_4_);
                    description.load("monument", "SECRETS1STC", 1, 0);  // 0x686470 / 0x686464
                } else {                                                 // 0x473C1A
                    description.load("monument", "SECRETS1STB", 1, 0);  // 0x686488 / 0x68647C
                }
            } else {                                                     // 0x473C2D
                description.load("monument", "SECRETS1STA", 1, 0);      // 0x6864A0 / 0x686494
            }
        } else {                                                         // 0x473C40
            parse_says(1, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            // 0x473CBE: `mov ecx,word_8_; inc ecx; je` (value not kept; reloaded at 0x473CEF)
            if (rec->word_8_ + 1 != 0) {
                parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
                parse_num(2, rec->word_8_);
                if (rec->word_4_ + 1 != 0) {
                    parse_num(3, rec->word_4_);
                    description.load("monument", "SECRETS2NDC", 1, 0);  // 0x6864B8 / 0x6864AC
                } else {                                                 // 0x473D2D
                    description.load("monument", "SECRETS2NDB", 1, 0);  // 0x6864D0 / 0x6864C4
                }
            } else {                                                     // 0x473D40
                description.load("monument", "SECRETS2NDA", 1, 0);      // 0x6864E8 / 0x6864DC
            }
        }
        break;

    case 3:                                                          // 0x473D53
        parse_num(0, rec->year_);
        if (rec->word_14_) {
            parse_says(0, get_title(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(1, get_name(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
            description.load("monument", "PROTO1ST", 1, 0);         // 0x686500 / 0x6864F4
        } else {                                                     // 0x473E54
            parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
            parse_says(1, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(2, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(3, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "PROTO2ND", 1, 0);         // 0x686518 / 0x68650C
        }
        break;

    case 4:                                                          // 0x473FB7
        parse_num(0, rec->year_);
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        if (rec->word_14_) {
            parse_says(1, get_title(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(2, get_name(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(3, FactionMonuments[preview_faction_or_page_].facility_base_name_, -1, -1);
            description.load("monument", "FACILITY1ST", 1, 0);      // 0x686530 / 0x686524
        } else {                                                     // 0x4740DC
            parse_says(1, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(2, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(3, get_adjective(rec->owner_faction_),
                       -1, -1);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "FACILITY2ND", 1, 0);      // 0x686548 / 0x68653C
        }
        break;

    case 5:                                                          // 0x4741E0
        parse_num(0, rec->year_);
        if (rec->word_14_) {
            parse_says(0, get_title(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(1, get_name(preview_faction_or_page_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            // Facility[70 + word_8_].name: [word_8_*0x30 + 0x9A5888]; StringTable @0x9B90D8
            parse_says(2, StringTable.get(Facility[FacilitySPStart +
                              rec->word_8_].name), -1, -1);
            description.load("monument", "PROJECT1ST", 1, 0);       // 0x686560 / 0x686554
        } else {                                                     // 0x4742EA
            parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
            parse_says(1, StringTable.get(Facility[FacilitySPStart +
                              rec->word_8_].name), -1, -1);
            parse_says(2, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(3, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(4, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "PROJECT2ND", 1, 0);       // 0x686578 / 0x68656C
        }
        break;

    case 6:                                                          // 0x47447A
        parse_num(0, rec->year_);
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        if (rec->word_14_) {
            // word_8_ is the defeated faction here.
            parse_says(2, get_title(rec->word_8_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(3, get_name(rec->word_8_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(4, get_noun(rec->word_8_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            description.load("monument", "BATTLEWIN1ST", 1, 0);     // 0x686594 / 0x686584
        } else {                                                     // 0x47465C
            parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
            parse_says(3, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(4, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(5, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "BATTLEWIN2ND", 1, 0);     // 0x6865B0 / 0x6865A0
        }
        break;

    case 7:                                                          // 0x4747BF
        parse_num(0, rec->year_);
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        if (rec->word_14_) {
            parse_says(2, get_noun(preview_faction_or_page_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_says(3, FactionMonuments[preview_faction_or_page_].conquered_base_name_, -1, -1);
            description.load("monument", "BASECAP1ST", 1, 0);       // 0x6865C8 / 0x6865BC
        } else {                                                     // 0x47491C
            parse_says(2, FactionMonuments[preview_faction_or_page_].conquered_base_name_, -1, -1);
            parse_says(3, get_adjective(preview_faction_or_page_), -1, -1);
            parse_says(4, get_title(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(5, get_name(rec->owner_faction_),
                       Players[preview_faction_or_page_].is_leader_female, -1);
            parse_says(6, get_noun(rec->owner_faction_),
                       Players[preview_faction_or_page_].noun_gender,
                       Players[preview_faction_or_page_].is_noun_plural);
            parse_num(1, FactionMonuments[rec->owner_faction_]
                             .records_[type].year_);
            description.load("monument", "BASECAP2ND", 1, 0);       // 0x6865E0 / 0x6865D4
        }
        break;
    case 8:
      parse_num(0, rec->year_);
      if (rec->word_14_) {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_noun(preview_faction_or_page_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "NAVY1ST", 1, 0);          // 0x6865F4 / 0x6865EC
      } else {
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(1, get_title(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_name(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(3, get_noun(rec->owner_faction_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        // 0x474D05: [edi + owner*0x4F4 + 0x94CA08] - indexed by `type` (edi), not literal 8.
        parse_num(1, FactionMonuments[rec->owner_faction_]
                         .records_[type].year_);
        description.load("monument", "NAVY2ND", 1, 0);          // 0x686608 / 0x686600
      }
      break;

    case 9:  // 0x474D3F, same shape as case 8
      parse_num(0, rec->year_);
      if (rec->word_14_) {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_noun(preview_faction_or_page_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "AIR1ST", 1, 0);           // 0x68661C / 0x686614
      } else {
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(1, get_title(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_name(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(3, get_noun(rec->owner_faction_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        parse_num(1, FactionMonuments[rec->owner_faction_]
                         .records_[type].year_);
        description.load("monument", "AIR2ND", 1, 0);           // 0x686630 / 0x686628
      }
      break;

    case 10:  // 0x474FDB
      parse_num(0, rec->year_);
      if (rec->word_14_) {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
        description.load("monument", "NATIVE1ST", 1, 0);        // 0x686648 / 0x68663C
      } else {
        parse_says(0, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(1, get_title(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_name(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(3, get_noun(rec->owner_faction_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        parse_num(1, FactionMonuments[rec->owner_faction_]
                         .records_[type].year_);
        description.load("monument", "NATIVE2ND", 1, 0);        // 0x686660 / 0x686654
      }
      break;

    case 11:  // 0x47523F
      parse_num(0, rec->year_);
      // ? 0x475255: [esi+0xA50] is loaded and its *45 lea started BEFORE the
      // `je`, shared by both arms (each arm starts with Players[pf]); scheduling only.
      if (rec->word_14_) {
        parse_says(0, get_noun(preview_faction_or_page_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        parse_says(1, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        description.load("monument", "SPACE1ST", 1, 0);         // 0x686678 / 0x68666C
      } else {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(3, get_noun(rec->owner_faction_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        parse_says(4, get_title(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(5, get_name(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_num(1, FactionMonuments[rec->owner_faction_]
                         .records_[type].year_);
        description.load("monument", "SPACE2ND", 1, 0);         // 0x686690 / 0x686684
      }
      break;

    case 12:  // 0x475575 - two pf parse_says BEFORE the word_14_ test
      parse_num(0, rec->year_);
      parse_says(0, get_title(preview_faction_or_page_),
                 Players[preview_faction_or_page_].is_leader_female, -1);
      parse_says(1, get_name(preview_faction_or_page_),
                 Players[preview_faction_or_page_].is_leader_female, -1);
      if (rec->word_14_) {
        parse_says(2, get_noun(preview_faction_or_page_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "PRESERVE1ST", 1, 0);      // 0x6866A8 / 0x68669C
      } else {
        parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
        parse_says(3, get_title(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(4, get_name(rec->owner_faction_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_num(1, FactionMonuments[rec->owner_faction_]
                         .records_[type].year_);
        description.load("monument", "PRESERVE2ND", 1, 0);      // 0x6866C0 / 0x6866B4
      }
      break;

    case 13:  // 0x4757AD - test is `lea edx,[ecx+1]; test edx,edx` = word_8_ != -1
      parse_num(0, rec->year_);
      if (rec->word_8_ + 1 != 0) {
        parse_says(0, get_title(rec->word_8_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(rec->word_8_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_noun(rec->word_8_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        // Bug-for-bug: slot 3 repeats the same noun as slot 2 (0x4758CF).
        parse_says(3, get_noun(rec->word_8_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "WINCO-OPUNIFY", 1, 0);    // 0x6866DC / 0x6866CC
      } else {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_adjective(preview_faction_or_page_), -1, -1);
        description.load("monument", "WINUNIFY", 1, 0);         // 0x6866F4 / 0x6866E8
      }
      break;

    case 14:  // 0x475A22
      parse_num(0, rec->year_);
      if (rec->word_8_ + 1 != 0) {
        parse_says(0, get_title(rec->word_8_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(rec->word_8_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_noun(rec->word_8_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        parse_says(3, get_noun(rec->word_8_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "WINCO-OPASCENT", 1, 0);   // 0x686710 / 0x686700
      } else {
        parse_says(0, get_title(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(1, get_name(preview_faction_or_page_),
                   Players[preview_faction_or_page_].is_leader_female, -1);
        parse_says(2, get_noun(preview_faction_or_page_),
                   Players[preview_faction_or_page_].noun_gender,
                   Players[preview_faction_or_page_].is_noun_plural);
        description.load("monument", "WINTRANSCEND", 1, 0);     // 0x68672C / 0x68671C
      }
      break;

    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:  // 0x475CCF
      parse_num(0, rec->year_);
      parse_says(0, get_noun(preview_faction_or_page_),
                 Players[preview_faction_or_page_].noun_gender,
                 Players[preview_faction_or_page_].is_noun_plural);
      parse_says(1, get_noun(rec->word_8_),
                 Players[preview_faction_or_page_].noun_gender,
                 Players[preview_faction_or_page_].is_noun_plural);
      parse_says(2, get_title(rec->word_8_),
                 Players[preview_faction_or_page_].is_leader_female, -1);
      // ? 0x475E00: here GenderDefault is stored BEFORE PluralityDefault=0
      // (every other parse_set site in 8..20 stores Plurality first). Likely
      // scheduling of the same inline parse_set; if it does not match, try
      // explicit `GenderDefault = ...; PluralityDefault = 0;` for this site.
      parse_says(3, get_name(rec->word_8_),
                 Players[preview_faction_or_page_].is_leader_female, -1);
      description.load("monument", "KILLFACTION", 1, 0);        // 0x686744 / 0x686738
      break;  // falls straight into the shared tail at 0x475E50
    }
    Buffer::set_clip(&rects_3134_[1]);
    Buffer::write_strings(&description, rects_3134_[1].left, rects_3134_[1].top,
                  rect_width(&rects_3134_[1]), 0);
    Buffer::restore_clip();
    StringTemp[0] = '\0';
    if (type == 0xD) {
      strcat(StringTemp, Players[preview_faction_or_page_].filename);
      strcat(StringTemp, "3.pcx");
      panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
      panel.copy(this, 0x155, 0xB, rects_3134_[2].left, rects_3134_[2].top,
                 0x258, 0x168);
    } else if (type >= 0xF) {
      strcat(StringTemp,
             Players[FactionMonuments[faction].records_[type].word_8_]
                 .filename);
      strcat(StringTemp, "3.pcx");
      panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
      panel.copy(this, 0x155, 0x174, rects_3134_[2].left, rects_3134_[2].top,
                 0x258, 0x168);
    } else {
      strcat(StringTemp, "monbig");
      _itoa(type, number, 10);
      strcat(StringTemp, number);
      strcat(StringTemp, ".pcx");
      panel.load_pcx(StringTemp, 0, 0xA, 0xEC);
      panel.copy(this, 0, 0, rects_3134_[2].left, rects_3134_[2].top,
                 0x258, 0x168);
    }
    button_first_.hide();
    button_second_.show(0);
  } else {
    spot_.kill_type(0);
    if (field_A48_ != 0) {
      buffer_first_.copy(this, 0, 0, 0, 0, 0x400, 0x300);
    } else {
      buffer_first_.copy(this, 0x70, 0x54, 0, 0, 0x320, 0x258);
    }
    for (i = 0; i < 15; i++) {
      if (FactionMonuments[preview_faction_or_page_].records_[i].achieved_) {
        order[i] = i;
      } else {
        order[i] = -1;
      }
    }
    for (i = 0; i < 14; i++) {
      for (j = i + 1; j < 15; j++) {
        if (order[j] != -1) {
          if (order[i] == -1
              || FactionMonuments[preview_faction_or_page_].records_[order[i]].year_
                 > FactionMonuments[preview_faction_or_page_].records_[order[j]].year_) {
            swap(&order[i], &order[j]);
          }
        }
      }
    }
    for (row = 0; row < 15; row++) {
      if (order[row] != -1
          && FactionMonuments[preview_faction_or_page_].records_[order[row]].achieved_) {
        scratch_rect_ = rects_3134_[row + 3];
        sprites_2C0C_[order[row]].draw(this, scratch_rect_.left, scratch_rect_.top);
        spot_.add(order[row], 0, &scratch_rect_);
      }
    }
    type = FactionMonuments[preview_faction_or_page_].latest_type_;
    selected_monument_type_ = type;
    spot_.add(0x45, 0, &rects_3134_[0]);
    if (type == 0xD) {
      sprite_first_.draw(this, rects_3134_[0].left, rects_3134_[0].top);
    } else if (type < 0xF) {
      sprites_2EA0_[type].draw(this, rects_3134_[0].left, rects_3134_[0].top);
    } else {
      sprites_34E0_[FactionMonuments[preview_faction_or_page_].records_[type].word_8_ - 1]
          .draw(this, rects_3134_[0].left, rects_3134_[0].top);
    }
    int count = 0;
    for (type = 15; type < 21; type++) {
      if (FactionMonuments[preview_faction_or_page_].records_[type].achieved_) {
        count++;
      }
    }
    if (count) {
      int last = count - 1;
      scratch_rect_ = icon_rects_[last / 2 * 2];
      sprites_3244_[last / 2 * 2].draw(this, scratch_rect_.left, scratch_rect_.top);
      count = last;
      if (count) {
        scratch_rect_ = icon_rects_[(count - 1) / 2 * 2 + 1];
        sprites_3244_[(count - 1) / 2 * 2 + 1].draw(this, scratch_rect_.left, scratch_rect_.top);
      }
      for (i = 0, j = 0; i < count + 1; i++, j++) {
        if (!FactionMonuments[preview_faction_or_page_].records_[j + 15].achieved_) {
          j++;
          continue;
        }
        sprites_33AC_[FactionMonuments[preview_faction_or_page_].records_[j + 15].word_8_ - 1]
            .draw(this, rects_366C_[i].left, rects_366C_[i].top);
        spot_.add(j + 15, 0, &rects_366C_[i]);
      }
    }
    sprite_second_.draw(this, rects_366C_[6].left, rects_366C_[6].top);
    button_second_.hide();
    button_first_.show(0);
  }
}

// ORIGINAL: 0x00476760 ?close@MonuWin@@QAEXXZ 0x00476760-0x00476774 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476760; retired seed provenance git:d8200f17:src/recovered/00476760.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?close@MonuWin@@QAEXXZ
void MonuWin::close() {
  release_modal();
  GraphicWin::close();
}

// ORIGINAL: 0x00476780 ?on_left_click@MonuWin@@QAEXHH@Z 0x00476780-0x00476841 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476780; retired seed provenance git:d8200f17:src/unrecovered/00476780.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_left_click@MonuWin@@UAEXHH@Z
// TRIED: fixed const-pointer global for 0x94cea0 (indexed table base, address itself scales - own raw cast used). First divergence #1 mov/push (prologue reg alloc), 214 vs 193 B.
void MonuWin::on_left_click(int x, int y) {
  const int faction = ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.primary_state_.trailing_words_[6] & (1u << faction)) != 0 ||
      NetDaemonNet.field_1BB4_ != 0 ||
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476850; retired seed provenance git:d8200f17:src/recovered/units/00476850.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ?on_mouse_move@MonuWin@@UAEXHH@Z
void MonuWin::on_mouse_move(int x, int y) {
  if (selection_pending_ != 0) {
    return;
  }
  const int faction = ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.primary_state_.trailing_words_[6] & (1u << faction)) != 0 ||
      NetDaemonNet.field_1BB4_ != 0 ||
      NetDaemonNet.primary_state_.leading_words_[2] != 0) {
    return;
  }

  if (spot_.check(x, y, &y, &x) + 1 == 0 || y == 0x45) {
    return;
  }

  if (y == 0xD) {
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprite_first_;
    sprite.draw(canvas, rects_3134_[0].left,
                rects_3134_[0].top);
  } else if (y < 0xF) {
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprites_2EA0_[y];
    sprite.draw(canvas, rects_3134_[0].left,
                rects_3134_[0].top);
  } else {
    // These victory/defeat records store the counterpart faction (0..7) and
    // index one run that starts at the last sprite of the preceding array.
    Buffer *canvas = static_cast<Buffer *>(this);
    Sprite &sprite = sprites_33AC_[6 + FactionMonuments[preview_faction_or_page_]
                                           .records_[y].word_8_];
    sprite.draw(canvas, rects_3134_[0].left,
                rects_3134_[0].top);
  }
  selected_monument_type_ = y;
  soft_update();
}

// ORIGINAL: 0x004769B0 ?on_button_clicked@MonuWin@@QAEXH@Z 0x004769B0-0x004769F7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004769B0; retired seed provenance git:d8200f17:src/recovered/units/004769b0.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00476A00; retired seed provenance git:d8200f17:src/recovered/units/00476a00.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned476B30 starts its writes at94CA18 with38stride in8faction rows4F4apart; founding476B70 and secrets476DA0 establish containing-record base94CA08", "recovery/tests/test_setup_dependencies_behavior.py checks everybyte against the exact original22word mutation footprint", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/00476b30.cpp"], "reason": "Reset the shared natural monument container: clear21achieved words atrecord+10 andlatest_type atrow+498, preserving all other bytes. Historical reset-only shifted interpretation remains in Git/task history; original seed and restoration obligations are intact. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
void __cdecl clear_monuments() {
    for (int faction = 0; faction < MaxPlayerNum; faction++) {
        for (int i = 0; i < 21; i++) {
            FactionMonuments[faction].records_[i].achieved_ = 0;
        }
        FactionMonuments[faction].latest_type_ = -1;
    }
}

// First-achiever producers: when a peer already holds the record, the image
// rereads the faction parameter from its stack home ([ebp+8]) as the default
// owner instead of reusing the register copy every other use shares. A
// volatile read of the parameter reproduces that single memory load (and the
// register assignment it leaves); a plain read lets VC6 coalesce it.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00476B70 ?mon_colony_founded@@YAXHPAD@Z 0x00476B70-0x00476C84 FILE BYTE_EXACT
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

// ORIGINAL: 0x00476C90 ?mon_tech_discovered@@YAXHH@Z 0x00476C90-0x00476D93 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00476C90 and seed git:a75c7108cd0be125d2ace1672e14a2f5994624b6:src/recovered/units/00476c90.cpp; record1 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
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
        (git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00476ee0.cpp, plain FILE). The seed's TRIED wall
        stands: the original reserves the year local with `push ecx` ahead
        of the callee-save pushes, and the C forms tried all save
        ebx/esi first. rule_flags is read as the typed dword here where
        the image tests the byte at 0x00946F58 + f*0x59C; bit 0x80 is the
        same test. game_year expands from its MEASURED inline in game.h
        where the image calls 0x005C89A0 - the accepted mon_winning_*
        house form above.
*/
// ORIGINAL: 0x00476EE0 ?mon_prototype_built@@YAXH@Z 0x00476EE0-0x00476FDE BYTE_EXACT
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00476FE0 and seed git:e927e439956c244db241fec9d58207d347487c19:src/recovered/units/00476fe0.cpp; faction stride4F4, record4, name field4C4 and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
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

// ORIGINAL: 0x00477100 ?mon_secret_project@@YAXHH@Z 0x00477100-0x00477207 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477100 and seed git:a75c7108cd0be125d2ace1672e14a2f5994624b6:src/recovered/units/00477100.cpp; record5 at factionstride4F4, year0, payload8, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
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
// RECOVERY_BINDING: v1 {"evidence": ["task:monument-enemy-destroyed-owner original identity, spans and historical claim retained from git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00477210.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:477210..477320 complete body; record6 stores at 94CB58/60/64/68/6C, latest_type at 94CEA0 and notification after updates.", "source:src/game.h MonumentRecord stride38, MonumentFaction stride4F4 and source bindings; game_year before all guards; Players rule_flags bit80 is alien, not unrelated game state.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/00477210.cpp"], "reason": "Recover complete enemy-destruction milestone with genuine FactionMonuments, Players and live ConsoleGlobal ownership. Preserve target-faction fallback when prior achievements lack a first flag; later qualifying peer ties win. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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

// ORIGINAL: 0x00477320 ?mon_conquer_base@@YAXHPAD@Z 0x00477320-0x00477434 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 00477320 and seed git:e927e439956c244db241fec9d58207d347487c19:src/recovered/units/00477320.cpp; faction stride4F4, record7, name field4DC and24byte strncpy are established by the complete native body.", "Native first-achiever selection includes all eight peer factions; alien target and duplicate events return without updates. Local notification reads the live ConsoleGlobal owner after game_year and name copying."], "reason": "Recover the native milestone and its bounded base-name storage in the genuine shared FactionMonuments owner, not a duplicate raw table view."}
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
        (git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00477440.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the naval record table.
*/
// ORIGINAL: 0x00477440 ?mon_naval_unit_built@@YAXH@Z 0x00477440-0x0047753E BYTE_EXACT
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
        (git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00477540.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the air record table.
*/
// ORIGINAL: 0x00477540 ?mon_air_unit_built@@YAXH@Z 0x00477540-0x0047763E BYTE_EXACT
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
        (git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00477640.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the native-life record table.
*/
// ORIGINAL: 0x00477640 ?mon_native_life_bred@@YAXH@Z 0x00477640-0x0047773E BYTE_EXACT
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
        (git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00477740.cpp, plain FILE); same shape and walls
        as mon_prototype_built above, over the space record table.
*/
// ORIGINAL: 0x00477740 ?mon_first_in_space@@YAXH@Z 0x00477740-0x0047783E BYTE_EXACT
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

// ORIGINAL: 0x00477840 ?mon_built_preserve@@YAXH@Z 0x00477840-0x0047793E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original00477840 and seed git:a75c7108cd0be125d2ace1672e14a2f5994624b6:src/unrecovered/00477840.cpp; record12 at factionstride4F4, year0, firstownerC, achieved10, firstflag14 and latesttype498 are independently established by native stores.", "The original calls game_year before alien/duplicate guards and includes all eight peer records regardless of peer flags; later qualifying first-achiever ties win."], "reason": "Recover the complete native milestone producer against genuine shared monument records, with typed peer selection and live local notification rather than executable-address table views."}
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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00477B10; retired seed provenance git:d8200f17:src/recovered/units/00477b10.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ??1MonuWin@@UAE@XZ

// The native constructor constructs GraphicWin, both FlatButtons, Spot,
// Buffers, the five Sprite arrays and two scalar Sprites in this declaration
// order. These are genuine typed members, so C++ performs exactly that
// lifetime sequence; the explicit native calls are the compiler's expansion
// of these default constructions, not a second user body to duplicate.
// ORIGINAL: 0x00477C80 ??0MonuWin@@QAE@XZ 0x00477C80-0x00477DC7;0x00656080-0x0065615E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00477C80; retired seed provenance git:d8200f17:src/recovered/units/00477c80.cpp; original spans and historical lessons retained.", "image:477C80/477B10 member order and36DC extent; primary66C880 slots12/17/19/25/44.", "image:476A50 binds singleton7F9F58 and independent count7FD634; Player59C and MonumentFaction4F4 strides.", "image:4737EE..475E58 localized owner/grammar cases; 472230/4736E0 resource, clipping, sprite and Spot accesses."], "reason": "Genuine GraphicWin-derived MonuWin owner with actual FlatButton/Spot/Buffer/Sprite members, real callback slots and automatic lifetimes; localized parser owner and grammar inputs follow the original branches rather than current-faction stand-ins."}
// symbol    ??0MonuWin@@QAE@XZ
MonuWin::MonuWin() : GraphicWin() {}
