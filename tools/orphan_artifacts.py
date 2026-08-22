#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Addresses claimed by BOTH product source and a leftover artifact.

Promoting a body into `src/` is supposed to delete the artifact it came from -
`src/recovered/`, `src/recovered/units/` or `src/unrecovered/`. Hundreds were
deleted correctly. When one is missed the catalogue holds TWO markers for one
address, and every `osmx measure`/`record` on it then refuses as ambiguous
until someone passes `--in`.

The gate does not report this: its duplicate check does not look across the
product/artifact boundary.

    uv run tools/orphan_artifacts.py

NOT EVERY HIT IS A MISTAKE. The common legitimate case is a body that lives in
a HEADER - the marker in the .cpp carries a `// body` fact pointing there, the
function is inlined away, and no out-of-line symbol exists for `measure` to
find. The artifact is then the only thing that can carry the claim at all.
Those are reported separately, because deleting one loses the proof.
"""
import collections
import pathlib
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_ROOT))

from decomp.reader import read  # noqa: E402

ARTIFACT = ("/recovered/", "/unrecovered/")


def is_artifact(record) -> bool:
    return any(part in str(record.path) for part in ARTIFACT)


if __name__ == "__main__":
    seen = collections.defaultdict(list)
    for record in read(REPO_ROOT / "src"):
        seen[record.address].append(record)

    stale, header_backed = [], []
    for address, records in sorted(seen.items()):
        artifacts = [r for r in records if is_artifact(r)]
        product = [r for r in records if not is_artifact(r)]
        if not artifacts or not product:
            continue
        # `body` names a file when the definition is not beside the marker.
        in_header = any(str(getattr(r, "body", "") or "").endswith(".h")
                        for r in product)
        (header_backed if in_header else stale).append((address, product,
                                                        artifacts))

    for address, product, artifacts in stale:
        print(f"  STALE   0x{address:08X}  {product[0].name}")
        print(f"          product  {product[0].path.name}")
        for r in artifacts:
            tier = "BYTE_EXACT" if r.byte_exact else "no claim"
            print(f"          artifact {r.path.name}  ({tier}) - delete this")
    for address, product, artifacts in header_backed:
        print(f"  header  0x{address:08X}  {product[0].name}")
        print(f"          body is in a header, so no out-of-line symbol "
              f"exists and only the artifact can carry the claim")

    print(f"\n{len(stale)} stale artifact(s) to delete, "
          f"{len(header_backed)} legitimately duplicated because the body "
          f"lives in a header")
    sys.exit(1 if stale else 0)
