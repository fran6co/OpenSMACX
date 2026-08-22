#!/usr/bin/env -S uv run python
"""Adjustor thunks VC6 emits, matched against the claims that hand-write them.

An adjustor thunk shifts `this` to a subobject and tail-jumps to the real body:
`sub ecx, 0x444; jmp ...` for a fixed offset, `sub ecx, [ecx-4]; jmp ...` for a
vtordisp. The compiler writes them, correctly, whenever a class overrides a
virtual reached through a base at a non-zero offset. A hand-written forwarder
cannot reproduce one that takes arguments, because the image tail-JUMPS with the
arguments untouched on the stack and C++ has to re-push them.

TWO THINGS COST THIS TREE 40 CLAIMS, both worth checking for by machine rather
than by eye:

  * THE NAME IN THE ANNOTATION IS NOT THE NAME THE COMPILER EMITS. The census
    reconstructs the scalar deleting destructor `??_G`; VC6 emits the VECTOR
    one, `??_E`. Forty thunks sat in the objects while a grep for `??_G`
    reported none. Same shape as the `QAE` receivers this tree already knows
    are invented - a reconstruction is a guess, and only the object file is
    evidence.
  * THE CLAIM HAS TO LIVE WHERE THE COMDAT IS EMITTED. MSVC emits a class's
    vtable and its thunks in EVERY translation unit that CONSTRUCTS the object,
    not in one owning TU. A claim filed with its family instead of with its
    construction measures "not among the object's .text symbols".

So this reports, for every adjustor thunk in the build, which source file emits
it and which annotation should name it.

    uv run tools/compiler_thunks.py            # candidates worth repointing
    uv run tools/compiler_thunks.py --all      # every emitted thunk
"""
import argparse, collections, pathlib, re, subprocess, sys

REPO = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from decomp.asm import text_symbols
OBJS = REPO / "build/CMakeFiles/OpenSMACX.dir/src"
THUNK = re.compile(
    rb'\?[?$A-Za-z_0-9]{1,40}@(?:[A-Za-z_0-9]+@)?@'
    rb'(?:\$4[A-Z@]+A@|\$R4[A-Z@]+@|W[A-Z]+@)[A-Z][A-Z0-9@$_]{0,40}Z')
MARKER = re.compile(
    r"^// ORIGINAL: (0x[0-9A-F]{8}) (\S+) (\S+)(?: (BYTE_EXACT|SEMANTIC))?\s*$", re.M)


def emitted():
    """symbol -> the source files whose object DEFINES it in .text.

    Reading the object's symbol table, not grepping its bytes. A translation
    unit that CALLS a thunk carries the mangled name too, as an external
    reference, so string presence proved nothing - nine claims were repointed
    at symbols that were merely referenced and every one measured "not among
    the object's .text symbols".
    """
    out = collections.defaultdict(set)
    if not OBJS.is_dir():
        return out
    for obj in sorted(OBJS.glob("*.obj")):
        try:
            names = {n for n, _value, _sec in text_symbols(obj.read_bytes())}
        except Exception:
            continue
        for name in names:
            if THUNK.fullmatch(name.encode()):
                out[name].add(obj.name.replace(".cpp.obj", ".cpp"))
    return out


def wanted(mangled):
    """The symbols an annotation's reconstructed name could really be.

    `??_G` is the scalar deleting destructor and `??_E` the vector one; the
    census guesses the first and VC6 writes the second. `thunk1_<Class>` is
    this tree's own name for a vtordisp adjustor, whose real spelling puts the
    class first and the adjustment in a `$4` prefix.
    """
    out = [mangled]
    if mangled.startswith("??_G"):
        out.append("??_E" + mangled[4:])
    m = re.match(r"\?(\w+)@thunk\d+_(\w+)@@QAE(.*)$", mangled)
    if m:
        method, cls, tail = m.groups()
        out.append(f"?{method}@{cls}@@$4PPPPPPPM@A@AE{tail}")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true", help="list every emitted thunk")
    ap.add_argument("--fix", action="store_true",
                    help="rewrite the `// symbol` fact of every claim that can "
                         "be named where it already is")
    args = ap.parse_args()

    have = emitted()
    if not have:
        print(f"SKIP: no objects under {OBJS.relative_to(REPO)} - build first")
        return 0
    if args.all:
        for sym in sorted(have):
            print(f"  {sym}\n      emitted by {', '.join(sorted(have[sym]))}")
        print(f"\n{len(have)} adjustor thunk(s) emitted")
        return 0

    rows = []
    for path in sorted(REPO.joinpath("src").rglob("*.cpp")):
        if "recovered" in path.parts or "unrecovered" in path.parts:
            continue
        text = path.read_text(errors="replace")
        here = path.name
        for addr, mangled, _spans, tier in MARKER.findall(text):
            if tier:                       # already proved; leave it alone
                continue
            for candidate in wanted(mangled):
                if candidate in have:
                    files = have[candidate]
                    rows.append((addr, here, sorted(files), candidate,
                                 here in files))
                    break

    ready = [r for r in rows if r[4]]
    if args.fix:
        n = 0
        # THE OBJECT FILES ARE A PREFILTER, NOT PROOF. They are built with the
        # project's flags; `osmx measure` compiles with the MATCH flag sets,
        # and a thunk's COMDAT is not emitted under all of them. Nine claims
        # were repointed at symbols the Debug object defines and the match
        # build does not, and every one measured "not among the object's .text
        # symbols". So each rewrite is measured, and reverted if it does not
        # hold.
        for addr, here, _files, sym, in_place in rows:
            if not in_place:
                continue
            path = next(q for q in REPO.joinpath("src").rglob(here))
            lines = path.read_text().splitlines(keepends=True)
            for i, line in enumerate(lines):
                if line.startswith(f"// ORIGINAL: {addr} "):
                    for j in range(i + 1, min(i + 5, len(lines))):
                        if lines[j].startswith("// symbol"):
                            lines[j] = f"// symbol    {sym}\n"
                            n += 1
                            break
                    else:                      # no symbol fact yet; add one
                        lines.insert(i + 1, f"// symbol    {sym}\n")
                        n += 1
                    break
            before = path.read_text()
            path.write_text("".join(lines))
            proof = subprocess.run(
                [sys.executable, str(REPO / "tools" / "osmx.py"), "measure", addr],
                cwd=REPO, capture_output=True, text=True)
            if "BYTE_EXACT" not in proof.stdout:
                path.write_text(before)
                n -= 1
                print(f"  {addr} {sym}: does not hold under the match flags, "
                      f"left alone")
        print(f"named {n} claim(s) after the compiler's own symbol")
        return 0
    moves = [r for r in rows if not r[4]]
    for addr, here, files, sym, _ in ready:
        print(f"  {addr}  {here}: name it {sym}")
    for addr, here, files, sym, _ in moves:
        print(f"  {addr}  {here} -> {files[0]}: name it {sym}")
    print(f"\n{len(rows)} unproved claim(s) have a compiler-emitted thunk; "
          f"{len(ready)} can be named where they are, {len(moves)} need moving "
          f"to the TU that emits the COMDAT")
    return 0


if __name__ == "__main__":
    sys.exit(main())
