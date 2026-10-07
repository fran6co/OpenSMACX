"""Permanent-test candidate for the CRT iterator source-owner migration.

This deliberately reads only active source and the immutable comparison
revision. It never consumes the ignored source candidate, compiler outputs, or
cached verification reports. Run after a coordinator transfers the annotations
to the active component source:

    UV_CACHE_DIR=/tmp/osmx-uv-cache uv run pytest -q \
      recovery/tests/test_crt_component_migration.py

Dependency ownership is checked here; original provider, linked selection and
runtime/unwind equivalence require separate evidence.
"""

from __future__ import annotations

from dataclasses import fields
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component
from match_recovery.source import read_text


BASELINE = "2c29aa969f7ccdc6b7e9c2e261dbc6d7aedf8b9b"
ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/IMAGE_PIN').is_file())
LEAF = Path("src/leaf_recoveries.cpp")
COMPONENT = Path("src/vendor/vc6-runtime.recovery.h")

EXPECTED = {
    0x6456E4: {
        "name": "??_M@YGXPAXIHP6EX0@Z@Z",
        "span": (0x6456E4, 0x645764),
        "symbol": "??_M@YGXPAXIHP6EX0@Z@Z",
        "unit": "build/intel/mt_obj/ehvecdtr.obj",
    },
    0x6457C2: {
        "name": "??_L@YGXPAXIHP6EX0@Z1@Z",
        "span": (0x6457C2, 0x645844),
        "symbol": "??_L@YGXPAXIHP6EX0@Z1@Z",
        "unit": "build/intel/mt_obj/ehvecctr.obj",
    },
}


def _git_text(path: Path) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:{path.as_posix()}"],
        cwd=ROOT,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def _states(text: str, path: Path):
    parsed = read_text(text, path)
    assert len({state.address for state in parsed}) == len(parsed), "duplicate ORIGINAL address"
    return {state.address: state for state in parsed}


def _preservation_view(state):
    """All parsed original facts except physical source location and region."""
    omitted = {"path", "line", "region", "extract_error", "origin"}
    values = []
    for item in fields(state):
        if item.name in omitted:
            continue
        value = getattr(state, item.name)
        values.append((item.name, value))
    # `origin` retains verbatim run-log lines; offsets change during rehoming,
    # but their exact text remains historical evidence and must be identical.
    origin = tuple((lesson.kind, lesson.key, lesson.prose,
                    tuple(line for _, line in lesson.lines))
                   for lesson in (state.origin or ()))
    return tuple(values), origin


def _raw_original_block(text: str, address: int) -> str:
    marker = f"// ORIGINAL: 0x{address:08X} "
    assert text.count(marker) == 1, (address, text.count(marker))
    marker_offset = text.index(marker)
    start = text.rfind("/*", 0, marker_offset)
    end = text.find("*/", marker_offset)
    assert start >= 0 and end >= 0, hex(address)
    return text[start:end + 2]


def _manifest(text: str):
    return parse_component(text)


def test_original_iterator_annotations_are_rehomed_without_losing_identity_or_history():
    baseline_leaf = _git_text(LEAF)
    assert not (ROOT / LEAF).exists(), "retired leaf owner source reappeared"
    active_component = (ROOT / COMPONENT).read_text(encoding="utf-8")
    component_states = _states(active_component, COMPONENT)
    baseline_states = _states(baseline_leaf, LEAF)

    for address in EXPECTED:
        original = baseline_states[address]
        assert address in component_states
        assert _preservation_view(component_states[address]) == _preservation_view(original)
        assert _raw_original_block(active_component, address) == _raw_original_block(
            baseline_leaf, address)


def test_runtime_component_adds_only_the_reviewed_iterator_bindings():
    baseline = _manifest(_git_text(COMPONENT))
    active_text = (ROOT / COMPONENT).read_text(encoding="utf-8")
    active = _manifest(active_text)
    baseline_ids = {binding.origin.entity_id for binding in baseline.bindings}
    additions = [binding for binding in active.bindings
                 if binding.origin.entity_id not in baseline_ids]

    assert len(additions) == len(EXPECTED)
    assert len({binding.origin.entity_id for binding in active.bindings}) == len(active.bindings)
    baseline_pin = baseline.bindings[0].origin
    for binding in additions:
        expected = EXPECTED[binding.origin.address]
        assert binding.origin.image_sha256 == baseline_pin.image_sha256
        assert binding.origin.address_space == baseline_pin.address_space
        assert binding.spans == (type(binding.spans[0])(*expected["span"]),)
        assert binding.symbol == expected["symbol"]
        assert binding.unit == expected["unit"]
        # Binding evidence is ownership evidence only, not a matching or
        # runtime-equivalence grant.
        assert "dependency ownership only" in binding.evidence.lower()
        assert "runtime/unwind equivalence remain unknown" in binding.evidence.lower()
        assert "final-link selection" in binding.evidence.lower()

    retained = tuple(binding for binding in active.bindings
                     if binding.origin.entity_id in baseline_ids)
    assert retained == baseline.bindings
    # The component's pinned identity and all non-binding metadata must remain
    # unchanged; only the two reviewed rows are new.
    active_without_additions = type(active)(
        **{item.name: (retained if item.name == "bindings" else getattr(active, item.name))
           for item in fields(active)})
    assert active_without_additions == baseline
