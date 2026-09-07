"""Real compiler control for typed multiple-inheritance owner queries.

Only synthetic C++/image bytes: no original executable fixtures. This checks
the local compiler protocol and portable verifier, not the game's hierarchy.
"""

import hashlib
from pathlib import Path
import struct
import sys

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from recovery.environment.compile import compile_object
from test_binary import pe_fixture


SOURCE = """
class Owner;
class Primary {
public:
    virtual ~Primary() {}
    virtual Owner *owner() { return 0; }
    int padding[2];
};
class Secondary {
public:
    virtual ~Secondary() {}
    virtual Owner *owner() { return 0; }
};
class Owner : public Primary, public Secondary {
public:
    Owner();
    virtual Owner *owner();
};
Owner::Owner() {}
Owner *Owner::owner() { return this; }
"""


def test_real_vc6_generates_and_verifies_secondary_owner_adjustment(tmp_path, monkeypatch):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    source = tmp_path / "owner.cpp"
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/GR-", "/GX")
    source.write_text(SOURCE)
    obj = Coff(compile_object(source, command, flags))
    owner = "?owner@Owner@@UAEPAV1@XZ"

    def entry(table):
        symbol, = (s for s in obj.symbols.values() if s.name == table)
        section = obj.sections[symbol.section - 1]
        index, = (index for offset, index, kind in section.relocations
                  if offset == 4 and kind == 6)
        return obj.symbols[index].name

    assert entry("??_7Owner@@6BPrimary@@@") == owner
    thunk = entry("??_7Owner@@6BSecondary@@@")
    assert thunk != owner
    assert obj.function(thunk).section != obj.function(owner).section
    code, relocations, _ = obj.subject(obj.function(thunk))
    assert len(relocations) == 1
    assert obj.symbols[int(relocations[0].target)].name == owner

    # Primary has a vptr plus two ints. The independent synthetic image
    # adjusts a Secondary receiver by 12 and jumps to the owner query.
    original = b"\x83\xe9\x0c\xe9" + struct.pack("<i", 0x402000 - 0x401008)
    raw = pe_fixture(original)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    binding = FunctionBinding(Origin(pin, "pe:va", 0x401000),
                              (Span(0x401000, 0x401008),), thunk, "owner.cpp")
    row, = check_objects((binding,), {"owner.cpp": obj}, image,
                         external_targets={owner: 0x402000})
    assert row["result"] == "pass", row
    wrong, = check_objects((binding,), {"owner.cpp": obj}, image,
                           external_targets={owner: 0x403000})
    assert wrong["references"] == "fail"

    # A coupled layout change must be measured, even though the C++ getter
    # remains identical. Rebind the new compiler-owned entry, not its old name.
    source.write_text(SOURCE.replace("padding[2]", "padding[3]"))
    obj = Coff(compile_object(source, command, flags))
    changed = FunctionBinding(binding.origin, binding.spans,
                              entry("??_7Owner@@6BSecondary@@@"), binding.unit)
    row, = check_objects((changed,), {"owner.cpp": obj}, image,
                         external_targets={owner: 0x402000})
    assert row["result"] == "fail", row
