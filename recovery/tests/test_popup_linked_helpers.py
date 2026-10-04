"""Actual-owner behavior and native controls for mapped Popup helper bodies.

This intentionally fails until all five actual product mappings exist in
src/popup.cpp. Bodies are read from the package source reader and bounded with
the existing game-logic test's _definition helper; no draft-body fallback is
permitted. Host controls are separate from the bounded native evidence below;
neither establishes full task acceptance or game correctness.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, State, read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/popup.cpp"
FIXTURE = Path(__file__).with_name("popup_linked_helpers_harness.cpp")
BODY_SPECS = (
    (0x00406B70, "void Popup::vslot_48("),
    (0x00406BA0, "void Popup::vslot_49("),
    (0x004050A0, "int Popup::on_link_clicked("),
    (0x0044CBD0, "void __cdecl popup_pending_link_dialog("),
    (0x0044CA90, "int __cdecl popup_faction_datalinks("),
)


def actual_body(address, signature):
    records = [record for record in read_file(SOURCE) if record.address == address]
    assert len(records) == 1, (
        "src/popup.cpp must own exactly one current source mapping for "
        f"{address:#010x}; found {len(records)}"
    )
    record = records[0]
    if address == 0x004050A0:
        # Existing producer retains its legacy FILE annotation in this TU;
        # extract its real implementation without rewriting that obligation.
        assert record.mode is Mode.FILE and record.byte_exact
        assert record.symbol == "?on_link_clicked@Popup@@UAEHHPAD@Z"
    else:
        assert record.mode is Mode.BODY, f"{address:#010x} is not a BODY mapping"
    assert record.state is State.IMPLEMENTED, f"{address:#010x} is not implemented"
    assert record.region, f"{address:#010x} has no source-reader region"
    assert signature in record.region, f"{address:#010x} mapping has wrong source identity"
    assert "BODY GOES HERE." not in record.region
    # The source-reader region is mapped and bounded by the next annotation;
    # use the project test helper to isolate the exact real C++ definition.
    return _definition(record.region, signature)


def current_bodies():
    return [actual_body(address, signature) for address, signature in BODY_SPECS]


def replace_once(text, old, new):
    assert text.count(old) == 1, f"mutation anchor not unique: {old!r}"
    return text.replace(old, new, 1)


def mutate_forwarded_second(body, slot):
    pattern = re.compile(
        r"(\.vslot_" + str(slot) + r"\s*\(\s*)([A-Za-z_]\w*)"
        r"(\s*,\s*)([A-Za-z_]\w*)(\s*\))"
    )
    changed, count = pattern.subn(lambda match: match.group(1) + match.group(2)
                                  + match.group(3) + match.group(2) + match.group(5),
                                  body, count=1)
    assert count == 1, f"could not mutate slot {slot} forwarded arguments"
    return changed


def compiler():
    path = shutil.which("clang++") or shutil.which("g++")
    if path is None:
        pytest.skip("host C++ compiler unavailable")
    return path


def run_fixture(tmp_path, bodies, allow_compile_failure=False):
    fixture = FIXTURE.read_text(encoding="utf-8")
    marker = "// ACTUAL_SOURCE_BODIES"
    assert fixture.count(marker) == 1
    source = fixture.replace(marker, "\n\n".join(bodies))
    cpp, exe = tmp_path / "actual_popup.cpp", tmp_path / "actual_popup"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler(), "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-pedantic", str(cpp), "-o", str(exe)],
        cwd=tmp_path, capture_output=True, text=True,
    )
    if built.returncode != 0:
        assert allow_compile_failure, built.stdout + built.stderr
        return built.returncode, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], cwd=tmp_path, capture_output=True, text=True)
    return ran.returncode, ran.stdout + ran.stderr


def test_mapped_popup_bodies_compile_and_pass_owner_controls(tmp_path):
    result, output = run_fixture(tmp_path, current_bodies())
    assert result == 0, output


@pytest.mark.parametrize("slot,body_index", [(48, 0), (49, 1)])
def test_forwarding_argument_damage_is_detected(tmp_path, slot, body_index):
    bodies = current_bodies()
    bodies[body_index] = mutate_forwarded_second(bodies[body_index], slot)
    result, _ = run_fixture(tmp_path, bodies)
    assert result != 0


def test_forwarding_wrong_receiver_damage_is_detected(tmp_path):
    bodies = current_bodies()
    bodies[0] = replace_once(
        bodies[0], "static_cast<GraphicWin &>(dialogs())",
        "static_cast<GraphicWin &>(*this)",
    )
    result, _ = run_fixture(tmp_path, bodies)
    assert result != 0


@pytest.mark.parametrize("loop_damage", ["initial_reset", "pending_reset"])
def test_pending_popup_reset_order_damage_is_detected(tmp_path, loop_damage):
    bodies = current_bodies()
    body = bodies[3]
    if loop_damage == "initial_reset":
        pattern = re.compile(
            r"PopupPendingLinkFlag\s*=\s*0\s*;\s*X_pop\s*\(\s*"
            r"([A-Za-z_]\w*)\s*,\s*0\s*\)\s*;"
        )
        bodies[3], count = pattern.subn(
            lambda match: "X_pop(" + match.group(1)
            + ", 0); PopupPendingLinkFlag = 0;", body, count=1
        )
    else:
        pattern = re.compile(
            r"PopupPendingLinkFlag\s*=\s*0\s*;\s*"
            r"X_pop\s*\(\s*PopupPendingLink\s*,\s*0\s*\)\s*;"
        )
        replacement = "X_pop(PopupPendingLink, 0); PopupPendingLinkFlag = 0;"
        bodies[3], count = pattern.subn(replacement, body, count=1)
    assert count == 1, f"could not mutate {loop_damage} ordering"
    result, _ = run_fixture(tmp_path, bodies)
    assert result != 0


def test_pending_link_producer_clear_damage_is_detected(tmp_path):
    bodies = current_bodies()
    bodies[2] = replace_once(bodies[2], "PopupPendingLink[0] = 0;", "")
    result, _ = run_fixture(tmp_path, bodies)
    assert result != 0


@pytest.mark.parametrize("old,new", [
    ("Players[faction_id].filename", "Players[faction_id].search_key"),
    ('"DATALINKS"', '"DATALINKS1"'),
    ("return x_pop_caption_label(", "x_pop_caption_label("),
])
def test_faction_popup_member_label_and_result_damage_is_detected(tmp_path, old, new):
    bodies = current_bodies()
    bodies[4] = replace_once(bodies[4], old, new)
    result, _ = run_fixture(tmp_path, bodies, allow_compile_failure=old.startswith("return "))
    assert result != 0


def test_native_popup_forwarder_slots_and_receivers():
    import os
    import struct
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.source_inventory import image_pin
    from match_recovery.model import Span
    from match_recovery.codeview import class_layout
    from match_recovery.application_binary import select_bindings, check_compiled
    from match_recovery.ownership_source import project_view
    from match_recovery.object_source import read_objects
    from match_recovery.literal_source import read_literals
    from match_recovery.model import Origin
    from test_application_link_recovery import references
    from recovery.environment.wine import VC6_ROOT

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned image, VC6 and Wine32 required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    assert struct.unpack("<2I", image.read(Span(0x6695C8 + 48*4, 0x6695C8 + 50*4))) == (0x406B70, 0x406BA0)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    pin = image_pin(ROOT)
    view, _snapshot = project_view(ROOT)
    selected = frozenset(Origin(pin, "pe:va", address).entity_id for address in (0x44CBD0, 0x44CA90))
    bindings, refused, externals = select_bindings(view, selected)
    assert len(bindings) == 2 and not refused
    globals_ = [b for b in read_objects(ROOT,pin) if b.symbol in
                ("?PopupPendingLink@@3PADA", "?PopupPendingLinkFlag@@3HA", "?Players@@3PAUPlayer@@A")]
    assert len(globals_) == 3
    literals = [b for b in read_literals(ROOT,pin) if b.unit == "src/popup.cpp" and b.origin.address == 0x68469C]
    assert len(literals) == 1
    strict_trials = []
    for flags in settings.flag_sets:
        obj = Coff(compiler(SOURCE, (), flags + settings.layout_flags))
        assert class_layout(obj, "Popup")["size"] == 0x537C
        table, = (s for s in obj.symbols.values() if s.name == "??_7Popup@@6BWin@@@" and s.section > 0)
        section = obj.sections[table.section - 1]
        relocated = {offset: obj.symbols[target].name for offset, target, kind in section.relocations if kind == 6}
        for slot in (48, 49):
            symbol = f"?vslot_{slot}@Popup@@UAEXHH@Z"
            assert relocated[table.value + slot*4] == symbol
            obj.function(symbol)
        for address in (0x44CBD0, 0x44CA90):
            record, = (r for r in read_file(SOURCE) if r.address == address)
            obj.function(record.symbol)
        assert references(obj, "?popup_pending_link_dialog@@YAXPBD@Z").count("?X_pop@@YAHPBDP6AHXZ@Z") == 2
        # Bounded body-reference controls, not the transitive whole-TU/global
        # graph or full task/game acceptance. Compile the actual Players owner
        # rather than resolving its address from the operand being checked.
        owners = {"src/popup.cpp":obj,
                  "src/alpha.cpp":Coff(compiler(ROOT/"src/alpha.cpp", (), flags+settings.layout_flags))}
        functions, _data, _generated, _imports, _shared, checked_literals = check_compiled(
            bindings, globals_, (), owners, image, externals, literal_bindings=literals)
        strict_trials.append((flags, functions))
        assert len(functions) == 2
        assert len(checked_literals) == 1 and checked_literals[0]["content"] == "pass"
    # Frame-pointer variants are alternative original body recipes, not a claim
    # that both instruction spellings occur at the same original address. Layout,
    # virtual receivers and literal contents above must hold in every trial.
    import json
    for address in ("0x0044CA90", "0x0044CBD0"):
        assert any(row["address"] == address and row["result"] == "pass"
                   for _flags, rows in strict_trials for row in rows), json.dumps(strict_trials, default=str)
