#!/usr/bin/env -S uv run python
"""Classes spelled FLAT that duplicate another class's fields at the same offsets.

A class written `class Wave {` while the image says it IS a Sound cannot
produce the image's code, and the failure is not subtle: constructing a base
whose destructor is non-trivial, then calling something VC6 cannot prove
nothrow, is what emits the SEH frame. With no base declared there is nothing to
protect and no frame. Wave's constructor sat at 3 of 73 and its destructor at
2 of 101 for exactly this reason; declaring `class Wave : public Sound` and
deleting the 13 duplicated fields took them to 23/73 and 44/101, with all
twelve prologue instructions agreeing.

That was found by hand, on one class, after a stale refusal claimed the flat
spelling was protecting something. This looks for the rest of them.

The signal is field DUPLICATION: a baseless class whose leading run of member
declarations repeats another class's leading run, name for name. Nothing here
proves inheritance on its own - a candidate is a place to go measure, and the
layout arithmetic (sizeof(base) + own fields == the pinned size) is what
actually settles it.

    uv run tools/flat_classes.py
    uv run tools/flat_classes.py --min 4     # longer prefixes only

Exit 0 always: this is a worklist, not a gate.

WHAT IT FOUND, AND WHY THAT IS THE USEFUL PART. Run over the whole tree on
2026-08-22 it returns exactly ONE candidate pair: StringList repeats
StringStruct's first seven fields, `virtual_base_abi_word_` among them, and the
arithmetic settles it - sizeof(StringStruct) is 0x24, sizeof(StringList) is
0x30, and 0x24 + 12 is 0x30.

One. That is a real negative result and it should stop anyone re-running this
expecting a harvest: the flat-class defect was very nearly a one-off, so the
SEH-frame family will NOT mostly dissolve this way. The lever with the large
population is the other one - a constructor calling a base `construct()` in its
BODY, where the image calls it before the members are constructed. See
tools/body_construct_order.py.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

CLASS_HEAD = re.compile(r"^\s*(?:class|struct)\s+(\w+)\s*(?::\s*([^{]*))?\{")
# A data member: optional const/mutable, a type, then a name, then `;` or `[`.
# Deliberately NOT trying to be a C++ parser - a method has `(` before the `;`,
# which is the only distinction that matters here.
MEMBER = re.compile(
    r"^\s*(?!public|private|protected|friend|typedef|using|return)"
    r"(?:const\s+|mutable\s+|static\s+|unsigned\s+|signed\s+)*"
    r"(?P<type>[\w:<>]+)\s*(?P<ptr>[*&]?)\s*(?P<name>\w+)"
    r"\s*(?:\[[^\]]*\])?\s*;")


def classes(path: Path) -> dict[str, tuple[str, list[tuple[str, str]]]]:
    """name -> (base-list, ordered (type, name) members).

    DEPTH IS TRACKED AGAINST THE CLASS'S OWN OPENING LEVEL, and the first
    version of this was not. It popped the class on any line whose `}` count
    was non-zero, so `CheckBox() { ; }` - an inline method body, opened and
    closed on one line - ended the class at its first such member. CheckBox and
    Dialogs both came back with ZERO fields, which is exactly backwards: they
    are the two classes this tool exists to find. Anything with an inline
    method above its data was being read truncated, and the tool reported a
    clean tree because it could not see into it.

    Members are collected only at `start + 1`, so a nested struct's fields do
    not leak into the enclosing class.
    """
    out: dict[str, tuple[str, list[tuple[str, str]]]] = {}
    stack: list[tuple[str, str, list[tuple[str, str]], int]] = []
    depth = 0
    for line in path.read_text(errors="replace").splitlines():
        head = CLASS_HEAD.match(line)
        if head and not line.rstrip().endswith(";"):
            stack.append((head.group(1), (head.group(2) or "").strip(), [],
                          depth))
            depth += line.count("{") - line.count("}")
            continue
        if stack and depth == stack[-1][3] + 1:
            member = MEMBER.match(line)
            if member and "(" not in line.split(";")[0]:
                stack[-1][2].append((member.group("type") + member.group("ptr"),
                                     member.group("name")))
        depth += line.count("{") - line.count("}")
        while stack and depth <= stack[-1][3]:
            name, base, fields, _ = stack.pop()
            out[name] = (base, fields)
    while stack:                      # a header that never closes its class
        name, base, fields, _ = stack.pop()
        out[name] = (base, fields)
    return out


if __name__ == "__main__":
    minimum = 3
    if "--min" in sys.argv:
        minimum = int(sys.argv[sys.argv.index("--min") + 1])

    catalogue: dict[str, tuple[str, list[str], str]] = {}
    for path in sorted(SRC.glob("*.h")):
        for name, (base, fields) in classes(path).items():
            catalogue[name] = (base, fields, path.name)

    # A PREFIX OF PLACEHOLDER NAMES IS NOT EVIDENCE. `field_0_, field_4_,
    # field_8_` is what every un-named layout starts with, so matching on it
    # pairs each of them with all the others: the first run produced 118
    # candidates, and the top of the list was thirty spellings of that one
    # coincidence. A shared prefix only means something if at least one name in
    # it was chosen by a person - `vtable_storage_`, `volume_`, `chain_prev_`.
    generic = re.compile(r"^field_[0-9A-Fa-f]+_$")
    # hypothesis_layouts.h is EXPLICITLY speculative - layouts nobody has
    # measured yet. Pairing hypotheses with each other says nothing about the
    # image, and it buried the real hits.
    skip_files = {"hypothesis_layouts.h"}

    hits = []
    for name, (base, fields, where) in sorted(catalogue.items()):
        if base or len(fields) < minimum or where in skip_files:
            continue        # already derives, too small, or speculative
        for other, (_, other_fields, other_where) in catalogue.items():
            if other == name or len(other_fields) < minimum:
                continue
            shared = 0
            for mine, theirs in zip(fields, other_fields):
                if mine[1] != theirs[1]:
                    break
                shared += 1
            if other_where in skip_files:
                continue
            if shared >= minimum and any(
                    not generic.match(n) for _ty, n in fields[:shared]):
                hits.append((shared, name, where, other, other_where,
                             fields[:shared]))

    # ---- SIGNAL 2: the base is EMBEDDED as a member ------------------------
    # A class that holds another class BY VALUE occupies exactly the storage a
    # base subobject would, and gets none of the language's ordering,
    # conversion or name lookup. CheckBox and Dialogs both carry
    # `GraphicWin virtual_base_;` spelled that way, and checkbox.h says
    # outright that the real declaration is `: virtual GraphicWin, virtual
    # Dialog`.
    #
    # TWO NARROWER RULES THAN THE OBVIOUS ONE, both because the obvious one was
    # measured and was noise:
    #
    #   * "first member is a class" returns AlphaMovie/MCIVideo,
    #     BaseWin/ProdPicker, Popup/Scroll - classes that ALREADY derive, whose
    #     first member merely follows the base. And it MISSES CheckBox, whose
    #     GraphicWin sits after six hand-composed fields. Position is the wrong
    #     discriminator.
    #   * The right one is whether the embedded type is a thing this tree
    #     DERIVES FROM ELSEWHERE. GraphicWin is a base for 28 classes, so a
    #     class holding one by value is suspicious; VOX_Vect and Sprite are
    #     bases for nobody, so FactionArt holding a Sprite is just a member.
    #
    # Still not proof - `osmx show` on the constructor is what settles it.
    used_as_base: set[str] = set()
    for _name, (base, _fields, _where) in catalogue.items():
        for token in re.split(r"[,\s]+", base or ""):
            token = token.strip()
            if token and token not in ("public", "private", "protected",
                                       "virtual"):
                used_as_base.add(token)

    # GROUPED BY CLASS, because more than one embedded base is MULTIPLE
    # INHERITANCE and that is the shape here - CheckBox holds a GraphicWin AND
    # a Dialog, and dialogs.h describes "what it tells its five bases to do".
    # Reported per-member the list was 36 rows of the same few classes, with
    # Sprite arrays drowning it; per-class it is short and the arity is the
    # interesting column.
    # RESTRICTED TO BASELESS CLASSES, AND THEN SHOWING EVERY CLASS-TYPED
    # MEMBER. Both halves were measured into place:
    #
    #   * A class that already declares a base is not the defect - its embedded
    #     members are members. Including them gave 36 rows of Sprite arrays.
    #   * "the embedded type is used as a base elsewhere" is CIRCULAR here and
    #     it silently dropped the most important row. CheckBox holds a
    #     GraphicWin AND a Dialog - checkbox.h says the real declaration is
    #     `: virtual GraphicWin, virtual Dialog` - but nothing in this tree
    #     derives from Dialog YET, precisely because these classes are the ones
    #     still spelled flat. Filtering on it hid the second base of a
    #     multiple-inheritance class.
    #
    # So: no declared base, and every member whose type is a class this tree
    # declares. More than one is multiple inheritance.
    embedded: dict[str, tuple[str, list[tuple[str, str]]]] = {}
    for name, (base, fields, where) in sorted(catalogue.items()):
        if where in skip_files or base.strip():
            continue
        seen: list[tuple[str, str]] = []
        for ty, member in fields:
            if ty.endswith("*") or ty.endswith("&") or ty == name:
                continue              # a pointer occupies no base storage
            if ty not in catalogue:
                continue
            if any(ty == t for t, _m in seen):
                continue              # an array of members, not a second base
            seen.append((ty, member))
        if seen:
            embedded[name] = (where, seen)

    hits.sort(key=lambda h: -h[0])
    for shared, name, where, other, other_where, names in hits:
        print(f"  {name} ({where}) repeats {other} ({other_where})'s first "
              f"{shared} field(s)")
        print(f"      {', '.join(n for _ty, n in names)}")
        print(f"      check:  sizeof({other}) + {name}'s own == its pinned size")

    if not hits:
        print("  no baseless class repeats another's leading fields")

    print()
    for name, (where, seen) in sorted(
            embedded.items(), key=lambda kv: -len(kv[1][1])):
        kind = (f"MULTIPLE INHERITANCE, {len(seen)} bases" if len(seen) > 1
                else "one embedded base")
        print(f"  {name} ({where}) has NO base and embeds {kind}:")
        for ty, member in seen:
            print(f"      {ty:16} as `{member}`")
    if not embedded:
        print("  no class embeds by value a type this tree derives from "
              "elsewhere")
    print(f"\n{len(catalogue):,} class(es) read from {len(list(SRC.glob('*.h'))):,} "
          f"header(s); {len(hits)} name-prefix candidate(s) at prefix >= "
          f"{minimum}, {len(embedded)} embedded-base candidate(s)")
