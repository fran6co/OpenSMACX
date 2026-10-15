"""Direct-object receiver checks for the real TutWin process global."""
from pathlib import Path

from match_recovery.source import read_file


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/tutwin.h").is_file())
CONSUMERS = (
    "time.cpp", "mapwin.cpp", "designwin.cpp", "console.cpp", "veh.cpp", "socialwin.cpp",
)
EXPECTED_IDENTITIES = (
    ("time.cpp", 0x0050EA40),
    ("mapwin.cpp", 0x0046B1F0),
    ("mapwin.cpp", 0x0046AE00),
    ("designwin.cpp", 0x00438460),
    ("designwin.cpp", 0x0043D290),
    ("designwin.cpp", 0x00439960),
    ("designwin.cpp", 0x00439A20),
    ("console.cpp", 0x00511800),
    ("console.cpp", 0x005169F0),
    ("console.cpp", 0x0051BC70),
    ("veh.cpp", 0x0057A660),
    ("socialwin.cpp", 0x004B2470),
)


def test_tutorial_window_uses_real_global_and_has_no_pointer_alias():
    header = (ROOT / "src/tutwin.h").read_text(encoding="utf-8")
    owner = (ROOT / "src/tutwin.cpp").read_text(encoding="utf-8")
    assert "extern TutWin TutorialWindow;" in header
    assert "TutWin *const TutWinGlobal" not in header
    assert "process-lifetime object" in header
    assert "no storage of its own" not in header
    assert "would need a constructor this tree does not model" not in header
    assert "// RECOVERY_OBJECT: v2 0x008C6E68 0x008CC240 ?TutorialWindow@@3VTutWin@@A" in owner
    assert "TutWin TutorialWindow;" in owner
    assert "static_assert(sizeof(TutWin) == 0x53D8" in header

    for name in CONSUMERS:
        source = (ROOT / "src" / name).read_text(encoding="utf-8")
        assert "TutWinGlobal" not in source, name
    for name, address in EXPECTED_IDENTITIES:
        rows = read_file(ROOT / "src" / name)
        row = next((row for row in rows if row.address == address), None)
        assert row is not None, (name, hex(address))
        source = (ROOT / "src" / name).read_text(encoding="utf-8")
        assert any(line.startswith(f"// ORIGINAL: 0x{address:08X} ")
                   for line in source.splitlines()), (name, hex(address))


def test_designwin_host_fixture_checks_real_receiver_identity():
    harness = (ROOT / "recovery/tests/designwin_button_harness.cpp").read_text(encoding="utf-8")
    assert "static TutWin TutorialWindow;" in harness
    assert "TutWin *TutWinGlobal" not in harness
    assert "last_tutorial_receiver = this" in harness
    assert "assert(last_tutorial_receiver == &TutorialWindow);" in harness
