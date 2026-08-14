# zlib 1.0.2, vendored verbatim

`terranx.exe` statically links zlib, and it says which version itself:

```
0x00697F28  '1.0.2'
0x00698CB8  ' deflate 1.0.2 Copyright 1995-1996 Jean-loup Gailly '
0x006A5A60  ' inflate 1.0.2 Copyright 1995-1996 Mark Adler '
```

So thirteen catalogued functions in this image are not code anybody here has
to write. They are code somebody else already wrote, published, and put in
the public domain, and the recovery for them is **provenance rather than
authorship**.

| | |
|---|---|
| source | <https://zlib.net/fossils/zlib-1.0.2.tar.gz> |
| sha256 | `517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6` |
| fetched | 2026-08-14 |

## These files are UNMODIFIED

Every file here is byte-identical to the one in that tarball. Nothing is
reformatted, no annotation marker is inserted, no `#include` is rewritten.
`tools/verify_vendor_zlib.py` re-hashes them and fails the gate if one
differs, because the whole value of this directory is that it is the release
and not a copy somebody tidied.

The annotations that claim these functions live in `src/recovered/zlib/`,
beside rather than inside the sources, for the same reason.

## Why it must not be amalgamated

Inlining the local `#include`s to make one self-contained file per function
is the obvious way to avoid needing an include path. It was tried and
**measured**: 12 of the 13 functions stay byte-exact and `_inflate_blocks`
breaks at instruction #1172, `je` against `add`.

`infblock.h`, `inftrees.h` and `infcodes.h` in this release carry no include
guards, so collapsing repeated includes is not the no-op it looks like. A
source transformation that changes the emitted code is precisely what a
matching decompilation must never do, so the sources stay as they are and the
compile is given the directory instead.

## How they are compiled

The unit is the whole `.c` file, written out with its `.c` extension - which
is all it takes for CL to compile as C rather than C++, and these files
*cannot* be compiled as C++: zlib 1.0.2 uses K&R definitions
(`local void build_tree(s, desc) deflate_state *s; ...`).

The flags are `/c /O2 /Gy /GR- /GX`, which is `byte_match.FRAMELESS_FLAGS`,
already one of the project's four. `/Oy-` gives a divergence at instruction 0
on every one of the thirteen.

`local` is `static` in this release, so none of these functions has an
external symbol; they are selected out of the object by name among the
STATIC-class symbols, which `/Gy` still gives their own COMDAT.
