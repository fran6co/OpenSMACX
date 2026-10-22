"""Preserve SelectPartWin ownership and source order in DesignWin's TU."""

from pathlib import Path
import subprocess

from match_recovery.source import read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "2c2c407fba69ba73e4556995d5cc2cf85e3e0308"


def _identity_facts(row):
    return (
        row.address,
        row.name,
        row.symbol,
        tuple(row.image_spans),
        row.mode.value,
        row.byte_exact,
        row.semantic,
        row.kind,
        row.exclusion,
        tuple(row.levers),
        tuple(row.ruled_out),
        tuple(row.unrecoverable),
        tuple(row.deferred),
        row.source_role.value,
        row.source_role_reason,
        tuple(
            (lesson.kind, lesson.key, lesson.prose,
             tuple(text for _, text in lesson.lines))
            for lesson in row.origin
        ),
    )


def test_both_original_owners_keep_all_mapping_identity_and_history():
    retired = ROOT / "src/selectpartwin.cpp"
    design = ROOT / "src/designwin.cpp"
    assert not retired.exists()

    original = {}
    for relative in ("src/designwin.cpp", "src/selectpartwin.cpp"):
        original_text = subprocess.check_output(
            ["git", "show", f"{BASE}:{relative}"], cwd=ROOT, text=True
        )
        rows = read_text(original_text, relative)
        assert len({row.address for row in rows}) == len(rows)
        for row in rows:
            assert row.address not in original
            original[row.address] = _identity_facts(row)
    assert len(original) == 89

    current_rows = read_file(design)
    assert len({row.address for row in current_rows}) == len(current_rows)
    current = {row.address: _identity_facts(row) for row in current_rows}
    assert len(current) == 89
    assert set(original) == set(current)
    for address, facts in original.items():
        assert current[address] == facts, f"identity or historical evidence changed at {address:08X}"

    source = design.read_text(encoding="utf-8")
    selectpart_definitions = (
        "int SelectPartWin::exec(",
        "void SelectPartWin::close()",
        "int SelectPartWin::on_key_click(",
        "void SelectPartWin::on_scrolling(",
        "void SelectPartWin::on_redraw()",
        "void SelectPartWin::on_mouse_move(",
        "void SelectPartWin::on_left_up(",
        "void SelectPartWin::on_scrolled(",
        "int SelectPartWin::draw_part(",
        "int SelectPartWin::draw_chassis(",
        "int SelectPartWin::draw_reactor(",
        "int SelectPartWin::draw_armor(",
        "int SelectPartWin::draw_weapon(",
    )
    positions = [source.index(signature) for signature in selectpart_definitions]
    assert positions == sorted(positions)
    assert positions[-1] < source.index("void DesignWin::init()")
