"""Installable continuity control for the five retained zlib identities.

Checks the restored mappings in src/vendor/zlib.recovery.h.
It is deliberately not a claim-grant or compiler
test; component binary and final-link qualification remain separate.
"""

from dataclasses import fields
import base64
import os
from pathlib import Path
import re
import subprocess

from match_recovery.component_original_routing import exact_component_record_bindings
from match_recovery.component_source import parse_component, render_component
from match_recovery.model import Origin
from match_recovery.source import read_file, read_text, write


REPO_ROOT = next(path for path in Path(__file__).resolve().parents if (path / ".git").exists())
ROOT = Path(os.environ.get("OPENSMACX_TEST_ROOT", REPO_ROOT)).resolve()
BASELINE = "62a0a9b6610fdb7be399b01425fb0d99f63f6ade"
HISTORY = {
    0x0063CF00: ("src/recovered/0063cf00.cpp", "deflateInit_", "?deflateInit_@@YAHPAXHPBDH@Z",
                 (0x0063CF00, 0x0063CF25), "_deflateInit_", "deflate.c"),
    0x0063D4D0: ("src/unrecovered/0063d4d0.cpp", "_deflateEnd", "_sub_63d4d0",
                 (0x0063D4D0, 0x0063D55F), "_deflateEnd", "deflate.c"),
    0x0063DF90: ("src/recovered/0063df90.cpp", "_inflateEnd", "_sub_63df90",
                 (0x0063DF90, 0x0063DFDD), "_inflateEnd", "inflate.c"),
    0x0063E0F0: ("src/recovered/0063e0f0.cpp", "_inflateInit_", "?sub_63e0f0@@YAHHHH@Z",
                 (0x0063E0F0, 0x0063E10A), "_inflateInit_", "inflate.c"),
    0x0063D450: ("src/unrecovered/0063d450.cpp", "_flush_pending", "_sub_63d450",
                 (0x0063D450, 0x0063D4C2), "_flush_pending", "deflate.c"),
}
EXISTING = {
    0x00640A80: (0x00640BAE, "_adler32", "adler32.c"),
    0x00644930: (0x00644A62, "_inflate_flush", "infutil.c"),
    0x00642940: (0x0064295F, "_bi_reverse", "trees.c"),
    0x0063D420: (0x0063D44F, "_putShortMSB", "deflate.c"),
}


def git_text(path):
    return subprocess.run(["git", "show", f"{BASELINE}:{path}"], cwd=REPO_ROOT,
                          check=True, capture_output=True, text=True).stdout


def facts(row):
    omitted = {"path", "line", "region", "extract_error", "origin", "state", "recipe", "symbol",
               "source_role", "source_role_reason"}
    rows = tuple((item.name, getattr(row, item.name)) for item in fields(row) if item.name not in omitted)
    lessons = tuple((lesson.kind, lesson.key, lesson.prose,
                     tuple(line for _, line in lesson.lines)) for lesson in (row.origin or ()))
    return rows, lessons


def initial_prefix(text, address):
    """Original seed prefix through its first blank line; no block comment assumed."""
    lines = text.splitlines(keepends=True)
    marker = f"// ORIGINAL: 0x{address:08X} "
    indices = [index for index, line in enumerate(lines) if line.startswith(marker)]
    assert len(indices) == 1
    start = indices[0]
    end = next((index for index in range(start + 1, len(lines)) if not lines[index].strip()), None)
    assert end is not None, hex(address)
    return "".join(lines[:end + 1])


def historical_prefix(block):
    block = re.sub(r"^// symbol\s+(.*)$", r"// Historical emitted symbol: \1",
                   block, flags=re.M)
    return "\n".join(line.rstrip() for line in block.splitlines()) + "\n"


def stored_prefix(text, address):
    lines = text.splitlines()
    marker = f"// ORIGINAL: 0x{address:08X} "
    indices = [index for index, line in enumerate(lines) if line.startswith(marker)]
    assert len(indices) == 1
    start = indices[0]
    end = next((index for index in range(start + 1, len(lines)) if lines[index].strip() == "*/"), None)
    assert end is not None, hex(address)
    return "\n".join(line.rstrip() for line in lines[start:end])


