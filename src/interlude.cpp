#include "stdafx.h"
// ORIGINAL: 0x0045F710 ??_GInterlude@@UAEPAXI@Z 0x0045F710-0x0045F731 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/deleting_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ??_GInterlude@@UAEPAXI@Z
// ORIGINAL: 0x0045F910 ??_GInterlude@@WEEE@AEPAXI@Z 0x0045F910-0x0045F91B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/deleting_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// Original adjustor subtracts 0x444 (Buffer subobject) and jumps to 0x0045F710; catalogue ??3 was corrected to ??_G, while this compiler emits ??_E.
// symbol    ??_EInterlude@@WEEE@AEPAXI@Z
#include "interlude.h"
#include "messagewin.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "base.h"
#include "alpha.h"
#include "strings.h"
#include "win.h"
#include "xpops.h"
#include "font.h"
#include "popup_fonts.h"

// Shared storage closure belongs to the coordinator's original owners.
// The pointer cell selects this observed NUL-terminated text-file identifier.
// The allocation extent of the pointed-to original string is unresolved.
// RECOVERY_OBJECT: v1 0x00691B70 _InterludeTextName
static char InterludeTextName[] = "INTERLUDEX";
// RECOVERY_OBJECT: v1 0x00691B08 ?InterludeTextFile@@3PADA
char *InterludeTextFile = InterludeTextName;
int __cdecl X_pop(char *caption, const char *label,
                                int (__cdecl *callback)());

// ORIGINAL: 0x0045F200 ??0Interlude@@QAE@XZ 0x0045F200-0x0045F2DF;0x00655800-0x00655820 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0045f200.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original construction45F200 and its cleanup span establish real GraphicWin base plus RECT A14, StringList A24 and Buffer A54; natural member construction replaces the opaque hypothesis layout."}
// TRIED: reproducing the `push -1/push handler/mov fs:[0]` SEH frame and the two unwind funclets (~GraphicWin, and a tail call into sub_406820 on `this+0xa24`) - explicit placement-new / raw pointer-cast statements in an ordinary function body do not make MSVC emit member-initializer-list unwind protection, so this is landed without it (same call as 0x004849D0).
Interlude::Interlude() {}
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0045F740 ??1Interlude@@QAE@XZ 0x0045F740-0x0045F905;0x00655820-0x00655859 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/0045f740.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original destructor45F740 and cold cleanup establish real member/base teardown; declare genuine virtual lifecycle so compiler owns deleting destructors and receiver adjustments."}
// symbol    ??1Interlude@@UAE@XZ
Interlude::~Interlude() { close(); }

// ORIGINAL: 0x0045F640 ?close@Interlude@@QAEXXZ 0x0045F640-0x0045F6E5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/stringstruct.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle", "Original45F640..45F6E5; retired seed git:117282ee:src/unrecovered/0045f640.cpp records the same identity and spans with its shim TRIED note kept below.", "The GraphicWin close chain is graphicwin.h's own close on the unadjusted receiver (the single-inheritance hypothesis layout), the caption list is the +0xA24 StringList block with head/next/count at +0xA2C/+0xA30/+0xA34, and the dialogue Buffer at +0xA54 closes through buffer.h's own close.", "The list's slot-1 release and each payload's deleting destructor run through member-fn-pointer unions because their classes are unnamed; the adjusted-receiver dispatch is the seed's vtbl+4 walk.", "Prior interpretation: Coverage transcription of the intermission teardown with the list drain kept at the seed's shape."], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?close@Interlude@@QAEXXZ
void Interlude::close() {
    GraphicWin::close();
    captions_.close();
    art_.close();
}

// ORIGINAL: 0x0045F6F0 ?on_key_down@Interlude@@QAEXH@Z 0x0045F6F0-0x0045F700 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/delegation_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?on_key_down@Interlude@@UAEHH@Z
int Interlude::on_key_down(int) {
    release_modal();
    return 1;
}

// ORIGINAL: 0x0045F700 ?on_left_down@Interlude@@QAEXHH@Z 0x0045F700-0x0045F70B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/delegation_thunks.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle"], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?on_left_down@Interlude@@UAEXHH@Z
void Interlude::on_left_down(int, int) { release_modal(); }

// stand-in: width helper; its inline expansion loads right before left
// as the image does (0x0045F610).
static inline int interlude_rect_width(const RECT *rect) {
    return rect->right - rect->left;
}

