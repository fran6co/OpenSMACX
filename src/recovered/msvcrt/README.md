# The Microsoft C runtime, recovered but never buildable

Seven catalogued functions in `terranx.exe` are not game code and were never
anybody's to write here: they are the C runtime VC6 links into every program
it builds. `docs/recovery/functions.csv` says so itself - each is
`kind library` with `lib` among its flags - and that flag is what selects
this directory, rather than a judgement about the name.

| | |
|---|---|
| `??2@YAPAXI@Z` | `operator new` |
| `??3@YAXPAX@Z` | `operator delete` |
| `_fgetc` `_fopen` `_fputc` | stdio |
| `_free` | the small-block heap |
| `_srand` | |

## Why they are not in the build, and cannot be

`OpenSMACX.exe` links the same C runtime. It already defines every one of
these, so a second definition is `LNK2005` - not a rule someone has to
remember, the linker enforces it. There is no arrangement of this tree that
puts `_free` in `OPENSMACX_SOURCES` and still links.

That is the difference between this directory and `src/recovered/` above it.
A body there has no owner YET; choosing a file for it is a recovery decision
nobody has made. A body here has no owner EVER.

## So what is a byte-exact claim worth here

It is evidence about the toolchain, not source to build.

`_free` at 0x00644EF2 compiles byte-for-byte from `free(void *)` under
`/c /O2 /Gy /GR- /GX`, calling `__lock`, `___sbh_find_block`,
`___sbh_free_block` and `__unlock` in that order. That is a statement that
the CRT this project links IS the CRT the shipped image was linked against -
same version, same build, same small-block-heap implementation. Nothing else
in the tree tests that, and it is worth knowing before a mismatch somewhere
else gets blamed on a recovered body.

## What is wrong with these files, and why it is not fixed here

They are the emitter's verification dialect, and it shows worst in the
callees. `00644ef2.cpp` declares

    extern "C" void *__cdecl sbh_find_block_impl(void *);
    extern "C" void __cdecl lock_impl(int);

and says why: the scaffold emits NULLARY declarations for unmangled
C-linkage names, so the body wrote typed shadows beside it under invented
names. The real symbols are `___sbh_find_block` and `__lock` - CRT internals
this project links but no header declares.

Rewriting them is not worth doing. A file that cannot be compiled cannot be
checked, so the rewrite would be unverifiable by construction, and the one
thing these files are for - the byte-exact claim - is already measured
against the addresses rather than the spelling.
