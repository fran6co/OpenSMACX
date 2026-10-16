"""Source ownership for inherited MapWin callbacks in the Console Win view."""
from pathlib import Path

from match_recovery import binding_source
from match_recovery.source import Mode, read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())

SPECS = (
    (0x0051DE40, 0x30, "?on_redraw@MapWin@@QAEXXZ", 0x0046B1D0),
    (0x0051DE80, 0x4C, "?on_left_click@MapWin@@QAEXHH@Z", 0x0046EBA0),
    (0x0051DE90, 0x50, "?on_left_down@MapWin@@QAEXHH@Z", 0x0046ED30),
    (0x0051DEA0, 0x54, "?on_left_up@MapWin@@QAEXHH@Z", 0x0046EFF0),
    (0x0051DEB0, 0x58, "?on_right_click@MapWin@@QAEXHH@Z", 0x0046EBE0),
    (0x0051DEC0, 0x5C, "?on_right_down@MapWin@@QAEXHH@Z", 0x0046EC10),
    (0x0051DEE0, 0x70, "?on_left_double_click@MapWin@@QAEXHH@Z", 0x0046EBD0),
    (0x0051DF20, 0xB0, "?on_button_clicked@MapWin@@QAEXH@Z", 0x0046F8C0),
)


def emitted_symbol(callback_symbol):
    owner_and_name, tail = callback_symbol.split("@@", 1)
    assert tail.startswith("Q")
    return f"{owner_and_name}@@$4PPPPPPPM@CDCI@{tail[1:]}"


def mapped_callback_symbol(original_target_symbol):
    owner_and_name, tail = original_target_symbol.split("@@", 1)
    assert tail.startswith("Q")
    return f"{owner_and_name}@@U{tail[1:]}"


def test_console_view_adjustors_are_hosted_with_console_not_mapwin_callbacks():
    console_path = ROOT / "src/console.cpp"
    mapwin_path = ROOT / "src/mapwin.cpp"
    console_rows = read_file(console_path)
    mapwin_rows = read_file(mapwin_path)
    bindings = binding_source.parse(console_path.read_text(), console_rows)

    for address, slot, callback_symbol, callback_address in SPECS:
        owner, = [row for row in console_rows if row.address == address]
        assert owner.mode is Mode.FILE
        assert owner.symbol == emitted_symbol(callback_symbol)
        assert not [row for row in mapwin_rows if row.address == address]
        callback, = [row for row in mapwin_rows if row.address == callback_address]
        assert callback.symbol == mapped_callback_symbol(callback_symbol)

        binding = bindings[owner.line]
        evidence = "\n".join(binding["evidence"])
        reason = binding["reason"]
        assert "??0Console@@QAE@H@Z" in evidence
        assert "0066EC18" in evidence
        assert f"slot +0x{slot:X}" in evidence
        assert f"{address:08X}" in evidence
        assert f"{callback_address:08X}" in evidence
        assert "src/mapwin.cpp" in evidence
        assert "src/console.cpp" in evidence
        assert "most-derived Console class-view entry" in reason
        assert "inherited MapWin callback" in reason
