"""Real timer calls, shared teardown and caller controls; not game acceptance."""

from dataclasses import replace
import os
from pathlib import Path
import shutil

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.compiler import Compiler
from match_recovery.import_source import import_targets, read_imports
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.object_source import object_targets, read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source.reader import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references, run_recovered_harness, symbol_at
import test_application_link_recovery as harness_support


@pytest.fixture(scope="module")
def timer_objects():
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    return compiler, settings, {unit: [Coff(compiler(ROOT / unit, (), flags + settings.layout_flags))
                                      for flags in settings.flag_sets]
        for unit in ("src/time.cpp", "src/time_lifecycle.cpp", "src/temp.cpp",
                     "src/editbox.cpp", "src/flatbutton.cpp", "src/netmsg.cpp", "src/console.cpp")}


def original_image():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    return PeImage(path.read_bytes(), image_pin(ROOT))


def binding(unit, address):
    record, = (r for r in read_file(ROOT / unit) if r.address == address)
    return FunctionBinding(Origin(image_pin(ROOT), "pe:va", address),
                           tuple(Span(a, b) for a, b in record.image_spans),
                           record.symbol or record.name, unit)


@pytest.mark.parametrize("unit,address", [("src/editbox.cpp", 0x614F30),
    ("src/flatbutton.cpp", 0x607DA0), ("src/netmsg.cpp", 0x47A7D0),
    ("src/console.cpp", 0x51D7D0)])
def test_callers_reach_the_real_timer_member(timer_objects, unit, address):
    _, _, objects = timer_objects
    for obj in objects[unit]:
        calls = references(obj, symbol_at(unit, address))
        assert calls.count("?close@Time@@QAEXXZ") == 1
        assert not any("time_close_call" in s.name for s in obj.symbols.values())


def test_lifecycle_and_all_emitted_stop_timers_copies_match_targets(timer_objects):
    _, settings, objects = timer_objects
    image = original_image()
    for trial in range(len(settings.flag_sets)):
        units = {unit: trials[trial] for unit, trials in objects.items()}
        targets, _ = object_targets(read_objects(ROOT, image_pin(ROOT)), units)
        imports, _ = import_targets(read_imports(ROOT), units, image)
        targets[symbol_at("src/time.cpp", 0x5FD370)] = 0x5FD370
        targets["?close@Time@@QAEXXZ"] = 0x616780
        for address in (0x616200, 0x616780):
            result, = check_objects([binding("src/time_lifecycle.cpp", address)], units, image,
                                     external_targets=targets, import_targets=imports)
            assert result["boundary"] == result["bytes"] == result["references"] == "pass", result
        stop = binding("src/time.cpp", 0x50F440)
        for unit, obj in units.items():
            copies = [s for s in obj.symbols.values() if s.name == stop.symbol and s.section > 0]
            if not copies:
                continue
            result, = check_objects([replace(stop, unit=unit)], units, image,
                                     external_targets=targets, import_targets=imports)
            assert result["result"] == "pass", (unit, result)
        # A helper must not become another exported original entry.
        obj = units["src/time_lifecycle.cpp"]
        assert not any("close_impl" in s.name for s in obj.symbols.values())


def test_editbox_strict_call_target_rejects_calling_stop_instead_of_close(timer_objects):
    compiler, settings, objects = timer_objects
    image = original_image()
    source = ROOT / "src/editbox.cpp"
    before = source.read_bytes()
    assert before.count(b"time_.close();") == 1
    for trial, flags in enumerate(settings.flag_sets):
        units = {unit: trials[trial] for unit, trials in objects.items()}
        targets, _ = object_targets(read_objects(ROOT, image_pin(ROOT)), units)
        targets.update({"?close@Time@@QAEXXZ": 0x616780, "?stop@Time@@QAEXXZ": 0x616730,
                        symbol_at("src/graphicwin.cpp", 0x5D4E40): 0x5D4E40})
        def verify():
            return check_objects([binding("src/editbox.cpp", 0x614F30)], units, image,
                                 external_targets=targets)[0]
        assert verify()["result"] == "pass"
        # Use an out-of-line wrong callee so the control changes the target,
        # not the instruction count through expansion of the inline stop body.
        damaged_header = (ROOT / "src/time.h").read_bytes()
        begin = damaged_header.index(b"  MEASURED void stop()")
        end = damaged_header.index(b"  void close();", begin)
        damaged_header = damaged_header[:begin] + b"  void stop();\n" + damaged_header[end:]
        units["src/editbox.cpp"] = Coff(compiler.overlay(source, {
            "src/editbox.cpp": before.replace(b"time_.close();", b"time_.stop();"),
            "src/time.h": damaged_header}, (), flags + settings.layout_flags))
        bad = verify()
        assert bad["boundary"] == "pass"
        assert bad["bytes"] == bad["references"] == bad["result"] == "fail", bad
    assert source.read_bytes() == before


def test_live_timer_close_and_destructor_share_ordered_cleanup(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "timer_close_harness.cpp", [
        ("// LIVE_CTOR", "src/time.cpp", 0x6161D0),
        ("// LIVE_CLOSE", "src/time_lifecycle.cpp", 0x616780),
        ("// LIVE_DTOR", "src/time_lifecycle.cpp", 0x616200)], declarations=[
        ("// LIVE_DECL", "src/time.h", "class Time {", "#if defined(_M_IX86)"),
        ("// LIVE_HELPER", "src/time_lifecycle.cpp", "inline void Time::close_impl() {", "/*\nPurpose: Close")])


def test_live_control_rejects_omitted_destructor_cleanup(tmp_path, monkeypatch):
    reader = harness_support.read_file
    def damaged(path):
        result = []
        for record in reader(path):
            if record.address == 0x616200:
                assert record.region.count("close_impl();") == 1
                record = replace(record, region=record.region.replace("close_impl();", ""))
            result.append(record)
        return result
    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_timer_close_and_destructor_share_ordered_cleanup(tmp_path, monkeypatch)
