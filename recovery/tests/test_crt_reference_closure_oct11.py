"""Four reference-selected CRT archive entries, not provider or app proof."""
from dataclasses import replace
import csv
import hashlib
import os
from pathlib import Path
import subprocess

import pytest
import capstone

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "40e1077fe7333ae5860a543518a52fab43558c15"
ADDITION = "7b8c10a83990bfbac2b2e315cd49bf53ec34df58"
SOURCE = "src/vendor/vc6-runtime.recovery.h"
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARCHIVE_SHA256 = "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b"
EXPECTED = (
    (0x645615, 0x645646, "__fsopen", "build/intel/mt_obj/fopen.obj", 1,
     "6b624198122b05a59ea114ef15ef6514b0ff6e31390f356bfb1e960dabea35f4"),
    (0x645646, 0x645659, "_fopen", "build/intel/mt_obj/fopen.obj", 3,
     "6b624198122b05a59ea114ef15ef6514b0ff6e31390f356bfb1e960dabea35f4"),
    (0x64B1BE, 0x64B1EF, "_write_multi_char", "build/intel/mt_obj/output.obj", 7,
     "aaabeb2d6637972ea62f282394b45972ec6e6bdb3ac31f590da1acc7d046cf29"),
    (0x64CF73, 0x64CF8A, "__open", "build/intel/mt_obj/open.obj", 1,
     "757581b80e90ffc823121b7120e1436cc26ca8ea1e8f4f79f1070533d493985a"),
)
WIDE_FAMILY = (
    (0x645615, 0x645646, "__wfsopen", "build/intel/mt_obj/wfopen.obj", 1,
     "afbd16c47bbc8fc1feeca6809ec2296b16e40bf1113d1efa31196bb04140a5bd"),
    (0x645646, 0x645659, "__wfopen", "build/intel/mt_obj/wfopen.obj", 3,
     "afbd16c47bbc8fc1feeca6809ec2296b16e40bf1113d1efa31196bb04140a5bd"),
)
ALTERNATIVES = (
    (0x645615, 0x645646, "__wfsopen", "build/intel/mt_obj/wfopen.obj", 1,
     "afbd16c47bbc8fc1feeca6809ec2296b16e40bf1113d1efa31196bb04140a5bd"),
    (0x645646, 0x645659, "__wfopen", "build/intel/mt_obj/wfopen.obj", 3,
     "afbd16c47bbc8fc1feeca6809ec2296b16e40bf1113d1efa31196bb04140a5bd"),
    (0x64B1BE, 0x64B1EF, "_write_multi_char", "build/intel/mt_obj/woutput.obj", 4,
     "61a7defaa73b8b7be3547b2d84a07dcf6cfc439fed5f44a093a4df09eb9a5538"),
    (0x64CF73, 0x64CF8A, "__wopen", "build/intel/mt_obj/wopen.obj", 1,
     "51a30880e21c3d36f0373275b75fc2e0bd77f7e181b3d82e2d5ac21c2af263fd"),
)
OLD_UNKNOWN = (0x647815, 0x64A5DA, 0x64B227, 0x64B234, 0x64B244)


def historical():
    return parse_component(subprocess.check_output(
        ["git", "show", f"{BASE}:{SOURCE}"], cwd=ROOT, text=True))


def image_offset(data: bytes, address: int) -> int:
    pe = PeImage(data, hashlib.sha256(data).hexdigest())
    section, raw, _ = next((s, offset, size) for s, offset, size in pe.sections
                           if s.start <= address < s.start + size)
    return raw + address - section.start


@pytest.fixture(scope="module")
def inputs():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if "OPENSMACX_IMAGE" in os.environ:
        assert image_path.is_file(), "explicit pinned image path unavailable"
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("pinned original image or external LIBCMT archive unavailable")
    manifest = parse_component(subprocess.check_output(
        ["git", "show", f"{ADDITION}:{SOURCE}"], cwd=ROOT, text=True))
    image, archive = image_path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == IMAGE_SHA256
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_SHA256
    return manifest, image, archive, read_imports(ROOT)


def verify(inputs, manifest=None, image=None):
    current, original, archive, imports = inputs
    return verify_toolchain(current if manifest is None else manifest,
                            original if image is None else image, archive,
                            import_bindings=imports)


def result(report, address):
    return next(row for row in report["functions"]
                if int(row["address"], 16) == address)


def direct_calls(image: bytes, address: int, end: int):
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    pe = PeImage(image, hashlib.sha256(image).hexdigest())
    return {(insn.address, int(insn.op_str, 16))
            for insn in decoder.disasm(pe.read(Span(address, end)), address)
            if insn.mnemonic == "call" and insn.op_str.startswith("0x")}


def row_binding(manifest, address):
    return next(binding for binding in manifest.bindings
                if binding.origin.address == address)


def replace_binding(manifest, address, unit, symbol):
    return replace(manifest, bindings=tuple(
        replace(binding, unit=unit, symbol=symbol)
        if binding.origin.address == address else binding
        for binding in manifest.bindings))


