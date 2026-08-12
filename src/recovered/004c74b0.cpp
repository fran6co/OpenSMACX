// ORIGINAL: 0x004C74B0 BYTE_EXACT
// name      sub_4c74b0
// size      64 bytes
// spans     0x004C74B0-0x004C74F0
// prototype 
// callers   0   call targets   4
// 0x004C74B0  sub_4c74b0  ->  _sub_4c74b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int _strcpy();  // arity unknown
int __cdecl _strlen();
int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

// `ecx` is read with no stack access before the stack parameter is ever
// touched, and `ret 4` pops exactly one stack slot: this is a __thiscall
// member taking the one explicit `char *` argument, not a free `__stdcall`
// function.
//
// `strlen`/`strcpy` (the declfix-respelled names) are VC6 intrinsics: naming
// them that way gets the call INLINED into a `repne scasb`/`rep movsd`
// expansion the original does not have here. Spelling them `_strlen`/
// `_strcpy` (a plain, non-"recognised" identifier - precedent in
// src/recovered/005bf350.cpp) keeps them real calls.
//
// Binding `this`/`name` to a named local flips the esi/edi assignment
// (both ways tried: named locals for both, named locals for just one,
// `register` hints) relative to the original's esi=this/edi=name pairing.
// Only inlining every access as a fresh `reinterpret_cast<char*>(this)`
// expression - no local at all - reproduces it.
int __cdecl _strlen(const char *);
char *__cdecl _strcpy(char *, const char *);
void __cdecl fn_0064557f(void *);
void *__cdecl fn_0064558a(unsigned int);

class Unknown_4c74b0 { public:
    void set_name(char *name);
};

void Unknown_4c74b0::set_name(char *name) {
    if (*reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200) != 0) {
        fn_0064557f(*reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200));
    }
    void *newbuf = fn_0064558a(_strlen(name) + 1);
    *reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x200) = newbuf;
    _strcpy(reinterpret_cast<char *>(newbuf), name);
}