// ORIGINAL: 0x0045F5C0 ?on_redraw@Interlude@@QAEHXZ 0x0045F5C0-0x0045F63E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original instruction/call evidence retained from src/stringstruct.cpp", "source:src/interlude.h: real GraphicWin base, embedded StringList and Buffer, compiler-generated deleting/adjustor lifecycle", "Original45F5C0..45F63E; retired seed git:117282ee:src/unrecovered/0045f5c0.cpp records the same identity and spans.", "The receivers are the +0xA54 dialogue Buffer whose seven-int copy overload is buffer.h's own 0x005DFF00 blitter and the +0xA24 StringList write_strings draw; the frame numbers are read at +0x4C4/+0x4C8 and +0xAD4/+0xAD8 exactly as the seed measured them.", "Prior interpretation: Coverage transcription of the intermission repaint with the receiver spelled explicitly because Interlude has no header declaration yet."], "reason": "Original member/callback/lifetime identity is owned by genuine Interlude C++ hierarchy; actual emitted symbol replaces the retired raw receiver shim, preserving historical spans and claims."}
// symbol    ?on_redraw@Interlude@@UAEXXZ
void Interlude::on_redraw() {
    Buffer *canvas = static_cast<Buffer *>(this);
    const int width = art_.dib_.bmiHeader.biWidth;
    const int height = art_.dib_.bmiHeader.biHeight;
    art_.copy(canvas, 0, 0, (dib_.bmiHeader.biWidth - width) / 2,
              (height - dib_.bmiHeader.biHeight) / 2, width, -height);
    Buffer::write_strings(&captions_, text_rect_.left, text_rect_.top,
                           interlude_rect_width(&text_rect_), 0);
}

// ORIGINAL: 0x0045F2E0 ?exec@Interlude@@QAEXPAD@Z 0x0045F2E0-0x0045F5C0
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0045f2e0.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original45F2E0 configures Interlude art, captions, clip rectangle and modal lifecycle on actual embedded Buffer/StringList members; source-owned borrowed fonts preserve shared producer relationships."}
// TRIED: many named C locals (rect, w, qv, qh, str_height, half, i, ...) push the compiler to `sub esp,N` for the frame; original reserves one dword via `push ecx` for the single Buffer* local that survives calls. Structure/order matches Ghidra.
void Interlude::exec(char *section) {
    close();
    GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0, 0, 0, 0);
    Buffer *canvas = static_cast<Buffer *>(this);
    canvas->set_text_color(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color2(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color3(MessageWinTextColor, -1, 1, 1);
    canvas->set_text_color_hyper(2, -1, 1, 1);
    canvas->set_font(InterludeNormalFont, InterludeSecondFont,
                     InterludeThirdFont, 0);
    art_.load_pcx(WinScreenWidth == 800 ? "credits.pcx" : "credits2.pcx",
                  nullptr, 10, 0xec);
    captions_.load((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN)
                      ? const_cast<char *>("INTERLUDEA") : InterludeTextFile,
                    section, 1, nullptr);
    text_rect_ = canvas->clip_rect();
    const int inset_height = (text_rect_.bottom - text_rect_.top) / -6;
    const int inset_width = (text_rect_.right - text_rect_.left) / -6;
    text_rect_.left -= inset_width;
    text_rect_.right += inset_width;
    text_rect_.top -= inset_height;
    text_rect_.bottom += inset_height;
    text_rect_.right -= (text_rect_.right - text_rect_.left) / 3;
    if (text_rect_.left > 24) text_rect_.left = 25;
    if (strcmp(section, "INTERLUDE21") == 0
        || strcmp(section, "INTERLUDE22") == 0) {
        text_rect_.top += 50;
        text_rect_.bottom += 50;
    }
    const int height = canvas->write_strings_height(
        &captions_, text_rect_.right - text_rect_.left, 0);
    const int offset = ((text_rect_.bottom - text_rect_.top) - height) / 2;
    text_rect_.top += offset;
    text_rect_.bottom += offset;
    set_modal(0, nullptr, nullptr);
    close();
    do_all_draws();
}

// RECOVERY_GENERATED: v1 vtable 0x0066C708 ? ??_7Interlude@@6BWin@@@ 0x0045F200
// RECOVERY_GENERATED: v1 vtable 0x0066C700 ? ??_7Interlude@@6BBuffer@@@ 0x0045F200
