"""Exact-block ownership migration controls, not complete runtime acceptance."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BASE = "767e838f6e7875f0a480fd19453684c3a1d3b9b9"
PUBLICATION = "8593c5ff0d777b55f72a28196385f1e7f44505a7"
INITIALIZERS = {0x402F10, 0x445450, 0x445500, 0x445530, 0x445560,
                0x459590, 0x477E40, 0x584D00, 0x5AE0F0, 0x5B9C10}


def original(name):
    return subprocess.check_output(["git", "show", BASE + ":" + name], cwd=ROOT, text=True)


def publication(name):
    return subprocess.check_output(["git", "show", PUBLICATION + ":" + name], cwd=ROOT, text=True)


def without_new_claims(source):
    # Only the native recorder may add claims; this control permits precisely
    # the ten initializer claims while protecting all other source bytes.
    lines = source.splitlines(keepends=True)
    for i, line in enumerate(lines):
        found = re.match(r"// ORIGINAL: (0x[0-9A-Fa-f]+)", line)
        if found and int(found[1], 16) in INITIALIZERS:
            lines[i] = line.replace(" BYTE_EXACT", "")
    return "".join(lines)


def test_ownership_moves_preserve_complete_bodies_annotations_and_storage_order():
    old = original("src/wave.cpp")
    ranges = [
        (old.index('/*\nPurpose: Initialise an Effect wave.'), old.index('// ===== MANAGED GLOBALS')),
        (old.index('// The signed pitch cycle'), old.index("// The window procedure's callback")),
        (old.index("// The window procedure's callback"), old.index('// RECOVERY_OBJECT: v1 0x0074D9D0')),
        (old.index('// The button hover hook'), old.index('// These start-only identities')),
    ]
    blocks = [old[a:b] for a, b in ranges]
    remaining = old
    for a, b in reversed(ranges):
        remaining = remaining[:a] + remaining[b:]
    # Audit the completed move at its checkpoint; later verified lifetime
    # work may change surrounding code. Keep the moved bodies live-checked.
    assert without_new_claims(publication('src/wave.cpp')) == without_new_claims(remaining)
    for name, additions in [('src/fx.cpp', [blocks[0], blocks[1], blocks[3]]),
                            ('src/ambience.cpp', [blocks[2]])]:
        expected = original(name)
        current = publication(name)
        # Definition placement within a TU is immaterial here; preserve every
        # moved block exactly and all surrounding pre-existing source in order.
        for block in additions:
            assert current.count(block.rstrip()) == 1
            assert (ROOT / name).read_text().count(block.rstrip()) == 1
            current = current.replace(block.rstrip(), '', 1)
        if name == 'src/fx.cpp':
            expected = expected.replace('#include "fx.h"\n', '#include "fx.h"\n#include "win.h"\n')
        assert re.sub(r'\n\s*\n', '\n', current).strip() == re.sub(r'\n\s*\n', '\n', expected).strip()


def test_effect_callback_callers_remain_real_source_dependencies():
    assert 'ok_callback();' in (ROOT / 'src/basewin.cpp').read_text()
    assert 'WinButtonHoverHook' in (ROOT / 'src/flatbutton.cpp').read_text()
    fx = (ROOT / 'src/fx.cpp').read_text()
    assert fx.index('WinMessageHook = input_callback;') < fx.index('WinOkHook = ok_callback;')
    assert fx.index('WinOkHook = ok_callback;') < fx.index('WinButtonHoverHook = button_hover_callback;')
