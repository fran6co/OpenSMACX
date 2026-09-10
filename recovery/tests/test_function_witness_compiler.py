"""Real VC6 retention controls on synthetic source, without a product export."""

from pathlib import Path
import sys

import pytest

from match_recovery.bound_objects import FunctionBinding
from match_recovery.function_witness import retained_definition
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff
from recovery.environment.compile import compile_object, compile_overlay
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_compiler_variants import variant_project


@pytest.mark.parametrize("extra_flags", [(), ("/Oy-",)])
def test_real_vc6_retains_unemitted_public_inline_member_privately(variant_project, extra_flags):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source, header = root / "code/unit.cpp", root / "code/widget.h"
    header.write_text("#pragma once\nclass Widget {\npublic:\n"
                      "    int value() { return field_; }\n"
                      "private:\n    int field_;\n};\n")
    source.write_text('#include "widget.h"\nint unrelated() { return 0; }\n')
    original = {source: source.read_bytes(), header: header.read_bytes()}
    symbol = "?value@Widget@@QAEHXZ"
    binding = FunctionBinding(Origin("a" * 64, "pe:va", 0x401000),
                              (Span(0x401000, 0x401003),), symbol, "code/unit.cpp")
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    # The two release variants, with embedded debug evidence for full controls.
    # The wrapper inherits the coordinator-selected VC6_PREFIX; do not lease or
    # substitute another prefix inside this test.
    flags = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Z7", *extra_flags)
    baseline = Coff(compile_object(source, command, flags))
    assert not any(item.name == symbol for item in baseline.symbols.values())
    calls = []

    class Compiler:
        def overlay(self, source, replacements, arguments, flags):
            calls.append(replacements.copy())
            return compile_overlay(root, source, replacements, arguments, flags)

    retained, report = retained_definition(root, binding, "code/widget.h", baseline,
                                          Compiler(), command, flags, "msvc")
    assert report["result"] == "pass", report
    assert report["compile_attempts"] == len(calls) == 5
    assert all(report[key] == "pass" for key in
               ("replica_control", "repeat_control", "target_body_control", "probe_control"))
    assert report["product_emitted"] is False
    assert report["diagnostic_only"] and report["never_link"]
    assert not report["persisted"] and not report["ready_for_collection"]
    assert retained.function(symbol).storage == 2
    assert sum(item.name == symbol for item in retained.symbols.values()) == 1
    # Exercise the ordinary VC6 instantiated-template sink encoding that the
    # synthetic COFF tests cannot establish comes from this actual compiler.
    assert any(item.name.startswith("?match_recovery_retention_")
               and "_keep@@YAXP8Widget@@" in item.name and item.section == 0
               for item in retained.symbols.values())
    original_directives = [section.data for section in baseline.sections if section.name == ".drectve"]
    retained_directives = [section.data for section in retained.sections if section.name == ".drectve"]
    assert retained_directives == original_directives
    assert all(b"/EXPORT:" not in data.upper() for data in retained_directives)
    assert all(path.read_bytes() == contents for path, contents in original.items())
    assert not list((root / "build/recovery/variants").iterdir())
