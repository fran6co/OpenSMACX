"""Permanent source/history checks for the twelve migrated adjustor identities."""
from pathlib import Path
import re
import hashlib

from match_recovery.source import Mode, SourceRole, read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
CLASS = {"radiobutton": "RadioButton", "dialogs": "Dialogs"}
EXPECTED = (
    (0x407030, "radiobutton", 0x60E020, "on_key_down"),
    (0x4070A0, "radiobutton", 0x60E220, "on_redraw"),
    (0x407150, "dialogs", 0x613220, "on_scroll_create"),
    (0x407160, "dialogs", 0x613260, "on_scroll_delete"),
    (0x407180, "dialogs", 0x612FC0, "on_mousewheel"),
    (0x4071B0, "dialogs", 0x612E80, "on_left_click"),
    (0x4071E0, "dialogs", 0x612F60, "on_right_click"),
    (0x407210, "dialogs", 0x612CC0, "on_key_down"),
    (0x4072B0, "dialogs", 0x612FE0, "attach"),
)
DELETING = (
    (0x48C040, "planwin", 0x48BF20, "adjust_plan_win2_scalar_delete"),
    (0x51DDE0, "console", 0x51DDF0, "adjust_console1_scalar_delete"),
    (0x51DF50, "console", 0x51DDF0, "adjust_console2_scalar_delete"),
)
SOURCE_SHA256 = "5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549"
HISTORY_SHA256 = {
    0x407030: "127bea19bd51d4ffaa256021e528250fd06478161070b5cf17c94a0e518b0262",
    0x4070A0: "a916e3f64f8676db3c9c3fb87acda341ac5bc95a463b2c4f7c26778110b7d2b3",
    0x407150: "1c61ef27792b94d97baf7e4eeeb2e729fa6bcaf698d2c35e24918f5d64099826",
    0x407160: "18eb40a5eb6529a13a790b90880a878d82f37e3b1324491eb4c2b7f3aadd384b",
    0x407180: "eec3a684474f320524f5a5a5646f28e479addd3d27da661c7df6fd78bb664400",
    0x4071B0: "311d6ed9fbd175ec5ccbbefdccb5abeca800bcc344d9479e6d9a6fbb18da1ed3",
    0x4071E0: "04ac05c124f77eaaa62921668d02a1c75f9ba667e2633e70a70be460a890422c",
    0x407210: "70dac72bf15018a278389576c3676553978f6797532eb074f8425aab32a3065f",
    0x4072B0: "8e2b34215067873fa587d91183d644ed7c4f81587bc029482942fc941f96b566",
    0x48C040: "b93efd888bdf42e0677fd6e0bbfb6fe5cb8ec58818d63167c25a45346f51d7ae",
    0x51DDE0: "0521c4f5ad1301580c581a88b1479693698786bf2c22227d2ccafa4d64ac80fb",
    0x51DF50: "28abc097b47dd56ed1ee00d1e916a015f477f4889625fe90dd40a0b9eab886c9",
}


def find_record(path: Path, address: int):
    return [row for row in read_file(path) if row.address == address]


def assert_archived_history_text(text: str, address: int):
    token = f"RECOVERY_HISTORY: v1 address=0x{address:08X} "
    assert text.count(token) == 1, f"{address:08X} full comment envelope missing/duplicated"
    line_at = text.index(token)
    start = text.rfind("/*", 0, line_at)
    close = text.find("\n */", line_at)
    assert start >= 0 and close >= 0, f"{address:08X} history archive boundaries malformed"
    archive = text[start:close + 4]
    header = text[line_at:text.find("\n", line_at)]
    assert f"source_sha256={SOURCE_SHA256}" in header
    assert f"envelope_sha256={HISTORY_SHA256[address]}" in header
    payload = []
    for line in archive.splitlines()[2:-1]:
        if line == " * [ARCHIVE]":
            payload.append("")
        else:
            assert line.startswith(" * [ARCHIVE] "), line
            payload.append(line[len(" * [ARCHIVE] "):])
    normalized = "\n".join(payload)
    assert hashlib.sha256(normalized.encode("utf-8", "surrogateescape")).hexdigest() == HISTORY_SHA256[address]
    assert f"// original marker: 0x{address:08X} " in normalized
    assert f"// ORIGINAL: 0x{address:08X} " not in normalized


