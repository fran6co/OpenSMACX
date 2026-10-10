"""Two pinned archive correspondences; not original link/runtime acceptance."""
from dataclasses import replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_source import parse_component, render_component
from match_recovery.model import Span
from match_recovery.source import read_text

from test_crt_eight_entry_bindings_oct10 import inputs, verify, row, repin, raw_offset


ROOT = Path(__file__).resolve().parents[2]
COMPANION = "src/vendor/vc6-runtime.recovery.h"
BASE = "bb8f5b194b7b907486db46f66c29fe803b6ae7d4"
EXPECTED = (
    (0x647C66, 0x647C89, "__lock_file2", "__lock", "__imp__EnterCriticalSection@4"),
    (0x647CB8, 0x647CDB, "__unlock_file2", "__unlock", "__imp__LeaveCriticalSection@4"),
)


def prior(manifest):
    added = manifest.bindings[-2:]
    assert tuple((binding.origin.address, binding.spans[0].end, binding.symbol)
                 for binding in added) == tuple(item[:3] for item in EXPECTED)
    assert all(binding.unit == "build/intel/mt_obj/_file.obj" for binding in added)
    assert all(binding.spans == (Span(address, end),)
               for binding, (address, end, *_) in zip(added, EXPECTED))
    return replace(manifest, bindings=manifest.bindings[:-2])


def test_only_two_dependency_bindings_added_and_all_history_preserved():
    old_text = subprocess.check_output(["git", "show", BASE + ":" + COMPANION],
                                       cwd=ROOT, text=True)
    text = (ROOT / COMPANION).read_text()
    current = parse_component(text)
    assert prior(current) == parse_component(old_text)
    assert parse_component(render_component(current)) == current
    path = ROOT / COMPANION
    # FILE regions include the modified component block. Compare parsed records
    # without that envelope; the exact restored-text assertion below covers it.
    assert [replace(record, line=1, region="") for record in read_text(text, path)] == [
        replace(record, line=1, region="") for record in read_text(old_text, path)
    ]
    assert not any(binding.symbol in ("_fopen", "__fsopen", "__ftbuf", "__flush")
                   for binding in current.bindings)
    # Source prose outside these two annotation lines and one comma is untouched.
    lines = [line for line in text.splitlines()
             if any('"origin": "0x%08X"' % address in line for address, *_ in EXPECTED)]
    assert len(lines) == 2
    for line in lines:
        text = text.replace(line + "\n", "", 1)
    previous_last = next(line for line in reversed(old_text.splitlines())
                         if '"origin":' in line and '"unit":' in line)
    assert text.count(previous_last + ",\n") == 1
    assert text.replace(previous_last + ",\n", previous_last + "\n", 1) == old_text


def test_both_pass_without_new_helper_hypotheses_and_prior_verdicts_unchanged(inputs):
    manifest, _, _, _ = inputs
    before, after = verify(inputs, prior(manifest)), verify(inputs)
    assert after["functions"][:-2] == before["functions"]
    for address, _, _, callee, imported in EXPECTED:
        result = row(after, address)
        assert all(result[key] == "pass" for key in ("result", "boundary", "bytes", "references"))
        assert {item["symbol"] for item in result["relocations"]} == {callee, imported}
        assert all(item["result"] == "pass" for item in result["relocations"])
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


@pytest.mark.parametrize("address,end,symbol,callee,imported", EXPECTED)
@pytest.mark.parametrize("damage", ("opcode", "call_target", "import_target"))
def test_original_damage_cannot_pass_even_with_fresh_pin(
    inputs, address, end, symbol, callee, imported, damage,
):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    offset = {"opcode": 0, "call_target": 14, "import_target": 30}[damage]
    changed[raw_offset(image, address + offset)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), address)
    assert result["result"] != "pass"
    if damage != "opcode":
        assert result["references"] != "pass"


@pytest.mark.parametrize("address,end,symbol,callee,imported", EXPECTED)
def test_shortened_boundary_and_wrong_archive_member_refuse(inputs, address, end, symbol, callee, imported):
    manifest, _, _, _ = inputs
    for damage in ("boundary", "member"):
        replacements = []
        for binding in manifest.bindings:
            if binding.origin.address == address:
                binding = (replace(binding, spans=(Span(address, end - 1),))
                           if damage == "boundary" else
                           replace(binding, unit="build/intel/mt_obj/mlock.obj"))
            replacements.append(binding)
        changed = replace(manifest, bindings=tuple(replacements))
        result = row(verify(inputs, changed), address)
        assert result["result"] != "pass"


def test_modified_archive_cannot_pass_unchanged_pin(inputs):
    _, _, archive, _ = inputs
    changed = bytearray(archive)
    changed[-1] ^= 1
    report = verify(inputs, archive=bytes(changed))
    assert report["archive"]["result"] == "fail"
    assert report["result"] == "fail"
