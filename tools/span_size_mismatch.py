#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Markers whose spans do not add up to the size they claim.

For a SINGLE-SPAN record, the marker's span and its `// size` fact are two
statements about the same bytes and they have to agree. 5,513 of them do; 2
do not. That is a real invariant, and it is worth checking because when a
span is short the body CANNOT MATCH for any reason the source can fix.

Multi-span records are deliberately out of scope: 398 of them have a primary
span differing from their size fact, so `// size` means something else there,
and checking them reports hundreds of false positives.

When they disagree the body CANNOT MATCH, and not for any reason the source
can fix - the question being asked is the wrong one.

HONEST PROVENANCE: this check was written after an agent reported fixing five
markers whose spans were "one or two bytes short of start + size". THAT CLAIM
DID NOT CHECK OUT. Comparing the markers before and after its commit,
0x0063B910's span is 0x0063B910-0x0063B92D both times, 29 bytes, against a
size fact of 29 - they agreed all along, and the four SpriteBox handlers are
the same. Whatever made those bodies land, it was not a span correction.

That was true until 2026-08-22, WHEN IT CAUGHT ITS FIRST. Promoting
close_palette_class out of src/recovered/units/ transcribed its span as
0x005FECF0-0x005FED0A where the artifact said 0x005FED0B - one byte, the
trailing `ret`. The body then measured 6 of 6 instructions AGREEING and
MISMATCH overall, because it was being compared against a truncated extent: a
short span passes every instruction it bothers to compare, so nothing in the
result says the extent is wrong. This reported it at once, as "spans cover 26b,
`// size` says 27b - short by 1".

RUN IT AFTER ANY PROMOTION. Twelve more artifact-only bodies are called by
compiled code, and copying a span by hand is the step that goes wrong quietly.

It is kept because the invariant is real and cheap - 5,513 single-span records hold it and 2 do not - and because
a short span is a failure mode that no amount of work on the C++ can fix. The
2 it does report are both already BYTE_EXACT, which means their SIZE FACT is
off by one rather than their span.

    uv run tools/span_size_mismatch.py

A DISAGREEMENT IS NOT AUTOMATICALLY A DEFECT, and the check says which is
which. `Time::init_class` and `Palette::set_from_dib` are both one byte short
and both already BYTE_EXACT at 5/5 and 40/40 - there the SIZE FACT is wrong,
not the span, and "fixing" the span would break a working claim. What matters
is a disagreement on a body that does NOT reproduce: then the question being
asked may be the wrong one. Those are listed first and marked SUSPECT.
"""
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402

SIZE = re.compile(r"^//\s*size\s+([\d,]+)\s*bytes", re.M)


def declared_size(record) -> int | None:
    """The `// size` fact beside this marker, if it carries one."""
    try:
        text = record.path.read_text(errors="replace")
    except OSError:
        return None
    # The facts sit immediately under the marker; read a small window rather
    # than the file, so a later body's `// size` cannot be picked up here.
    lines = text.splitlines()
    window = "\n".join(lines[record.line - 1: record.line + 12])
    found = SIZE.search(window)
    return int(found.group(1).replace(",", "")) if found else None


if __name__ == "__main__":
    rows, checked = [], 0
    for record in read(REPO_ROOT / "src"):
        if not record.image_spans:
            continue
        size = declared_size(record)
        if size is None:
            continue
        checked += 1
        # SINGLE-SPAN RECORDS ONLY, and that restriction is the whole check.
        # Measured across the tree: 5,513 single-span records have span ==
        # size and 2 do not, which is a real invariant. Multi-span records do
        # not follow it at all - 398 of them have a primary span differing
        # from their size fact - so whatever `// size` means there, it is not
        # this, and including them reported hundreds of false positives.
        if len(record.image_spans) != 1:
            continue
        low, high = record.image_spans[0]
        spanned = high - low
        if spanned != size:
            # A body that already reproduces is evidence the SPAN is right and
            # the size fact is not; only a body that does not reproduce can be
            # failing BECAUSE of its span.
            rows.append((not record.byte_exact, abs(spanned - size), record,
                         spanned, size))

    rows.sort(key=lambda r: (not r[0], -r[1]))
    for suspect, delta, record, spanned, size in rows:
        where = record.path.relative_to(REPO_ROOT / "src")
        sign = "short by" if spanned < size else "over by"
        mark = "SUSPECT " if suspect else "ok      "
        print(f"  {mark}{record.address_hex}  spans cover {spanned:,}b, "
              f"`// size` says {size:,}b - {sign} {delta}")
        print(f"           {where}:{record.line}  {record.name}")
    suspects = sum(1 for r in rows if r[0])
    print(f"\n{len(rows)} marker(s) disagree with their own size fact, of "
          f"{checked:,} checked; {suspects} on a body that does NOT reproduce")
    print("a disagreement on a body that already reproduces means the SIZE "
          "fact is wrong, not the span - do not 'fix' those")
    sys.exit(1 if suspects else 0)
