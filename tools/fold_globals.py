#!/usr/bin/env -S uv run python
"""Turn `T *X = (T *)0x...;` pointer BINDINGS into folded constants.

WHY. The tree names the image's globals by binding a pointer to their address:

    Sprite *g_CHECKBOX_SPRITE_1 = (Sprite *)0x009B8F60;   // in a .cpp
    extern Sprite *g_CHECKBOX_SPRITE_1;                   // in the header

Every use then compiles `mov ecx, dword ptr [g_X]` - a load from a slot the
shipped image never reads, where the image has `mov ecx, 0x9b8f60`. So the
binding costs its callers the ratchet, exactly as a function-pointer seam does.

THE FIX IS ONE WORD. A namespace-scope `const` pointer has internal linkage in
C++, so moving the DEFINITION into the header and marking it `const` lets every
translation unit fold it to the immediate:

    Sprite *const g_CHECKBOX_SPRITE_1 = (Sprite *)0x009B8F60;   // in the header

Nothing at the call sites changes, and no cast moves - the one that was already
there just stops being a variable's initialiser and becomes a constant's.

WHAT IT REFUSES. A binding is only folded when the tree cannot write through
the variable itself: any `X =` outside its own definition, or an `&X`, means
the slot is real storage and the fold would change what the program does.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"

BINDING = re.compile(
    r"^(?P<type>[A-Za-z_]\w*(?:\s*::\s*\w+)*)\s*(?P<stars>\*+)\s*"
    r"(?P<name>\w+)\s*=\s*"
    r"(?:\((?P<cast>[^)]*)\)|reinterpret_cast<(?P<rcast>[^>]*)>\()"
    r"\s*(?P<address>0x[0-9A-Fa-f]+)\s*\)?\s*;[^\n]*\n", re.M)


def _code(text: str) -> str:
    """`text` with comments blanked.

    Every prose block in this tree names what it discusses, so a comment
    explaining why `&AltNatural` was wrong is enough to make the
    address-taken test refuse AltNatural forever.
    """
    return re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)


def _declaration(name: str) -> re.Pattern:
    return re.compile(rf"^extern\s+[\w:]+\s*\*+\s*{re.escape(name)}\s*;"
                      rf"(?P<trailing>[^\n]*)\n", re.M)


def main(apply: bool) -> int:
    files = {path: path.read_text()
             for path in sorted(SRC.glob("*.[ch]*"))
             if path.suffix in (".c", ".h", ".cpp", ".hpp")}
    whole = _code("\n".join(files.values()))
    folded = 0
    for path, text in list(files.items()):
        if path.suffix != ".cpp":
            continue
        for match in list(BINDING.finditer(text)):
            name = match.group("name")
            header = next((h for h, body in files.items()
                           if h.suffix == ".h" and _declaration(name).search(body)),
                          None)
            if header is None:
                continue
            # WRITTEN THROUGH, or its address taken: the slot is storage.
            # `*X = v` writes the POINTEE, which a folded constant still
            # allows; only `X = v` writes the variable. Missing that
            # distinction refused every flag byte the teardowns set.
            writes = re.findall(
                rf"(?<![=!<>*])\s*\b{re.escape(name)}\s*=(?!=)", whole)
            writes = [w for w in writes if not w.lstrip().startswith("*")]
            # `&X->member` takes the MEMBER's address, which a folded
            # constant still allows; only a bare `&X` needs storage. Missing
            # that refused SecretProject, and with it every `base_project`
            # caller.
            taken = re.search(
                rf"&\s*{re.escape(name)}\b\s*(?![-.\[])", whole)
            if len(writes) > 1 or taken:
                print(f"  - {name}: written through or address-taken")
                continue
            stars = match.group("stars")
            definition = (f"{match.group('type')} {stars}const {name} = "
                          f"({match.group('cast') or match.group('rcast')})"
                          f"{match.group('address')};")
            files[header] = _declaration(name).sub(
                lambda m: definition + m.group("trailing") + "\n",
                files[header])
            files[path] = files[path].replace(match.group(0), "")
            text = files[path]
            print(f"  + {name} -> {definition}")
            folded += 1
    if apply:
        for path, body in files.items():
            if body != path.read_text():
                path.write_text(body)
    return folded


LVALUE = re.compile(
    r"^(?P<type>[A-Za-z_]\w*(?:\s*::\s*\w+)*)\s*\*\*const\s+(?P<name>\w+)"
    r"\s*=\s*\([^)]*\)\s*(?P<address>0x[0-9A-Fa-f]+)\s*;[^\n]*\n", re.M)


def lvalues(apply: bool) -> int:
    """Turn `T **const X = (T **)addr` into an lvalue accessor `X()`.

    A DIFFERENT FACT FROM THE ONE ABOVE. `T *const X = (T *)addr` says the
    OBJECT is at that address and folds to an immediate. `T **const X` says a
    POINTER is at it, read at runtime - and reading it through a folded
    constant costs two loads, because VC6 keeps storage for the constant,
    reads that, and dereferences it. The image has one absolute
    `mov ecx, [addr]`, which is what a reference to that address compiles to.

    Only when every use is a dereference: a name also used bare is being
    indexed or passed as a `T **`, and that is a different shape.
    """
    files = {path: path.read_text()
             for path in sorted(SRC.glob("*.[ch]*"))
             if path.suffix in (".c", ".h", ".cpp", ".hpp")}
    changed = 0
    for path, text in list(files.items()):
        if path.suffix != ".h":
            continue
        for match in list(LVALUE.finditer(text)):
            name, kind = match.group("name"), match.group("type")
            whole = _code("\n".join(files.values()))
            bare = len(re.findall(rf"(?<![\w*])\b{re.escape(name)}\b", whole))
            # One bare mention is the declaration itself.
            if bare > 1:
                print(f"  - {name}: used {bare - 1} time(s) without a `*`")
                continue
            accessor = (
                f"inline {kind} *&{name}() {{ return "
                f"*reinterpret_cast<{kind} **>({match.group('address')}); }}\n")
            files[path] = files[path].replace(match.group(0), accessor)
            for other, body in list(files.items()):
                if name not in body:
                    continue
                files[other] = re.sub(rf"\*\s*{re.escape(name)}\b",
                                      f"{name}()", body)
            files[path] = files[path].replace(f"*{name}()", f"{name}()")
            files[path] = files[path].replace(
                f"inline {kind} *&{name}() {{ return "
                f"{name}();", accessor.split("return ")[0] + "return "
                f"*reinterpret_cast<{kind} **>({match.group('address')});")
            text = files[path]
            print(f"  + {name} -> {name}()")
            changed += 1
    if apply:
        for path, body in files.items():
            if body != path.read_text():
                path.write_text(body)
    return changed


if __name__ == "__main__":
    if "--lvalues" in sys.argv:
        count = lvalues("--apply" in sys.argv)
        print(f"{count} pointer slot(s) "
              f"{'converted' if '--apply' in sys.argv else 'convertible'}")
        raise SystemExit(0)
    count = main("--apply" in sys.argv)
    print(f"{count} binding(s) "
          f"{'folded' if '--apply' in sys.argv else 'foldable'}")
