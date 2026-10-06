"""Actual-source controls for the ListBox/Dialogs complete-object migration.

The host executable is a C++ language/lifetime control, not VC6 ABI evidence.
"""
from __future__ import annotations

import hashlib
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import read_file

TEST_PATH = Path(__file__).resolve()
ROOT = next(parent for parent in TEST_PATH.parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT
EXPECTED = {
    0x4080B0: {
        "name": "sub_4080b0", "span": ((0x4080B0, 0x4080D3),),
        "emitted": "??_DListBox@@QAEXXZ",
        "identity_sha256": "7f40e52c1ff3ef507b71d0c25dfd2abaf2fdf382b0a3a1768c0f9b1a6768afa7",
        "envelope_sha256": "70a83c580e4e9e8fb68dc8242a1600ace29ea9d13c303d3766643ed345f5ad22",
    },
    0x406AF0: {
        "name": "sub_406af0", "span": ((0x406AF0, 0x406B16),),
        "emitted": "??_DDialogs@@QAEXXZ",
        "identity_sha256": "f82f593c27defa1d3ce24e6a302818296972d065fd8ccf1161b5a82065c219ca",
        "envelope_sha256": "5ad2c6040e39375aec52f29e501ddd98b12bdebb4e038d81f02f2de8b6f38047",
    },
}


def source(rel: str) -> str:
    return (SOURCE / rel).read_text(encoding="utf-8")


def annotation_digest(row) -> str:
    value = {
        "name": row.name, "spans": row.image_spans, "kind": row.kind,
        "byte_exact": row.byte_exact, "semantic": row.semantic,
        "levers": row.levers, "ruled_out": row.ruled_out,
        "unrecoverable": row.unrecoverable, "deferred": row.deferred,
        "origin": [(item.kind, item.key, item.prose,
                    tuple(text for _, text in item.lines))
                   for item in row.origin or ()],
    }
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def extract_definition(text: str, signature: str) -> str:
    assert text.count(signature) == 1, signature
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unclosed body for {signature}")


def test_original_complete_destructor_identities_are_rehomed_without_claim_loss():
    found = {}
    for rel in ("src/datalink.cpp", "src/factionprofilewin.cpp"):
        for row in read_file(SOURCE / rel):
            if row.address in EXPECTED:
                assert row.address not in found
                found[row.address] = row
    assert set(found) == set(EXPECTED)
    for address, expected in EXPECTED.items():
        row = found[address]
        assert row.name == expected["name"]
        assert row.image_spans == expected["span"]
        assert row.mode.value == "file"
        assert row.symbol == expected["emitted"]
        assert row.kind == "game"
        assert not row.byte_exact and not row.semantic
        assert annotation_digest(row) == expected["identity_sha256"]
        assert "historical original translation unit" in row.region.lower()

    leaf_rows = [row for row in read_file(SOURCE / "src/leaf_recoveries.cpp")
                 if row.address in EXPECTED]
    assert leaf_rows == []
    for address, expected in EXPECTED.items():
        body = source("src/datalink.cpp" if address == 0x4080B0
                      else "src/factionprofilewin.cpp")
        envelopes = re.findall(r"// RECOVERY_HISTORY: /\*.*?// RECOVERY_HISTORY: \*/",
                               body, re.DOTALL)
        envelope = next((item for item in envelopes if f"0x{address:08X}" in item), None)
        assert envelope, f"historical full comment envelope missing for {address:08X}"
        archived = "\n".join(line.removeprefix("// RECOVERY_HISTORY:").removeprefix(" ")
                               for line in envelope.splitlines())
        assert hashlib.sha256(archived.encode()).hexdigest() == expected["envelope_sha256"]


def test_natural_typed_owners_and_only_superseded_seams_are_removed():
    listbox_h = source("src/listbox.h")
    datalink_h = source("src/datalink.h")
    dialogs_h = source("src/dialogs.h")
    basepop_h = source("src/basepop.h")
    profile_h = source("src/factionprofilewin.h")
    assert re.search(r"class ListBox\s*:\s*public virtual GraphicWin,\s*public virtual Dialog", listbox_h)
    assert re.search(r"\bListBox\s+listBox_\s*;", datalink_h)
    assert re.search(r"class Dialogs\s*:\s*public ListBox,.*?public EditGroup", dialogs_h, re.S)
    assert re.search(r"\bDialogs\s+dialogs_\s*;", basepop_h)
    assert re.search(r"\bPopup\s+popup_\s*;", profile_h)
    assert not re.search(r"~Dialogs\s*\(", dialogs_h + source("src/dialogs.cpp"))

    for rel in ("src/leaf_recoveries.cpp", "src/leaf_recoveries.h",
                "src/listbox.cpp", "src/listbox.h", "src/dialogs.cpp",
                "src/dialogs.h", "src/graphicwin.cpp", "src/graphicwin.h"):
        text = source(rel)
        for obsolete in ("LeafListBoxDestructor", "LeafDialogsDestructor",
                         "LeafGraphicWinDestructor", "leaf_004080b0_redirect",
                         "leaf_00406af0_redirect", "list_box_destructor_redirect",
                         "dialogs_destructor_redirect", "graphic_win_destructor_redirect"):
            assert obsolete not in text, (rel, obsolete)

    # This partial Dialog teardown helper is an independently exercised real
    # behavior and remains separate from complete-object compiler lifetimes.
    assert "void __fastcall dialog_destructor_redirect(Dialog *self, void *)" in source("src/dialog.cpp")
    assert "dialog_destructor_redirect(dialog, 0);" in source("recovery/tests/dialog_lifetime_harness.cpp")
    assert "dialog_destructor_redirect" in source("recovery/tests/test_dialog_close_recovery.py")


def test_host_control_compiles_the_actual_listbox_destructor_body_and_virtual_base_lifetime():
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is not installed; source/mapping controls still run")
    actual = extract_definition(source("src/listbox.cpp"), "ListBox::~ListBox()")
    assert re.search(r"ListBox::~ListBox\(\)\s*\{\s*close\(\);\s*\}", actual, re.S)
    # The class bases below are a minimized host model validated against the
    # actual declarations above. The exact product destructor body is pasted
    # from src/listbox.cpp; no product stand-in function is invoked.
    fixture = r'''#include <string>
static std::string events;
struct GraphicWin { virtual ~GraphicWin() { events += "G"; } };
struct Dialog : virtual GraphicWin { virtual ~Dialog() { events += "D"; } };
class ListBox : public virtual GraphicWin, public virtual Dialog {
public:
  virtual ~ListBox();
  void close() { events += "C"; }
};
'''+actual+r'''
struct RadioButton : virtual GraphicWin, virtual Dialog { virtual ~RadioButton() { events += "R"; } };
struct CheckBox : virtual GraphicWin, virtual Dialog { virtual ~CheckBox() { events += "K"; } };
struct SpriteBox : virtual GraphicWin, virtual Dialog { virtual ~SpriteBox() { events += "S"; } };
struct EditGroup : virtual GraphicWin, virtual Dialog { virtual ~EditGroup() { events += "E"; } };
struct Dialogs : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {};
int main() {
  { ListBox value; }
  if (events != "CDG") return 1;
  events.clear();
  { Dialogs value; }
  return events == "ESKRCDG" ? 0 : 2;
}
'''
    with tempfile.TemporaryDirectory(prefix="osmx-complete-dtor-") as temp:
        cpp = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        cpp.write_text(fixture)
        built = subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(exe)],
                               capture_output=True, text=True, check=False)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, check=False)
        assert ran.returncode == 0, (ran.returncode, ran.stdout, ran.stderr)


def test_proposal_records_consumer_emission_not_historical_translation_unit():
    for rel in ("src/datalink.cpp", "src/factionprofilewin.cpp"):
        text = source(rel)
        matching = [row for row in read_file(SOURCE / rel) if row.address in EXPECTED]
        if matching:
            assert "not a claim that the historical original translation unit" in text
            assert "OpenSMACX.map" in text and "current linked copy only" in text
