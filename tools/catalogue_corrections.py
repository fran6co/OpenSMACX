#!/usr/bin/env python3
"""Mangled names the catalogue gets wrong, and the bytes that say so.

`functions.csv` is promoted from the canonical IDB export, and the image
carries NO SYMBOLS AT ALL - every one of its 4,821 mangled names is IDA's
analysis, not something the linker wrote down. So a name can simply be wrong,
and 62 of them provably are. The first family found were four that spell a
`void` return (`X`) for functions whose bodies end `xor eax, eax; ret N`, which
is how VC6 returns zero; the largest is 47 rows that spell `operator delete` for
bodies that are this-adjusting vtable thunks.

There was no way to say so. `emit_translation_unit.Signature` lets
`decode_signature(self.mangled)` overwrite the return type and the parameter
list read from the prototype, and that is deliberate and correct: the derived
prototypes are written in IDA's alphabet, which collapses `char`/`signed char`
and `long`/`int`, and those mangle differently - so the name has to outrank the
prototype's SPELLING or 108 rows emit a symbol no target object holds.

The name outranking IDA's alphabet is not the same as the name being right.
This is the third rank: a hand-curated correction, keyed by address, carrying
the byte evidence for itself. It rewrites the mangled name rather than patching
a decoded return type, so everything downstream - the decode, the emitted
declaration, the symbol - stays consistent with itself.

EVERY CORRECTION ASSERTS WHAT IT REPLACES. If a future export already carries
the fixed name, or carries a third spelling, this raises instead of silently
applying a correction to something it has not seen. A correction that no longer
matches is not a no-op; it means the thing being corrected has moved.
"""

from __future__ import annotations


class Stale(Exception):
    """A correction no longer describes the row it names."""


