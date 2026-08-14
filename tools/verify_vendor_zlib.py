#!/usr/bin/env python3
"""Hold the vendored zlib to the release it claims to be.

`src/vendor/zlib-1.0.2/` exists so thirteen catalogued functions can be
recovered by COMPILING the code they were built from rather than by
transcribing their disassembly. That argument rests entirely on the files
being the release - unmodified, unreformatted, un-tidied - so the claim is
checked rather than asserted.

WHAT A DRIFT WOULD MEAN. Not a stale copy: a recovery whose provenance note
says "this is zlib 1.0.2" while the bytes say otherwise. Every one of the
thirteen annotations cites this directory as its authorship, so an edit here
silently rewrites what thirteen BYTE_EXACT claims are claiming.

    tools/verify_vendor_zlib.py            # the gate's question
    tools/verify_vendor_zlib.py --record   # re-stamp after a deliberate bump

THE HASHES ARE OF THE FILES, not of the tarball, and that is deliberate. A
tarball hash proves what was downloaded; these prove what is in the tree
right now, which is what the compiler reads. The tarball's own hash is in
PROVENANCE.md for anyone re-fetching it.
"""
from __future__ import annotations

import argparse
import hashlib
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
VENDOR = REPO_ROOT / "src" / "vendor" / "zlib-1.0.2"
PROVENANCE = VENDOR / "PROVENANCE.md"

# The compile recipe these files are recovered with, restated here so a
# change to it has to pass this file's reasoning. See PROVENANCE.md.
FLAGS = "/c /O2 /Gy /GR- /GX"
SUFFIX = ".c"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


# The recovery wrappers are NOT part of the release. Each carries one
# annotation and a single `#include` of the upstream file it claims, and they
# live here rather than in a sibling directory because the compile is handed
# THIS directory - see `byte_match.seed_context`. They are excluded by name so
# that adding a fourteenth recovery does not read as tampering, while any edit
# to an actual release file still does.
WRAPPER = re.compile(r"^recovered_[0-9a-f]{8}\.c$")


def present() -> list:
    if not VENDOR.is_dir():
        return []
    return sorted(p for p in VENDOR.iterdir()
                  if p.suffix in (".c", ".h") and not WRAPPER.match(p.name))


def recorded() -> dict:
    """{name: sha256} parsed out of this file's own table below the marker."""
    text = Path(__file__).read_text()
    block = re.search(r"^# --- HASHES ---\n(.*?)^# --- END ---",
                      text, re.S | re.M)
    if block is None:
        return {}
    return dict(re.findall(r"^#\s+(\S+)\s+([0-9a-f]{64})$", block.group(1),
                           re.M))


def rewrite(hashes: dict) -> None:
    path = Path(__file__)
    text = path.read_text()
    body = "".join(f"#   {name:<14} {value}\n"
                   for name, value in sorted(hashes.items()))
    path.write_text(re.sub(r"^# --- HASHES ---\n.*?^# --- END ---",
                           f"# --- HASHES ---\n{body}# --- END ---",
                           text, flags=re.S | re.M))


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--record", action="store_true",
                        help="re-stamp the table after a deliberate bump")
    arguments = parser.parse_args(argv)

    files = present()
    if not files:
        print(f"REFUSED: {VENDOR} holds no sources, so this compared nothing.",
              file=sys.stderr)
        return 2

    live = {path.name: digest(path) for path in files}
    if arguments.record:
        rewrite(live)
        print(f"recorded {len(live)} file hash(es) from {VENDOR}")
        return 0

    want = recorded()
    if not want:
        print("REFUSED: no hashes are recorded, so this compared nothing. Run "
              "--record once, deliberately, and commit the result.",
              file=sys.stderr)
        return 2

    problems = []
    for name in sorted(set(want) | set(live)):
        if name not in live:
            problems.append(f"{name} is recorded and missing from the tree")
        elif name not in want:
            problems.append(f"{name} is in the tree and not recorded")
        elif want[name] != live[name]:
            problems.append(f"{name} differs from the recorded release")

    # THE PROVENANCE MUST STILL SAY WHAT THE CHECK ASSUMES. A hash table that
    # matches a directory nobody can trace is a checksum, not a provenance.
    note = PROVENANCE.read_text() if PROVENANCE.is_file() else ""
    if "zlib.net/fossils/zlib-1.0.2.tar.gz" not in note:
        problems.append("PROVENANCE.md no longer names the release it came "
                        "from")
    if FLAGS not in note:
        problems.append(f"PROVENANCE.md no longer states the compile flags "
                        f"`{FLAGS}` these are recovered with")

    for line in problems:
        print(f"  {line}", file=sys.stderr)
    if problems:
        print("FAIL: the vendored zlib is not the release it claims to be, and "
              "thirteen BYTE_EXACT claims cite it as their authorship.",
              file=sys.stderr)
        return 1
    print(f"vendored zlib 1.0.2: {len(live)} file(s), all identical to the "
          f"recorded release")
    return 0


# --- HASHES ---
#   deflate.c      ba5534261c7460779d9ad69e35816b12a93c680ae1d7bca37e8702fb394a81c5
#   deflate.h      a9bc4798decd29e8e4a6b39967d4cc1910581003f0039c76e80ec5d933726d9a
#   infblock.c     60397923842507eb1a198144ad9a2d83f05ef0facfc42833eed92ca0cdb9bef0
#   infblock.h     94528229d881020e228924bceaae3ae777f79d1e6994b0c9e1bbddddd47295a4
#   infcodes.c     6d8be743acd8dde57bc8828f978ee0d0331e5767f06b509bfc8ef115b7843308
#   infcodes.h     45763c49e139c90816082d4a6b9a47789552092e79f0ed2f5889aa6ecdd762dd
#   inffast.c      3fe7b52ea58ae6d22ee1e56b89a2a4938642c5b5a34559a855a5b42114ea15cd
#   inffast.h      64ffb16acd1ca5f6ec346e9966a319a681e4a4c11c9aad17069d017ad5f2e85a
#   inflate.c      07ae572683d213b27f4a84963b593182994078445a848b469da0336d82510c0a
#   inftrees.c     9e2318f33ff1050813490ecef44de2824d2ea46bdeac34abde1728d74d99a36d
#   inftrees.h     0f5068077261db7e626f87f61bdb2fe44f48818ff3aa3a0b8dcc0704bee8008a
#   infutil.h      f64b86e0152fa7b5a4f773224a863b5affd97cbd4802dfb745e5da8f7a41440a
#   trees.c        b40b8b101f12e90c8aaa85284fc993f3e588ef59765c8c84cf3a7f21cfb04371
#   zconf.h        d4a7203fb98839b218a85b88624e2899ce811ca85c702645311c5712563dbc58
#   zlib.h         5c3baa1eb8dda799f11a50614d57c55ba13650c880d0be71755486fdc9b122cc
#   zutil.h        ad708e87db545116b8a66130676ce07a32c8e27f41bb91694ecacef814812d30
# --- END ---


if __name__ == "__main__":
    sys.exit(main())