def repin(manifest, image):
    digest = hashlib.sha256(image).hexdigest()
    return replace(
        manifest,
        bindings=tuple(replace(binding, origin=replace(binding.origin, image_sha256=digest))
                       for binding in manifest.bindings),
        data_sections=tuple(replace(binding, origin=replace(binding.origin, image_sha256=digest))
                            for binding in manifest.data_sections),
        code_sections=tuple(replace(section, origin=replace(section.origin, image_sha256=digest))
                            for section in manifest.code_sections))


def without_additions(manifest):
    addresses = {entry[0] for entry in EXPECTED}
    added = tuple(binding for binding in manifest.bindings
                  if binding.origin.address in addresses)
    assert tuple((b.origin.address, b.spans[0].end, b.symbol, b.unit)
                 for b in added) == tuple(entry[:4] for entry in EXPECTED)
    return replace(manifest, bindings=tuple(
        binding for binding in manifest.bindings
        if binding.origin.address not in addresses))


def test_addition_preserves_immutable_244_binding_prefix_and_source_claims(inputs):
    manifest, _, _, _ = inputs
    old = historical()
    assert len(old.bindings) == 244
    assert len(manifest.bindings) == 248
    assert replace(without_additions(manifest), code_sections=old.code_sections) == old
    current_source = subprocess.check_output(
        ["git", "show", f"{ADDITION}:{SOURCE}"], cwd=ROOT, text=True)
    previous_source = subprocess.check_output(
        ["git", "show", f"{BASE}:{SOURCE}"], cwd=ROOT, text=True)
    normalize = lambda text: [replace(row, line=1, region="")
                              for row in read_text(text, ROOT / SOURCE)]
    assert normalize(current_source) == normalize(previous_source)
    installed = parse_component((ROOT / SOURCE).read_text())
    assert installed.bindings[:len(manifest.bindings)] == manifest.bindings
    installed_rows = normalize((ROOT / SOURCE).read_text())
    assert all(row in installed_rows for row in normalize(current_source))


def test_physical_members_catalogue_extents_and_native_reference_closure(inputs):
    manifest, image, archive, _ = inputs
    members = CoffArchive(archive)
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as stream:
        catalogue = {int(row["address"], 16): row for row in csv.DictReader(stream)}
    for address, end, symbol, unit, section, digest in EXPECTED:
        assert int(catalogue[address]["end_address"], 16) == end
        member = members.member(unit)
        assert hashlib.sha256(member).hexdigest() == digest
        obj = Coff(member)
        function = obj.function(symbol, comdat=False)
        body, _, relocations = obj.subject(function)
        assert (function.section, function.value) == (section, 0)
        assert len(body) == end - address
        assert relocations

    before = verify(inputs, without_additions(manifest))
    after = verify(inputs)
    additions = {address for address, *_ in EXPECTED}
    assert [row for row in after["functions"]
            if int(row["address"], 16) not in additions] == before["functions"]
    for address, *_ in EXPECTED:
        current = result(after, address)
        assert all(current[key] == "pass"
                   for key in ("result", "boundary", "bytes", "references"))
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


def test_narrow_stdio_family_is_selected_by_both_independent_call_targets(inputs):
    manifest, image, archive, _ = inputs
    after = verify(inputs)
    fsopen = result(after, 0x645615)
    fopen = result(after, 0x645646)
    assert (0x645631, 0x649608) in direct_calls(image, 0x645615, 0x645646)
    assert (0x645650, 0x645615) in direct_calls(image, 0x645646, 0x645659)
    assert (0x64B1D8, 0x64B189) in direct_calls(image, 0x64B1BE, 0x64B1EF)
    assert (0x64CF81, 0x64CF8A) in direct_calls(image, 0x64CF73, 0x64CF8A)
    assert {item["symbol"] for item in fsopen["relocations"]} == {
        "__getstream", "__openfile", "__unlock_file"}
    assert [item["symbol"] for item in fopen["relocations"]] == ["__fsopen"]

    # Evaluate both wide entries together, retaining the rest of the current manifest.
    wide = manifest
    for address, _, symbol, unit, _, _ in WIDE_FAMILY:
        expected = next(entry for entry in ALTERNATIVES if entry[0] == address)
        assert expected[:4] == (address, expected[1], symbol, unit)
        wide = replace_binding(wide, address, unit, symbol)
    wide_report = verify(inputs, wide)
    assert result(wide_report, 0x645615)["boundary"] == "pass"
    assert result(wide_report, 0x645615)["result"] == "unknown"
    assert result(wide_report, 0x645646)["result"] == "pass"
    assert not all(result(wide_report, address)["result"] == "pass"
                   for address in (0x645615, 0x645646))
    assert result(wide_report, 0x645615)["relocations"][1]["symbol"] == "__wopenfile"
    assert result(wide_report, 0x645615)["relocations"][1]["result"] == "unknown"


