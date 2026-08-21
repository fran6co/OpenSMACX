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


def _declaration(name: str) -> re.Pattern:
    return re.compile(rf"^extern\s+[\w:]+\s*\*+\s*{re.escape(name)}\s*;"
                      rf"(?P<trailing>[^\n]*)\n", re.M)


def main(apply: bool) -> int:
    files = {path: path.read_text()
             for path in sorted(SRC.glob("*.[ch]*"))
             if path.suffix in (".c", ".h", ".cpp", ".hpp")}
    whole = "\n".join(files.values())
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
            if len(writes) > 1 or re.search(rf"&\s*{re.escape(name)}\b", whole):
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


if __name__ == "__main__":
    count = main("--apply" in sys.argv)
    print(f"{count} binding(s) "
          f"{'folded' if '--apply' in sys.argv else 'foldable'}")
