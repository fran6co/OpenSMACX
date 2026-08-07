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