def test_each_unselected_entry_alternative_has_its_own_physical_and_reference_check(inputs):
    manifest, _, archive, _ = inputs
    members = CoffArchive(archive)
    for address, end, symbol, unit, section, digest in ALTERNATIVES:
        obj_bytes = members.member(unit)
        assert hashlib.sha256(obj_bytes).hexdigest() == digest
        obj = Coff(obj_bytes)
        function = obj.function(symbol, comdat=False)
        body, _, _ = obj.subject(function)
        assert function.section == section and function.value == 0
        assert len(body) == end - address
        candidate = replace_binding(manifest, address, unit, symbol)
        candidate_row = result(verify(inputs, candidate), address)
        assert candidate_row["boundary"] == "pass"
        assert candidate_row["result"] == "unknown"
    wide_open = result(verify(inputs, replace_binding(
        manifest, 0x64CF73, "build/intel/mt_obj/wopen.obj", "__wopen")), 0x64CF73)
    assert [item["symbol"] for item in wide_open["relocations"]] == ["__wsopen"]
    assert wide_open["relocations"][0]["result"] == "unknown"


def test_wide_output_and_open_target_replacements_fail_boundary(inputs):
    manifest, _, archive, _ = inputs
    archive_view = CoffArchive(archive)
    cases = (
        (0x64B189, "build/intel/mt_obj/woutput.obj", "_write_char", 32,
         "61a7defaa73b8b7be3547b2d84a07dcf6cfc439fed5f44a093a4df09eb9a5538"),
        (0x649608, "build/intel/mt_obj/_wopen.obj", "__wopenfile", 374,
         "e29163180ea89333bf518481a9e7b24b7b101406d2d37c652b5b3becf259b4da"),
        (0x64CF8A, "build/intel/mt_obj/wopen.obj", "__wsopen", 720,
         "51a30880e21c3d36f0373275b75fc2e0bd77f7e181b3d82e2d5ac21c2af263fd"),
    )
    for address, unit, symbol, expected_size, digest in cases:
        binding = row_binding(manifest, address)
        member = archive_view.member(unit)
        assert hashlib.sha256(member).hexdigest() == digest
        obj = Coff(member)
        function = obj.function(symbol, comdat=False)
        body, _, _ = obj.subject(function)
        assert len(body) == expected_size
        if symbol == "__wopenfile":
            assert [(offset, kind) for offset, _, kind
                    in obj.sections[function.section - 1].relocations] == [
                        (17, 6), (326, 20), (349, 6)]
        candidate = replace_binding(manifest, address, unit, symbol)
        damaged_target = result(verify(inputs, candidate), address)
        assert damaged_target["boundary"] == "fail"
        assert damaged_target["result"] == "fail"
        assert binding.spans[0].end - binding.spans[0].start != expected_size


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
@pytest.mark.parametrize("damage", ("body", "extent", "member"))
def test_selected_attribution_damage_refuses(inputs, address, end, symbol, unit, section, digest, damage):
    manifest, image, _, _ = inputs
    if damage == "body":
        changed = bytearray(image)
        changed[image_offset(image, address)] ^= 1
        changed = bytes(changed)
        manifest = repin(manifest, changed)
        row = result(verify(inputs, manifest, changed), address)
        assert row["bytes"] == "fail" and row["result"] == "fail"
    else:
        manifest = replace(manifest, bindings=tuple(
            (replace(binding, spans=(Span(address, end - 1),)) if damage == "extent"
             else replace(binding, unit="build/intel/mt_obj/mlock.obj"))
            if binding.origin.address == address else binding
            for binding in manifest.bindings))
        row = result(verify(inputs, manifest), address)
        assert row["result"] != "pass"
        if damage == "extent":
            assert row["boundary"] != "pass"


@pytest.mark.parametrize("address", [entry[0] for entry in EXPECTED])
def test_selected_distinguishing_reference_damage_refuses(inputs, address):
    manifest, image, archive, _ = inputs
    binding = row_binding(manifest, address)
    obj = Coff(CoffArchive(archive).member(binding.unit))
    function = obj.function(binding.symbol, comdat=False)
    section = obj.sections[function.section - 1]
    expected_target = {
        0x645615: "__openfile", 0x645646: "__fsopen",
        0x64B1BE: "_write_char", 0x64CF73: "__sopen",
    }[address]
    sites = [offset for offset, index, kind in section.relocations
             if kind == 20 and obj.symbols[index].name == expected_target]
    assert len(sites) == 1
    changed = bytearray(image)
    changed[image_offset(image, address + sites[0])] ^= 1
    changed = bytes(changed)
    manifest = repin(manifest, changed)
    measured = result(verify(inputs, manifest, changed), address)
    assert measured["references"] == "fail" and measured["result"] == "fail"


def test_prior_unknown_runtime_identities_remain_unknown(inputs):
    manifest, _, _, _ = inputs
    after = verify(inputs)
    bound_addresses = {binding.origin.address for binding in manifest.bindings}
    assert not (bound_addresses & set(OLD_UNKNOWN))
    assert after["result"] == "unknown"