def test_five_full_seed_prefixes_archive_and_native_writer_roundtrip():
    component_path = ROOT / "src/vendor/zlib.recovery.h"
    component_text = component_path.read_text(encoding="utf-8")
    records = {row.address: row for row in read_file(component_path)}
    manifest = parse_component(component_text)
    bindings = {item.origin.address: item for item in manifest.bindings}

    for address, (path, name, old_symbol, span, current_symbol, unit) in HISTORY.items():
        before_text = git_text(path)
        before_rows = read_text(before_text, path)
        before, = (row for row in before_rows if row.address == address)
        after = records[address]
        assert before.name == after.name == name
        assert before.symbol == old_symbol
        assert after.symbol in (None, ""), "do not activate a current symbol before physical observation"
        assert before.image_spans == after.image_spans == (span,)
        assert before.byte_exact is True and after.byte_exact is True
        assert facts(before) == facts(after)
        binding = bindings[address]
        assert binding.symbol == current_symbol and binding.unit == unit
        assert binding.origin.image_sha256 == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
        assert binding.origin.address_space == "pe:va"
        evidence = binding.evidence.lower()
        assert "source" in evidence and "zlib 1.0.2" in evidence
        assert "current coff" in evidence or "coff qualification" in evidence
        assert "coff bytes/boundary/references pass" not in evidence
        expected = historical_prefix(initial_prefix(before_text, address))
        component_normalized = "\n".join(line.rstrip() for line in component_text.splitlines()) + "\n"
        assert expected.rstrip() == stored_prefix(component_text, address).rstrip()
        assert f"// Historical emitted symbol: {old_symbol}" in expected
        assert "// symbol " + current_symbol not in expected
        assert "BYTE_EXACT" in expected
        assert f"Historical seed catalogue: git:{BASELINE}:{path}" in component_text

    for address, (end, symbol, unit) in EXISTING.items():
        binding = bindings[address]
        assert [(span.start, span.end) for span in binding.spans] == [(address, end)]
        assert (binding.symbol, binding.unit) == (symbol, unit)

    assert len(manifest.bindings) == 9
    assert set(bindings) == set(EXISTING) | set(HISTORY)
    assert set(manifest.units) == {"adler32.c", "deflate.c", "inflate.c", "infutil.c", "trees.c"}
    # The initial-prefix comparisons retain the useful original caller/indirect
    # edges and the correction lessons; don't copy post-prefix emitter boilerplate
    # or function bodies into the dependency companion.
    model_notes = {
        0x0063CF00: ("callers   1", "calls     0x0063CF30"),
        0x0063D4D0: ("indirect  0x0063D4EC", "0x0063D53D"),
        0x0063DF90: ("calls     0x00643950", "indirect  0x0063DFC5"),
        0x0063E0F0: ("calls     0x0063DFE0", "CORRECTED from sub_63e0f0"),
        0x0063D450: ("callers   4", "LEVER: `rep movsd`/`rep movsb`", "CORRECTED from sub_63d450"),
    }
    for address, notes in model_notes.items():
        for note in notes:
            assert note in component_normalized

    # Review finding beyond the copied prefixes: 0063DF90's old body notes also
    # established that its 0x00643950 callee reads one stack pointer argument
    # (__cdecl), correcting the earlier nullary catalog guess. This concise ABI
    # fact may be worth a separate historical note; the accompanying C-linkage
    # compiler workaround, generated declarations, and body are not.

    # Exercise the native exact origin/span selection join, not just manifest parsing.
    source_rows = [
        {"path": "src/vendor/zlib.recovery.h",
         "entity_id": Origin(manifest.bindings[0].origin.image_sha256, "pe:va", row.address).entity_id,
         "spans": [list(span) for span in row.image_spans], "invalid_spans": []}
        for row in records.values() if row.address in set(bindings)
    ]
    snapshot = {
        "image_sha256": manifest.bindings[0].origin.image_sha256,
        "source_configuration": {"image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
                                 "seed_roots": ["src/recovered", "src/unrecovered"],
                                 "symbol_format": "msvc"},
        "records": source_rows,
        "files": {"src/vendor/zlib.recovery.h": {
            "content_base64": base64.b64encode(component_text.encode()).decode("ascii")}},
    }
    selected = exact_component_record_bindings(snapshot)
    expected_keys = {("src/vendor/zlib.recovery.h", binding.origin.entity_id)
                     for binding in manifest.bindings}
    assert set(selected) == expected_keys
    assert {owner for owner, _, _ in selected.values()} == {"zlib"}

    assert parse_component(render_component(manifest)) == manifest
    assert write(component_text, read_file(component_path)) == component_text


def test_static_flush_pending_stays_conditional_on_physical_coff_symbol():
    manifest = parse_component((ROOT / "src/vendor/zlib.recovery.h").read_text(encoding="utf-8"))
    binding = next(item for item in manifest.bindings if item.origin.address == 0x0063D450)
    source = (ROOT / "src/vendor/zlib-1.0.2/deflate.c").read_text(encoding="latin1")
    assert "local void flush_pending(strm)" in source
    assert "#  define local static" in (ROOT / "src/vendor/zlib-1.0.2/zutil.h").read_text(encoding="latin1")
    assert binding.symbol == "_flush_pending" and binding.unit == "deflate.c"
    assert "no current COFF" in binding.evidence or "unqualified" in binding.evidence
