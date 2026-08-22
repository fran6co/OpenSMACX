#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["typer", "capstone==5.0.6"]
# ///
"""Constructors and destructors whose `int` is a most-derived flag.

`??0Class@@QAE@H@Z` looks like a constructor taking an int. It is not. That
`H` is MSVC's compiler-injected MOST-DERIVED FLAG, and it only exists for a
class with VIRTUAL BASES: "construct the virtual bases only if I am the
most-derived object". A destructor carries it too, for the same reason in
reverse.

The consequence is that part of the recovery is a DECLARATION rather than a
body: a class declared with no base, or with ordinary inheritance, cannot
produce the vbtable-relative vtable stores or the vtordisp its image body
writes. Declaring the virtual inheritance is what gives those.

WHAT DECLARING IT DOES NOT GIVE YOU IS THE CONSTRUCTOR. Measured with `nm`
and `objdump` on VC6 12.00.8168: a constructor with no explicit parameter
compiles to `??0Class@@QAE@XZ`, with no `H` - the hidden flag is a separate
stack parameter and never appears in the mangled name. Written to take an
`int` it does mangle `@H@Z`, but then reads that int from [ebp+8] and the
hidden flag from [ebp+0xc], and its call site pushes the value TWICE where
the image's `??__E` initialisers push once. So these constructors have to
stay `construct(int a1)` METHODS, which never receive the flag. Destructors
are the opposite case and are safe as real C++ destructors.

    uv run tools/most_derived_flag.py

Reports every catalogued `??0`/`??1` whose mangled name takes exactly one
`H`, together with how its class is declared in `src/`, so the ones whose
declaration still has to be fixed are the ones that stand out.
"""
import os
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402
from decomp.asm import original_asm  # noqa: E402

IMAGE = pathlib.Path(
    os.environ.get("OPENSMACX_IMAGE",
                   REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"))

# A VBTABLE-RELATIVE STORE, which is the tell no name can give you. With
# virtual bases every vtable write goes through the vbtable rather than to a
# fixed offset - `mov eax, [esi]` (the vbtable pointer), `mov ecx, [eax + 4]`
# (the base's offset out of it), `mov [ecx + esi], <vtable>` - so the store's
# address is register PLUS the `this` register. A constructor that genuinely
# takes an int never emits that.
VBTABLE_STORE = re.compile(
    r"mov\s+dword ptr \[e(?:ax|bx|cx|dx|si|di) \+ e(?:ax|bx|cx|dx|si|di)"
    r"(?: \+ 0x[0-9a-f]+)?\], 0x[0-9a-f]+")


def flag_evidence(record) -> str:
    """What the BODY says about the `int`, not what the name says.

    A name filter cannot separate a most-derived flag from a real `int`
    parameter - both mangle to `@H@Z`. This looks for the store shape only
    virtual inheritance produces.
    """
    try:
        listing = original_asm(record, IMAGE)
    except Exception as problem:                      # no span, no image
        return f"unreadable ({str(problem).splitlines()[0][:38]})"
    text = "\n".join(listing.lines).lower()
    stores = len(VBTABLE_STORE.findall(text))
    if stores:
        return f"CONFIRMED, {stores} vbtable-relative store(s)"
    return "NO vbtable store - the int may be a real parameter"

# `??0Name@@QAE@H@Z` / `??1Name@@QAE@H@Z` - one H parameter, nothing else.
FLAGGED = re.compile(r"^\?\?([01])(\w+)@@[QIU]A[EA]@H@Z$")


def declaration_of(name: str, headers: dict) -> tuple[str, str]:
    """How `class name` is declared, and whether that declaration is enough."""
    pattern = re.compile(rf"\bclass\s+{re.escape(name)}\b\s*(:[^{{]*)?\{{")
    for header, text in headers.items():
        found = pattern.search(text)
        if not found:
            continue
        bases = (found.group(1) or "").strip(": \n\t")
        bases = " ".join(bases.split())
        if not bases:
            return header.name, "NO BASE CLASS - layout needs one"
        if "virtual" not in bases:
            return header.name, f"non-virtual `{bases[:56]}` - check it"
        return header.name, f"virtual: {bases[:56]}"
    return "-", "class not declared in src/"


if __name__ == "__main__":
    headers = {}
    for header in sorted((REPO_ROOT / "src").glob("*.h")):
        try:
            headers[header] = header.read_text(errors="replace")
        except OSError:
            continue

    seen, rows = set(), []
    for record in read(REPO_ROOT / "src"):
        found = FLAGGED.match(record.name or "")
        if not found or record.address in seen:
            continue
        seen.add(record.address)
        kind = "ctor" if found.group(1) == "0" else "dtor"
        where, verdict = declaration_of(found.group(2), headers)
        evidence = flag_evidence(record)
        real = evidence.startswith("CONFIRMED")
        rows.append((real and verdict.startswith(
                        ("NO BASE", "non-virtual", "class not")),
                     found.group(2), kind, record, where, verdict, evidence))

    rows.sort(key=lambda r: (not r[0], r[1]))
    for needs_fix, cls, kind, record, where, verdict, evidence in rows:
        mark = "FIX " if needs_fix else "ok  "
        print(f"  {mark}{record.address_hex}  {cls:<16} {kind}  "
              f"{where:<18} {verdict}")
        print(f"        body: {evidence}")
    broken = sum(1 for r in rows if r[0])
    unproven = sum(1 for r in rows if not r[6].startswith("CONFIRMED"))
    print(f"\n{len(rows)} name(s) end @H@Z; {broken} are a CONFIRMED virtual "
          f"base whose declaration cannot produce one")
    print("\nWHAT 'needs one' MEANS, AND WHAT IT DOES NOT. The vbtable stores prove\n"
          "the LAYOUT has a virtual base. They do NOT mean declaring\n"
          "`: public virtual Base` will produce this symbol, and on this compiler it\n"
          "cannot: a constructor on a class with a genuine virtual base mangles to\n"
          "`??0Class@@QAE@XZ`, with the most-derived flag a hidden stack parameter that\n"
          "is never part of the name - while every row above ends `@H@Z`. Measured on\n"
          "VC6 12.00.8168 with nm and objdump, 2026-08-21, and re-checked against\n"
          "CheckBox on 2026-08-22: the real declaration does not compile to the image's\n"
          "symbol, and the hand-composed layout it replaces is the workaround, not an\n"
          "oversight. Fix the LAYOUT here; reach the symbol with a `construct(int)`\n"
          "METHOD, whose call site pushes the flag exactly once.")
    if unproven:
        print(f"{unproven} could not be confirmed from the body - those `int`s "
              f"may be real parameters, do not assume")