# address: (name as catalogued, name the bytes support, why)
CORRECTIONS = {
    0x006077F0: (
        "?on_key_click@BaseButton@@QAEXHH@Z",
        "?on_key_click@BaseButton@@QAEHHH@Z",
        "body is `xor eax, eax; ret 8` - it returns zero, and the sibling "
        "class BasePop declares the same handler `int on_key_click(int, int)`"),
    0x00607800: (
        "?on_key_down@BaseButton@@QAEXH@Z",
        "?on_key_down@BaseButton@@QAEHH@Z",
        "body is `xor eax, eax; ret 4`; BasePop::on_key_down returns int"),
    0x00607810: (
        "?on_key_up@BaseButton@@QAEXH@Z",
        "?on_key_up@BaseButton@@QAEHH@Z",
        "body is `xor eax, eax; ret 4`; BasePop::on_key_up returns int"),
    0x005FD2B0: (
        "?do_sound@@YAXXZ",
        "?do_sound@@YAHXZ",
        "body is `xor eax, eax; ret` - a void function would emit `ret` alone"),
    # THE TREE ALREADY KNEW THIS AND COULD NOT ACT ON IT. src/graphicwin.cpp
    # has carried the finding in prose since the body was recovered, ending
    # "Do not 'fix' this back to void on the strength of the name" - and the
    # emitter went on declaring it void, because a note in a .cpp file is not
    # something a tool can read. Reported again by an agent in batch 10, which
    # is the cost of leaving a fact stated instead of encoded.
    #
    # 47 callers inherit the wrong return type, and it is not a cosmetic one:
    # a caller that cannot capture the failure code cannot early-return on it,
    # so the divergence is in the control flow rather than in the byte order.
    0x005D4EF0: (
        "?init@GraphicWin@@QAEXHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z",
        "?init@GraphicWin@@QAEHHHHHPADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z",
        "BaseButton::init calls it at 0x006072A2 and immediately tests the "
        "result (`test eax, eax` / `jne` at 0x006072A7); all three exits set "
        "EAX deliberately - the Win::init passthrough at 0x005D500E, the "
        "Buffer::init passthrough at 0x005D5071, and `xor eax, eax` at "
        "0x005D5081. The independent IDA prototype already reads int, so only "
        "the symbol string is stale"),
    # FOUND BY THE CHECK BELOW, not by an agent - the same family, four rows
    # of it, in one class. Each of the four ends by COMPUTING a value into EAX
    # and returning it: `neg eax; sbb eax, eax; and al, 0xfe; add eax, 2`,
    # which yields 1 or 2. A void function does not build a number in EAX on
    # the instruction before `ret`, and the idiom cannot be a register restore
    # - `sbb eax, eax` reads the carry flag and exists only to make a value.
    0x00616350: (
        "?start@Time@@QAEXP6AXH@ZHHH@Z",
        "?start@Time@@QAEHP6AXH@ZHHH@Z",
        "tail is `and al, 0xfe; add eax, 2; ret 0x10` - a computed return; "
        "0x0063C340 tests the result at 0x0063C356 with `test eax, eax`"),
    0x006164D0: (
        "?pulse@Time@@QAEXP6AXH@ZHHH@Z",
        "?pulse@Time@@QAEHP6AXH@ZHHH@Z",
        "tail is `and al, 0xfe; add eax, 2; ret 0x10`, the same computed "
        "return as its `start` sibling"),
    0x00616650: (
        "?start@Time@@QAEXXZ",
        "?start@Time@@QAEHXZ",
        "tail is `neg eax; sbb eax, eax; and al, 0xfe; add eax, 2; ret`"),
    0x006166C0: (
        "?pulse@Time@@QAEXXZ",
        "?pulse@Time@@QAEHXZ",
        "tail is `neg eax; sbb eax, eax; and al, 0xfe; add eax, 2; ret`"),
    # The RETURN WIDTH is byte-visible, which is easy to miss because both
    # clears are two bytes. `xor al, al` is 32 C0 and `xor eax, eax` is 33 C0,
    # and VC6 emits the narrow one only for a `bool` return. Measured: an
    # otherwise identical int-returning member compiles the null path to
    # `xor eax, eax` under every flag set, never to `xor al, al`.
    0x004C7AB0: (
        "?is_trackset_playing@Midi@@QAEHI@Z",
        "?is_trackset_playing@Midi@@QAE_NI@Z",
        "the no-delegate path is `xor al, al`, which clears AL alone; only a "
        "bool return produces that opcode, and `H` (int) cannot"),
    # The reverse of the BaseButton cases: catalogued void, actually int. Here
    # the giveaway is not the presence of a clear but WHERE it sits - at a
    # merge point both paths reach, which is a `return 0`, not a side effect.
    0x004C5A50: (
        "?release@Wave_In_Device@@QAEXXZ",
        "?release@Wave_In_Device@@QAEHXZ",
        "a trailing `xor eax, eax` at 0x004C5A6D is reached from BOTH the "
        "guard-taken and guard-skipped paths before `pop esi; ret`, so it is a "
        "shared `return 0` and the function returns int. With the void head "
        "the body matches through instruction 10 and is 2 bytes short"),
    # A different KIND of error from every one above. Those correct a return
    # type; these correct whether the function has a `this` at all.
    #
    # `QAA` is a public, NON-STATIC, __cdecl member, and VC6 passes such a
    # member's `this` as a hidden first STACK argument - so the first declared
    # parameter lands at [esp+8]. All three of these read their first
    # parameter at [esp+4], and none references `ecx` or `[ecx+N]` anywhere in
    # its body. Measured rather than reasoned: an isolated one-parameter
    # non-static member compiled through this project's own CL puts the
    # parameter at [esp+8], and the `static` equivalent reproduces [esp+4]
    # exactly. `SAA` is the static spelling.
    #
    # The consequence is why this belongs here rather than in a note: under
    # `QAA` the scaffolding emits a head no body can match, because every
    # stack offset is wrong before anything else is decided. vx_read and
    # vx_write both reach SHAPE_EXACT with each parameter read off by
    # precisely the width of the `this` the bytes do not have.
    0x00618E10: (
        "?vx_malloc@Caviar@@QAAXK@Z",
        "?vx_malloc@Caviar@@SAXK@Z",
        "reads its sole parameter at [esp+4]; a non-static __cdecl member "
        "puts `this` there and the parameter at [esp+8]. No ecx use anywhere"),
    0x00618E30: (
        "?vx_read@Caviar@@QAAXHPADJ@Z",
        "?vx_read@Caviar@@SAXHPADJ@Z",
        "SHAPE_EXACT with all three parameter reads off by exactly +4 "
        "(0x4/0x8/0xc against 0x8/0xc/0x10) - the width of the `this` that "
        "QAA inserts and the bytes do not have"),
    0x00618E50: (
        "?vx_write@Caviar@@QAAXHPAXJ@Z",
        "?vx_write@Caviar@@SAXHPAXJ@Z",
        "same +4 shift as vx_read, mirrored; pure stack-operand forwarding "
        "with zero ecx-as-receiver use"),
    # Back to the return-width family, and this pair travels together: the
    # caller was catalogued void BECAUSE the callee was, so correcting one
    # without the other leaves the residue unexplained at the other end.
    0x00608BA0: (
        "?add_special@StringList@@QAEXPAD@Z",
        "?add_special@StringList@@QAEHPAD@Z",
        "ends `neg eax; sbb eax, eax; neg eax; ret 4` - the canonical VC6 "
        "normalisation of a value into 0 or 1, which a void function has no "
        "reason to compute"),
    0x00629750: (
        "?add_special@StringBox@@QAEXPAD@Z",
        "?add_special@StringBox@@QAEHPAD@Z",
        "ends `xor eax, eax; pop esi; ret 4`. No source form for a genuinely "
        "void function reproduces an unread clear: a `volatile` local spills "
        "to the stack instead of the register, and a plain one is dead-store "
        "eliminated. It is a `return 0`"),
    # The clearest of the family, because BOTH paths are visible: the taken
    # one returns whatever the vtable call returned and the other returns a
    # literal zero. A void function would do neither.
    0x00613220: (
        "?on_scroll_create@Dialogs@@QAEXXZ",
        "?on_scroll_create@Dialogs@@QAEHXZ",
        "ends `call dword ptr [eax+0x20]; ret` on the success path - no eax "
        "handling at all, so the callee's return value IS this function's - "
        "and `xor eax, eax; ret` on the failure path, which is `return 0`. It "
        "also explains why the original does not tail-jmp the dispatch: VC6 "
        "folds that only when the value is discarded"),
    0x004C8F40: (
        "?unload@VoiceTx@@QAEXXZ",
        "?unload@VoiceTx@@QAEHXZ",
        "ends `mov dword ptr [esi+0x3c], 0; xor eax, eax; pop esi; ret`. The "
        "clear is the LAST thing before the epilogue and nothing reads it, so "
        "it is a `return 0`. With the void head the body matches 17 of 18 "
        "mnemonics and is 2 bytes short; with `int` it is byte-exact"),
    # A RETURNED REFERENCE, which is the first correction here whose evidence
    # is not an instruction but a register still holding something at `ret`.
    # Both bodies open `mov eax, ecx` - spending an instruction to move the
    # receiver out of ECX - and both end with `this` in EAX, which a void
    # __thiscall member has no reason to arrange. These are `operator+=` and
    # `operator-=` written the ordinary way, `return *this` included.
    #
    # Measured, not argued: compiled through this project's own CL at
    # `/c /O2 /Gy /GR- /GX`, the `Vector &` head reproduces all 33 bytes of
    # each original, and the catalogued `void` head emits a DIFFERENT
    # sequence - it never moves ECX to EAX, keeps the receiver in ECX and
    # loads the argument into EAX, so every operand is swapped and the body
    # is 29 bytes. The catalogue's whole Vector family is IDA's
    # reconstruction rather than a linker record, and it shows: it spells
    # `AAVVector@@AAVVector@@` where VC6 back-references the repeat to `0`.
    0x00634480: (
        "?__apl@Vector@@QAEXAAVVector@@@Z",
        "?__apl@Vector@@QAEAAVVector@@AAVVector@@@Z",
        "opens `mov eax, ecx` and leaves `this` in EAX at `ret 4`; with a "
        "`Vector &` return and `return *this` the body is byte-exact, with "
        "the catalogued `void` it is a different 29-byte sequence"),
    0x006344B0: (
        "?__ami@Vector@@QAEXAAVVector@@@Z",
        "?__ami@Vector@@QAEAAVVector@@AAVVector@@@Z",
        "the same shape with fsub for fadd, and the same two verdicts"),
    # THE LARGEST FAMILY HERE, and the only one that corrects what KIND of
    # function a row is rather than a type inside it. 47 rows are catalogued
    # `??3<Class>@@SAXPAXI@Z`, which demangles to
    # `public: static void __cdecl <Class>::operator delete(void *, unsigned
    # int)` - a __cdecl static taking two stack arguments, so a callee purge
    # of 0. Every one of the 47 bodies contradicts that:
    #
    #     0x00404430  81 E9 44 04 00 00   sub ecx, 0x444
    #                 E9 65 FF FF FF      jmp 0x004043A0
    #
    # 11 bytes, no stack access at all, the receiver arriving in ECX, and a
    # tail jump into that class's own `??_G<Class>@@UAEPAXI@Z`, which executes
    # `ret 4`. An `operator delete` reads its two arguments off the stack and
    # cannot purge 4. These are MSVC this-adjusting vtable thunks.
    #
    # Measured over all 47, from the pinned image, not assumed from one:
    #   * 46 are `sub ecx, 0x444; jmp rel32` at 11 bytes; 1 is
    #     `sub ecx, dword ptr [ecx - 4]; jmp rel32` at 8 (0x0048BF10 PlanWin,
    #     the vtordisp form);
    #   * all 47 tail-jump into a catalogued `??_G<Class>@@UAEPAXI@Z` whose
    #     first reachable `ret` is `ret 4`;
    #   * the class the target is named for equals the class the thunk was
    #     catalogued under in all 47 - the correction changes the KIND of name
    #     and the adjustment it carries, never which class it belongs to;
    #   * each thunk is referenced by exactly ONE .rdata dword, i.e. it sits in
    #     a vtable slot. `operator delete` is not a virtual function and does
    #     not appear in one. The retired docs/recovery/vtables.csv had written
    #     that down without drawing the conclusion - nine of its rows named one
    #     of these 47 (FlatButton, ImageButton, PushButton) as the FIRST SLOT
    #     of a secondary vtable at +0x444 - and the file is now deleted for
    #     being contradicted elsewhere. THE POINT SURVIVES ITS SOURCE: the
    #     .rdata reference above is measured here, and
    #     `tools/derive_class_vtables.py` finds +0x444 on 43 classes from the
    #     constructors alone, which is `sizeof(Win)` and not an arbitrary
    #     constant;
    #   * 0x444 is not an arbitrary constant: `sizeof(Win) == 0x444`
    #     (src/win.h:196), and src/ has long documented a second vptr stored at
    #     +0x444 by these classes' constructors - the secondary base subobject
    #     whose offset an adjustor thunk exists to subtract.
    #
    # THE CORRECTED SPELLING ROUND-TRIPS BACK TO THE CONSTANT THE BODY
    # SUBTRACTS, which is what makes it a derivation rather than a guess. Each
    # name below was built from the bytes - adjustment from the `sub`, class
    # from the tail-jump target - and then fed to llvm-undname-18, which must
    # print a scalar deleting destructor for that class with the matching
    # adjustment or the row is not published:
    #
    #   ??_GAlphaMovie@@WEEE@AEPAXI@Z ->
    #     [thunk]: public: virtual void * __thiscall
    #     AlphaMovie::`scalar deleting dtor'`adjustor{1092}'(unsigned int)
    #   1092 == 0x444, the immediate at 0x00404432.
    #
    #   ??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z ->
    #     ... PlanWin::`scalar deleting dtor'`vtordisp{-4, 0}'(unsigned int)
    #   and 0x0048BF10 opens `sub ecx, dword ptr [ecx - 4]`.
    #
    # src/deleting_thunks.cpp already implements all 47 as `__fastcall` bodies
    # that step the receiver back by 0x444 and forward - the C++ was never
    # fooled, only the catalogued name was.
    0x00404430: (
        "??3AlphaMovie@@SAXPAXI@Z",
        "??_GAlphaMovie@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004043A0` into "
        "??_GAlphaMovie@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004070B0: (
        "??3BasePop@@SAXPAXI@Z",
        "??_GBasePop@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00406B40` into "
        "??_GBasePop@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004070C0: (
        "??3Scroll@@SAXPAXI@Z",
        "??_GScroll@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00406F20` into "
        "??_GScroll@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004070D0: (
        "??3Popup@@SAXPAXI@Z",
        "??_GPopup@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00406BD0` into "
        "??_GPopup@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004070E0: (
        "??3FlatButton@@SAXPAXI@Z",
        "??_GFlatButton@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004068E0` into "
        "??_GFlatButton@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00408390: (
        "??3AlphaSave@@SAXPAXI@Z",
        "??_GAlphaSave@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00408140` into "
        "??_GAlphaSave@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004083A0: (
        "??3EditBox@@SAXPAXI@Z",
        "??_GEditBox@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00408080` into "
        "??_GEditBox@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00421810: (
        "??3BaseWin@@SAXPAXI@Z",
        "??_GBaseWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004213D0` into "
        "??_GBaseWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00421970: (
        "??3ProdPicker@@SAXPAXI@Z",
        "??_GProdPicker@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004213A0` into "
        "??_GProdPicker@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00428740: (
        "??3CouncWin@@SAXPAXI@Z",
        "??_GCouncWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00428710` into "
        "??_GCouncWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00428F80: (
        "??3Credits@@SAXPAXI@Z",
        "??_GCredits@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00428E60` into "
        "??_GCredits@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00432830: (
        "??3StringBox@@SAXPAXI@Z",
        "??_GStringBox@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00432770` into "
        "??_GStringBox@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00432840: (
        "??3Datalink@@SAXPAXI@Z",
        "??_GDatalink@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00432800` into "
        "??_GDatalink@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0043EFA0: (
        "??3DesignWin@@SAXPAXI@Z",
        "??_GDesignWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0043EDB0` into "
        "??_GDesignWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x0043EFB0: (
        "??3SelectPartWin@@SAXPAXI@Z",
        "??_GSelectPartWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0043EDE0` into "
        "??_GSelectPartWin@@UAEPAXI@Z, which executes `ret 4`; no stack "
        "access and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00440EF0: (
        "??3DiploPop@@SAXPAXI@Z",
        "??_GDiploPop@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00440EC0` into "
        "??_GDiploPop@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00445120: (
        "??3DiploWin@@SAXPAXI@Z",
        "??_GDiploWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00445080` into "
        "??_GDiploWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0044B330: (
        "??3FameWin@@SAXPAXI@Z",
        "??_GFameWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0044B300` into "
        "??_GFameWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004562B0: (
        "??3Gamma@@SAXPAXI@Z",
        "??_GGamma@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00456280` into "
        "??_GGamma@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0045F1F0: (
        "??3MainInterface@@SAXPAXI@Z",
        "??_GMainInterface@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0045F1C0` into "
        "??_GMainInterface@@UAEPAXI@Z, which executes `ret 4`; no stack "
        "access and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x0045F910: (
        "??3Interlude@@SAXPAXI@Z",
        "??_GInterlude@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0045F710` into "
        "??_GInterlude@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00477E00: (
        "??3MonuWin@@SAXPAXI@Z",
        "??_GMonuWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00477DD0` into "
        "??_GMonuWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0047A760: (
        "??3MultiWin@@SAXPAXI@Z",
        "??_GMultiWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0047A6C0` into "
        "??_GMultiWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0047AFE0: (
        "??3NetMsg@@SAXPAXI@Z",
        "??_GNetMsg@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0047AD70` into "
        "??_GNetMsg@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00483840: (
        "??3NetWin@@SAXPAXI@Z",
        "??_GNetWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004837C0` into "
        "??_GNetWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x00484AB0: (
        "??3NewTechWin@@SAXPAXI@Z",
        "??_GNewTechWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00484A80` into "
        "??_GNewTechWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x00488740: (
        "??3PickTech@@SAXPAXI@Z",
        "??_GPickTech@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00488660` into "
        "??_GPickTech@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0048ADF0: (
        "??3PickWin@@SAXPAXI@Z",
        "??_GPickWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0048ADC0` into "
        "??_GPickWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0048BF10: (
        "??3PlanWin@@SAXPAXI@Z",
        "??_GPlanWin@@$4PPPPPPPM@A@AEPAXI@Z",
        "8 bytes, `sub ecx, dword ptr [ecx - 4]; jmp 0x0048BF20` into "
        "??_GPlanWin@@UAEPAXI@Z, which executes `ret 4`. `$4PPPPPPPM@A@` re- "
        "demangles to vtordisp{-4, 0} - the field at -4 and no further "
        "adjustment, which is exactly the one subtraction the body performs"),
    0x00492410: (
        "??3PrefWin@@SAXPAXI@Z",
        "??_GPrefWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004923E0` into "
        "??_GPrefWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004968F0: (
        "??3QuayleWin@@SAXPAXI@Z",
        "??_GQuayleWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004968C0` into "
        "??_GQuayleWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004AD870: (
        "??3ReportWin@@SAXPAXI@Z",
        "??_GReportWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004AD840` into "
        "??_GReportWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004AE9A0: (
        "??3SetupWin@@SAXPAXI@Z",
        "??_GSetupWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004AE760` into "
        "??_GSetupWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004B3F80: (
        "??3CheckButton@@SAXPAXI@Z",
        "??_GCheckButton@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004B3F20` into "
        "??_GCheckButton@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004B3F90: (
        "??3SocialWin@@SAXPAXI@Z",
        "??_GSocialWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004B3F50` into "
        "??_GSocialWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x004BEA20: (
        "??3TutWin@@SAXPAXI@Z",
        "??_GTutWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004BE9F0` into "
        "??_GTutWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004C4CA0: (
        "??3WorldWin@@SAXPAXI@Z",
        "??_GWorldWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004C4C70` into "
        "??_GWorldWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x004E23F0: (
        "??3DipEdit@@SAXPAXI@Z",
        "??_GDipEdit@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x004E23C0` into "
        "??_GDipEdit@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x0051DF70: (
        "??3AlphaMenu@@SAXPAXI@Z",
        "??_GAlphaMenu@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0051DC90` into "
        "??_GAlphaMenu@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x005AE0E0: (
        "??3ReplayWin@@SAXPAXI@Z",
        "??_GReplayWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x005AE0B0` into "
        "??_GReplayWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x005C9EB0: (
        "??3MultiDebug@@SAXPAXI@Z",
        "??_GMultiDebug@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x005C9E80` into "
        "??_GMultiDebug@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x005D7160: (
        "??3GraphicWin@@SAXPAXI@Z",
        "??_GGraphicWin@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x005D7140` into "
        "??_GGraphicWin@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x005FA790: (
        "??3PullDown@@SAXPAXI@Z",
        "??_GPullDown@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x005FA770` into "
        "??_GPullDown@@UAEPAXI@Z, which executes `ret 4`; no stack access and "
        "the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092} "
        "and 1092 == 0x444, the constant subtracted"),
    0x005FC6F0: (
        "??3Menu@@SAXPAXI@Z",
        "??_GMenu@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x005FC6D0` into ??_GMenu@@UAEPAXI@Z, "
        "which executes `ret 4`; no stack access and the receiver stays in "
        "ECX. `WEEE@` re-demangles to adjustor{1092} and 1092 == 0x444, the "
        "constant subtracted"),
    0x00607CE0: (
        "??3BaseButton@@SAXPAXI@Z",
        "??_GBaseButton@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x00607CC0` into "
        "??_GBaseButton@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x006256F0: (
        "??3ImageButton@@SAXPAXI@Z",
        "??_GImageButton@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x006256D0` into "
        "??_GImageButton@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
    0x0062C850: (
        "??3PushButton@@SAXPAXI@Z",
        "??_GPushButton@@WEEE@AEPAXI@Z",
        "11 bytes, `sub ecx, 0x444; jmp 0x0062C830` into "
        "??_GPushButton@@UAEPAXI@Z, which executes `ret 4`; no stack access "
        "and the receiver stays in ECX. `WEEE@` re-demangles to "
        "adjustor{1092} and 1092 == 0x444, the constant subtracted"),
}


def apply(rows: dict) -> dict:
    """Rewrite the catalogued names this module can prove wrong, in place."""
    for address, (catalogued, corrected, _why) in CORRECTIONS.items():
        row = rows.get(address)
        if row is None:
            continue
        if row["name"] == corrected:
            continue
        if row["name"] != catalogued:
            raise Stale(
                f"0x{address:08X} is catalogued as {row['name']!r}, but the "
                f"correction here replaces {catalogued!r}. The row moved; "
                f"re-derive the correction rather than forcing it.")
        row["name"] = corrected
    return rows
