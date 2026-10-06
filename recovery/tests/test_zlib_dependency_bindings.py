"""Installed zlib dependency bindings and retired-seed continuity controls.

It reads only the checkout's installed source annotations and public recovery
APIs; it does not import the guarded emitter or a generated proposal fixture.
The assertions inspect the actual integrated source tree.
"""
from __future__ import annotations

from pathlib import Path

import pytest

from match_recovery.component_original_routing import exact_component_record_bindings
from match_recovery.component_source import parse_component
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.source.reader import read_text
from match_recovery.source_inventory import source_snapshot


def project_root() -> Path:
    for path in Path(__file__).resolve().parents:
        if (path / "recovery/project.json").is_file() and (path / "src/IMAGE_PIN").is_file():
            return path
    raise RuntimeError("project root not found")


ROOT = project_root()
IMAGE_SHA = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
MANIFEST_PATH = "src/vendor/zlib.recovery.h"
MOVED = {
    0x0063CF00: (0x0063CF25, "deflateInit_", "_deflateInit_", "deflate.c", True),
    0x0063D4D0: (0x0063D55F, "_deflateEnd", "_deflateEnd", "deflate.c", True),
    0x0063DF90: (0x0063DFDD, "_inflateEnd", "_inflateEnd", "inflate.c", True),
    0x0063E0F0: (0x0063E10A, "_inflateInit_", "_inflateInit_", "inflate.c", True),
    0x00644100: (0x0064413F, "_inflate_trees_free", "_inflate_trees_free", "inftrees.c", False),
    0x00644140: (0x00644179, "_inflate_codes_new", "_inflate_codes_new", "infcodes.c", False),
    0x0063D450: (0x0063D4C2, "_flush_pending", "_flush_pending", "deflate.c", True),
}


def test_installed_source_has_one_intersectrect_import_owner():
    matches = [binding for binding in read_imports(ROOT)
               if binding.symbol == "__imp__IntersectRect@12"]
    assert len(matches) == 1
    binding, = matches
    assert (binding.module, binding.name, binding.path) == (
        "USER32.dll", "IntersectRect", "src/graphicwin.cpp")


def test_integrated_companion_contains_each_original_and_exact_vendor_binding():
    companion = ROOT / MANIFEST_PATH
    text = companion.read_text(encoding="utf-8")
    manifest = parse_component(text)
    records = {record.address: record for record in read_text(text, companion)}
    assert "callback at +0x24 is a direct one-indirection function pointer, not a vtable" in text
    assert "allocator/context at +0x20/+0x28" in text
    assert "prior byte-register mismatch remains historical only" in text
    bindings = {binding.origin.address: binding for binding in manifest.bindings}
    snapshot = source_snapshot(ROOT)
    routed = exact_component_record_bindings(snapshot)
    routed_by_address = {}
    for (_, entity_id), (component, _manifest, binding) in routed.items():
        routed_by_address[binding.origin.address] = (entity_id, component, binding)

    for address, (end, name, symbol, unit, byte_exact) in MOVED.items():
        record = records[address]
        assert record.name == name
        assert record.image_spans == ((address, end),)
        assert record.byte_exact is byte_exact
        assert address in bindings and address in routed_by_address
        binding = bindings[address]
        assert binding.origin == Origin(IMAGE_SHA, "pe:va", address)
        assert binding.spans == (Span(address, end),)
        assert (binding.symbol, binding.unit) == (symbol, unit)
        assert routed_by_address[address][1] == "zlib"
        assert routed_by_address[address][2] == binding

    record = records[0x0063D450]
    assert record.name == "_flush_pending"
    assert record.image_spans == ((0x0063D450, 0x0063D4C2),)
    assert record.byte_exact and record.levers
    assert "old-symbol=_sub_63d450" in text
    assert not (ROOT / "src/unrecovered/0063d450.cpp").exists()

    for relative in (
        "src/recovered/0063cf00.cpp", "src/unrecovered/0063d4d0.cpp",
        "src/recovered/0063df90.cpp", "src/recovered/0063e0f0.cpp",
        "src/recovered/units/00644100.cpp", "src/recovered/units/00644140.cpp",
        "src/unrecovered/0063d450.cpp",
    ):
        assert not (ROOT / relative).exists()
