"""Source-owned sound-loader identities; native binary checks remain separate."""
from pathlib import Path

import pytest

from match_recovery.object_source import parse_objects
from match_recovery.source import read_text, write_symbol


ROOT = Path(__file__).resolve().parents[2]
UNIT = "src/sound.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def require_wrapper_fact(raw, address):
    record, = (record for record in read_text(raw.decode(), ROOT / UNIT) if record.address == address)
    assert record.name == f"sub_{address:x}"
    assert record.symbol == f"_sub_{address:x}"
    return record


@pytest.mark.parametrize("address", (0x4C6000, 0x4C6030, 0x4C6060))
def test_existing_c_wrapper_has_its_explicit_emitted_symbol(address):
    raw = (ROOT / UNIT).read_bytes()
    record = require_wrapper_fact(raw, address)
    # Native symbol facts are stable updates, not marker or claim rewrites.
    assert write_symbol(raw, record, record.symbol) == raw


def require_module_fact(text):
    binding, = (binding for binding in parse_objects(text, UNIT, PIN, address_space="pe:va")
                if binding.symbol == "?SoundDllModule@@3PAUHINSTANCE__@@A")
    assert binding.origin.address == 0x90DB78
    assert binding.unit == UNIT
    assert binding.span is None and binding.evidence is None
    assert binding.parent_symbol is None and binding.target_symbol is None
    assert "HMODULE SoundDllModule;" in text


def test_loader_module_handle_has_only_evidenced_start_binding():
    require_module_fact((ROOT / UNIT).read_text())


@pytest.mark.parametrize("address", (0x4C6000, 0x4C6030, 0x4C6060))
def test_missing_c_underscore_is_detected(address):
    raw = (ROOT / UNIT).read_bytes()
    record = require_wrapper_fact(raw, address)
    damaged = write_symbol(raw, record, record.name)
    with pytest.raises(AssertionError):
        require_wrapper_fact(damaged, address)


def test_wrong_module_cell_is_detected():
    text = (ROOT / UNIT).read_text()
    expected = "// RECOVERY_OBJECT: v1 0x0090DB78 ?SoundDllModule@@3PAUHINSTANCE__@@A"
    assert text.count(expected) == 1
    damaged = text.replace(expected, expected.replace("0x0090DB78", "0x0090DB74"), 1)
    with pytest.raises(AssertionError):
        require_module_fact(damaged)
