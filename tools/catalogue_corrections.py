#!/usr/bin/env python3
"""Mangled names the catalogue gets wrong, and the bytes that say so.

`functions.csv` is promoted from the canonical IDB export, and the image
carries NO SYMBOLS AT ALL - every one of its 4,821 mangled names is IDA's
analysis, not something the linker wrote down. So a name can simply be wrong,
and four of them provably are: they spell a `void` return (`X`) for functions
whose bodies end `xor eax, eax; ret N`, which is how VC6 returns zero.

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
    0x004C8F40: (
        "?unload@VoiceTx@@QAEXXZ",
        "?unload@VoiceTx@@QAEHXZ",
        "ends `mov dword ptr [esi+0x3c], 0; xor eax, eax; pop esi; ret`. The "
        "clear is the LAST thing before the epilogue and nothing reads it, so "
        "it is a `return 0`. With the void head the body matches 17 of 18 "
        "mnemonics and is 2 bytes short; with `int` it is byte-exact"),
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
