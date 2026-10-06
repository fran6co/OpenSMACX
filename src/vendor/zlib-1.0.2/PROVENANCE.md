# zlib 1.0.2, vendored verbatim

`terranx.exe` statically links zlib, and it says which version itself:

```
0x00697F28  '1.0.2'
0x00698CB8  ' deflate 1.0.2 Copyright 1995-1996 Jean-loup Gailly '
0x006A5A60  ' inflate 1.0.2 Copyright 1995-1996 Mark Adler '
```

These functions are not application code to reconstruct by hand. They are
upstream code to reuse and verify under the license notice in `zlib.h`; their
recovery is provenance and integration rather than authorship.

| | |
|---|---|
| source | <https://zlib.net/fossils/zlib-1.0.2.tar.gz> |
| sha256 | `517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6` |
| fetched | 2026-08-14 |

## These files are UNMODIFIED

Every file here is byte-identical to the one in that tarball. Nothing is
reformatted, no annotation marker is inserted, no `#include` is rewritten.
Verify the upstream files against the SHA-256-pinned archive when changing this
dependency. Source provenance is not a binary-equivalence verdict.

## It is BUILT, and it is not a recovery

`CMakeLists.txt` compiles these sources as a `zlib` static library and links
it into `OpenSMACX`. That is the whole relationship: zlib is a dependency of
this program exactly as it was a dependency of `terranx.exe`.

Until 2026-08-16 there were thirty-nine `recovered_*.c` files beside these,
each an annotation header over `#include "deflate.c"`, each carrying a
BYTE_EXACT claim. They were counted as recoveries and they were not
recoveries: nobody here wrote a line of them. `docs/EXCLUSIONS.md` section 1
makes the same argument about the MSVC 6 CRT - code that can be compiled or
linked is not code to rewrite - and seven CRT reimplementations were deleted
the same day for the same reason. The claim count fell 1595 -> 1556, which
is the honest number.

What was lost with them was a real check: those thirty-nine claims said that
this release, compiled with these flags, reproduces the image byte for byte,
which is how the version and the flags were established in the first place.
The former per-function companion is retired along with the wrappers. Git keeps
the original mappings and experiments; they do not belong in application-function
coverage. Keep release provenance and the actual build requirements here and in
CMake. Dependency equivalence still requires fresh verification; source provenance
alone cannot replace that check.

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

The historically matching flags are `/c /O2 /Gy /GR- /GX`. Adding `/Oy-`
diverged at instruction 0 in the original
thirteen-function experiment.

`local` is `static` in this release. Those internal functions must also be
selected by name from STATIC-class object symbols; `/Gy` gives them their own
COMDAT sections. Public entry points retain external symbols.
