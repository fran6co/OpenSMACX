"""Conventional C++ recovery smells, not semantic or ABI proofs.

Versioned regex rules preserve the established line-counting lens. These names
are conventions used by decompilers, not requirements on an arbitrary project.
Opt in through audit configuration. A hit requests investigation, not an
automatic rewrite; real placement construction or indirect dispatch may be valid.
"""

import re

# name -> (pattern, include interface files, explanation). No project ceilings.
CPP_RECOVERY_V1 = {
    "vtable": (
        re.compile(r"""(?x)
        (?: \w+ \s* \[ \s* 0x[0-9A-Fa-f]+ \s* / \s* 4 \s* \]
          | \* \s* reinterpret_cast \s* < [^>]*? \* \s* > \s* \([^)]*\)
          | \w+ \s* \[ \s* \d+ \s* \] )
        \s* = \s* [A-Za-z_]\w*Vtable\w* \s* ;""", re.X), False,
        "Possible manual vptr store; investigate genuine construction and compiler-owned virtual dispatch."),
    "construct": (
        re.compile(r"^\w[\w :*&]*\b\w+::construct\(\)\s*\{", re.M), False,
        "A construct() method may be standing in for a C++ constructor; establish lifetime and initialization order."),
    "free-function destructor": (
        re.compile(r"^\w[\w :*&]*\b\w*destructor_redirect\w*\s*\(", re.M), False,
        "A free-function destructor wrapper may conceal real class destruction; verify callback and lifetime requirements."),
    "explicit base construct/destroy": (
        re.compile(r"\b\w+::(?:destroy|construct)\(\)\s*;"), False,
        "An explicit base construct/destroy call may duplicate compiler-generated lifecycle work."),
    "placement new on a subobject": (
        re.compile(r"new\s*\(\s*(?:&|static_cast|reinterpret_cast)"), False,
        "Placement construction of a subobject may duplicate base/member initialization or introduce a null guard."),
    "hand-walked vbtable": (
        re.compile(r"\*reinterpret_cast<(?:const )?int32_t \*(?:const )?\*>\([^)]*\)\)\[[12]\]"), False,
        "A hand-read virtual-base displacement may conceal an undeclared virtual base."),
    "VCall shim": (
        re.compile(r"\bVCall\b"), False,
        "A VCall shim may stand in for a missing virtual method declaration."),
    "ORIGINAL() vtable-slot dispatch": (
        re.compile(r"\bORIGINAL\((?=.*vtable\[)"), False,
        "A hand-read vtable slot may stand in for genuine C++ virtual dispatch."),
    "ORIGINAL() named-pointer seam": (
        re.compile(r"\bORIGINAL\((?!.*vtable\[)"), False,
        "A function-pointer seam may conceal a recoverable direct call; verify the original reference."),
    "vtable-as-member": (
        re.compile(r"\b[A-Za-z_]\w*\s+vtable\w*\s*;"), True,
        "A named vtable field may conceal an undeclared base or missing virtual methods."),
    "vtable-initialiser store": (
        re.compile(r"\w+\s*\(\s*[A-Za-z_]\w*Vtable\w*\s*\)"), True,
        "Possible manual vptr initialization; investigate real bases and virtual methods."),
    "vtable address constant": (
        re.compile(r"\b[A-Za-z_]\w*Vtable\w*\s*=\s*\(?\s*0x"), True,
        "A named vtable address constant may supply manually installed dispatch tables."),
    "short-named fixed-address binding": (
        re.compile(r"\bconst\s+[a-z][a-z0-9]?\s*=\s*\(\s*[\w:\s\*]+\)\s*0x00"), True,
        "A one- or two-letter fixed-address binding hides its meaning; name it from independent evidence."),
    "anonymous fixed-address global": (
        re.compile(r"\bg_00[0-9a-f]{4,6}\b"), True,
        "Address-derived global name; recover its meaning from independent data and use-site evidence."),
}