def assert_archived_history(path: Path, address: int):
    assert_archived_history_text(path.read_text(), address)


def test_all_twelve_full_seed_comment_envelopes_are_archived_in_owners():
    for address, owner, *_ in EXPECTED:
        assert_archived_history(ROOT / f"src/{owner}.cpp", address)
    for address, owner, *_ in DELETING:
        assert_archived_history(ROOT / f"src/{owner}.cpp", address)


def test_each_adjustor_identity_has_one_real_class_owner_and_original_member():
    shim = ROOT / "src/adjustor_thunks.cpp"
    for address, owner, target, _method in EXPECTED:
        owner_path = ROOT / f"src/{owner}.cpp"
        assert not find_record(shim, address), f"{address:08X} remains in the shim"
        owner_record = find_record(owner_path, address)
        assert len(owner_record) == 1, f"{address:08X} must have one owner mapping"
        assert owner_record[0].mode is Mode.FILE
        assert owner_record[0].source_role is not SourceRole.SCAFFOLD
        assert owner_record[0].symbol and "adjust_" not in owner_record[0].symbol
        member = find_record(owner_path, target)
        assert len(member) == 1, f"target {target:08X} must stay mapped in {owner}.cpp"
        assert member[0].symbol and "@" + CLASS[owner] + "@@" in member[0].symbol


def test_radio_and_dialogs_express_the_real_virtual_owner_contracts():
    radio = (ROOT / "src/radiobutton.h").read_text()
    dialogs = (ROOT / "src/dialogs.h").read_text()
    win = (ROOT / "src/win.h").read_text()
    assert re.search(r"class\s+RadioButton\s*:\s*public\s+virtual\s+GraphicWin\s*,\s*public\s+virtual\s+Dialog", radio)
    assert re.search(r"\bint\s+on_key_down\s*\([^;]*\)\s*override\s*;", radio)
    assert re.search(r"\bvirtual\s+void\s+on_redraw\s*\(\s*\)", win)
    assert re.search(r"\bvoid\s+on_redraw\s*\(\s*\)\s*;", radio)
    assert re.search(r"class\s+Dialogs\s*:\s*public\s+ListBox", dialogs)
    for declaration in (
        r"virtual\s+int\s+on_scroll_create\s*\(\s*\)",
        r"virtual\s+int\s+on_scroll_delete\s*\(\s*Scroll\s*\*",
        r"void\s+on_mousewheel\s*\([^;]*\)\s*override",
        r"void\s+on_left_click\s*\([^;]*\)\s*override",
        r"void\s+on_right_click\s*\([^;]*\)\s*override",
        r"int\s+on_key_down\s*\([^;]*\)\s*override",
        r"int\s+attach\s*\(\s*GraphicWin\s*\*[^;]*\)",
    ):
        assert re.search(declaration, dialogs), declaration


def test_scalar_deleting_adjustors_stay_with_real_destructor_owners():
    shim = ROOT / "src/adjustor_thunks.cpp"
    for address, owner, primary, _helper in DELETING:
        owner_path = ROOT / f"src/{owner}.cpp"
        assert not find_record(shim, address), f"{address:08X} remains in the shim"
        owner_record = find_record(owner_path, address)
        assert len(owner_record) == 1, f"{address:08X} must have one owner mapping"
        assert owner_record[0].mode is Mode.FILE
        assert owner_record[0].source_role is not SourceRole.SCAFFOLD
        assert owner_record[0].symbol and "adjust_" not in owner_record[0].symbol
        dtor = find_record(owner_path, primary)
        assert len(dtor) == 1 and dtor[0].mode is Mode.FILE
    planwin = (ROOT / "src/planwin.h").read_text()
    console = (ROOT / "src/console.h").read_text()
    assert re.search(r"class\s+PlanWin\s*:\s*public\s+MapWin", planwin)
    assert re.search(r"~PlanWin\s*\(\s*\)", planwin)
    assert re.search(r"class\s+Console\s*:\s*public\s+MapWin", console)
    assert re.search(r"MEASURED\s+~Console\s*\(\s*\)", console)
