"""Actual header declarations must expose receiver-free message entry types.

Host type probes reject hidden receivers and argument/return substitutions.
They do not prove Windows ABI widths, original body semantics or game acceptance;
native compiler symbols and pinned stack contracts provide separate evidence.
"""
from pathlib import Path
import re

import pytest

from test_texture_sub61f0d0_actual_source import build_and_run

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/win.h").is_file())
NAMES = ("OnKey", "OnSysCommand", "OnQueryNewPalette", "OnActivate")


def unit(member=None):
    header = (ROOT / "src/win.h").read_text()
    declarations = []
    for name in NAMES:
        matches = re.findall(r"^\s*([^\n;]*\b" + name + r"\([^;]*\);)", header, re.M)
        assert len(matches) == 1, name
        declaration = matches[0]
        if name == member:
            assert "static " in declaration
            declaration = declaration.replace("static ", "", 1)
        declarations.append(declaration)
    return r"""
#include <cassert>
#define __cdecl
struct HWND__;
typedef HWND__ *HWND;
class Win { public:
DECLARATIONS
};
typedef void (*KeyEntry)(void *, unsigned int, long, int, unsigned int);
typedef int (*CommandEntry)(HWND, unsigned int, int, int);
typedef int (*PaletteEntry)(void *);
typedef long (*ActivationEntry)(void *, unsigned int, void *, long);
char probe(KeyEntry);
char probe(CommandEntry);
char probe(PaletteEntry);
char probe(ActivationEntry);
template<class T> long probe(T);
int main() {
    assert(sizeof(probe(&Win::OnKey)) == sizeof(char));
    assert(sizeof(probe(&Win::OnSysCommand)) == sizeof(char));
    assert(sizeof(probe(&Win::OnQueryNewPalette)) == sizeof(char));
    assert(sizeof(probe(&Win::OnActivate)) == sizeof(char));
}
""".replace("DECLARATIONS", "\n".join(declarations))


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_message_entry_function_pointer_types(compiler):
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("name", NAMES)
def test_rejects_hidden_receiver(compiler, name):
    build_and_run(unit(name), compiler, expect_success=False)
