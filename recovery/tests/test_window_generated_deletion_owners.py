"""Real compiler-generated deleting entries, not handwritten seed wrappers.

Source and native controls do not prove historical TU or retained COMDAT
selection. The baseline comparison protects the complete compiled owner input,
not the linked application or original-game runtime.
"""
import os
from pathlib import Path
import shutil
import struct
import subprocess

import pytest

from match_recovery.source import Mode, State, read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = "4e9b5d1b3d4fc87e8a3d616e3936f6deaf2c061f"
SPECS = (
    (0x421830, "MapWin", "src/mapwin.cpp", "src/recovered/units/00421830.cpp", 0x22480),
    (0x48BF20, "PlanWin", "src/planwin.cpp", "src/recovered/0048bf20.cpp", 0x22A64),
    (0x51DDF0, "Console", "src/console.cpp", "src/recovered/0051ddf0.cpp", 0x247A8),
)


@pytest.mark.parametrize("address,klass,unit,seed,size", SPECS)
def test_generated_deleting_identity_retains_seed_history(address, klass, unit, seed, size):
    original = subprocess.check_output(["git", "show", BASE + ":" + seed], cwd=ROOT).decode()
    old, = read_text(original, ROOT / seed)
    current, = (r for r in read_file(ROOT / unit) if r.address == address)
    assert current.mode is Mode.FILE and current.state is State.IMPLEMENTED
    assert (current.symbol or current.name) == f"??_G{klass}@@UAEPAXI@Z"
    for field in ("address", "name", "image_spans", "byte_exact", "semantic",
                  "levers", "ruled_out", "deferred", "unrecoverable"):
        assert getattr(current, field) == getattr(old, field), field
    assert current.image_spans == ((address, address + 54),)
    assert not (ROOT / seed).exists(), "retire only the completely superseded seed"


def test_real_generated_entries_and_unchanged_compiled_owner_inputs():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.coff_replica import object_signature, _object_evidence
    from match_recovery.codeview import class_layout
    from match_recovery.source_inventory import image_pin
    from recovery.environment.wine import VC6_ROOT

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned original, VC6 and Wine32 required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    replacements = {unit: subprocess.check_output(["git", "show", BASE + ":" + unit], cwd=ROOT)
                    for _address, _klass, unit, _seed, _size in SPECS}
    for flags in settings.flag_sets:
        for address, klass, unit, _seed, size in SPECS:
            current = Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
            baseline = Coff(compiler.overlay(ROOT / unit, replacements, (), flags + settings.layout_flags))
            if object_signature(current) != object_signature(baseline):
                # VC6 attributes generated entry .bf/.ef records to EOF. Appending
                # source mappings moves those debug line numbers, not code.
                # This is a narrowly checked control, not a relaxed package
                # preservation gate or a declaration of whole-object equality.
                old = _object_evidence(baseline, replica=True)
                new = _object_evidence(current, replica=True)
                assert old["sections"] == new["sections"], (unit, flags)
                assert old["associations"] == new["associations"], (unit, flags)
                assert len(old["symbols"]) == len(new["symbols"])
                added_lines = len((ROOT/unit).read_bytes().splitlines()) - len(replacements[unit].splitlines())
                assert added_lines > 0
                changes = 0
                for (old_key, old_aux), (new_key, new_aux) in zip(old["symbols"], new["symbols"]):
                    assert old_key == new_key, (unit, flags)
                    if old_aux == new_aux:
                        continue
                    assert old_key[0] in (".bf", ".ef")
                    expected_kind = "begin-function" if old_key[0] == ".bf" else "raw"
                    assert old_aux[0] == new_aux[0] == expected_kind
                    assert old_aux[2:] == new_aux[2:]
                    before, after = bytes.fromhex(old_aux[1]), bytes.fromhex(new_aux[1])
                    assert before[:4] == after[:4] and before[6:] == after[6:]
                    assert struct.unpack_from("<H", after, 4)[0] - struct.unpack_from("<H", before, 4)[0] == added_lines
                    changes += 1
                assert changes > 0
            assert class_layout(current, klass)["size"] == size
            symbol = current.function(f"??_G{klass}@@UAEPAXI@Z")
            section = current.sections[symbol.section - 1]
            targets = [current.symbols[target].name for _offset, target, kind in section.relocations if kind == 20]
            # Complete-object destruction is generated from the real class,
            # including its virtual base; do not write the native two-stage
            # destructor cascade again in the source.
            if not any(target.startswith(f"??_D{klass}@@") for target in targets):
                assert f"??1{klass}@@UAE@XZ" in targets, targets
                assert "??1GraphicWin@@UAE@XZ" in targets, targets
            assert "??3@YAXPAX@Z" in targets, targets
            native, = (r for r in read_file(ROOT / unit) if r.address == address)
            from match_recovery.model import Span
            body = image.read(Span(*native.image_spans[0]))
            assert len(body) == 54 and body[-3:] == b"\xc2\x04\x00"
