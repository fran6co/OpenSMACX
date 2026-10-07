"""Primary BasePop identities must not also claim an additional shared body.

These source controls unblock native data verification; they do not establish
compiled vtable contents, linked folding, or application acceptance.
"""
from pathlib import Path

import pytest

from match_recovery.ownership_source import project_view
from match_recovery.shared_source import parse_shared, read_shared, validate_shared
from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
PRIMARY = {
    0x00406B20: (0x00406B23, "?on_adjust_button_placement@BasePop@@UAEXPAH00@Z"),
    0x00406840: (0x00406846, "?on_exec@BasePop@@UAEHXZ"),
}


@pytest.fixture(scope="module")
def live_bindings():
    view, snapshot = project_view(ROOT, allow_empty=True)
    pin = snapshot["image_sha256"]
    return view, pin, read_shared(ROOT, pin)


def test_live_shared_bindings_do_not_duplicate_primary_originals(live_bindings):
    view, _pin, shared = live_bindings
    validate_shared(shared, view)
    records = {row.address: row for row in read_file(ROOT / "src/basepop.cpp")}
    for address, (end, symbol) in PRIMARY.items():
        row = records[address]
        assert row.image_spans == ((address, end),)
        assert row.byte_exact and row.symbol == symbol
        assert row.body == "src/basepop.h"
        assert all(binding.symbol != symbol for binding in shared)
        # Remove only the redundant primary interpretation, not genuine
        # additional definitions of the same folded original code interval.
        expected_aliases = ({"?on_iface_group_clicked@BaseWin@@UAEXHHH@Z",
                             "?on_iface_group_clicked@DiploWin@@UAEXHHH@Z"}
                            if address == 0x00406B20 else set())
        actual_aliases = {binding.symbol for binding in shared
                          if binding.origin.address == address}
        assert actual_aliases == expected_aliases


@pytest.mark.parametrize("address", PRIMARY)
def test_reintroduced_primary_shared_binding_is_refused(live_bindings, address):
    view, pin, shared = live_bindings
    end, symbol = PRIMARY[address]
    definition = ("void default_method(int *, int *, int *) {}" if address == 0x00406B20
                  else "int default_method() { return 1; }")
    duplicate = parse_shared(
        f"// RECOVERY_SHARED: v1 0x{address:08X} 0x{end:08X} {symbol} 0x00602600\n"
        + definition, "src/basepop.h", pin, address_space="pe:va")
    assert len(duplicate) == 1 and duplicate[0].body_error is None
    with pytest.raises(ValueError, match="duplicates an existing source binding"):
        validate_shared((*shared, *duplicate), view)
