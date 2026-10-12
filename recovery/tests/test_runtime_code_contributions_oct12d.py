"""Whole pinned LIBCMT contributions remain distinct from callable identities."""
from collections import Counter
from dataclasses import replace
import hashlib
import os
from pathlib import Path
import subprocess

import pytest

from match_recovery.coff_archive import CoffArchive
from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.components import Origin
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage


ROOT = Path(__file__).resolve().parents[2]
SOURCE = "src/vendor/vc6-runtime.recovery.h"
BASE = "bb26bc273d8a50ae0c578c783fa02d561acfc890"
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ARCHIVE_SHA256 = "1ef9c27b4f76031e40fcbdf7d518adc4969f90d6a00dd1ea88697907268c1b2b"
ARCHIVE_PATH = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")

# Original full-contribution placements and independently reviewed physical
# member/section hashes. These are measurement units, not function identities.
EXPECTED = (
    (0x64522C, 0x64531A, "build/intel/mt_obj/exsup.obj",
     "2477f227980a96f170950398ffdf8df79fda14fd66c43b3038a6ae2873d6e1c7",
     "b487a5e4e98d13b9e85c0878222ef37cf6aaa02f97da9e064fa9c861e9b35685"),
    (0x645550, 0x64557F, "build/intel/mt_obj/chkstk.obj",
     "dfebd0cac82ac321febe227a1363be138b5da5ba194ac95649f39995db610b4f",
     "4be32df90e8e3db3f5eed0d31857bddd88255c7ed09ae57e0e20e4510124a255"),
    (0x645DC0, 0x645E8C, "build/intel/mt_obj/strchr.obj",
     "68820eba347b92305d113bfe56ed5a7ca9988157dc3120b86926391a5f6b0d6c",
     "a4d43aa01f5b3db8839024433e2579f873e3cd2a234dbd89ac151292935a770c"),
    (0x6463D0, 0x64647E, "../build/intel/mt_obj/sin.obj",
     "cf248024a6a59b13e9041b9d8e29d961cd36ec126c366197af055044a7464797",
     "1be14f1387d4aa5dbe553ed90d6819064207b9de615c071bc65a6b0a89f7682a"),
    (0x646480, 0x64652E, "../build/intel/mt_obj/cos.obj",
     "12886f6a79df5deb7d5df22697d40cd0ba546f66e2af3ed8da53d3f7b3b35274",
     "5e029bb95747ab7d2864d74d82c4d31d9cb88f08d268498ca68a9f8f2b4c09f8"),
    (0x646530, 0x6465EA, "../build/intel/mt_obj/sqrt.obj",
     "fdfe7847e2fda91f6008ef48e5c953380cddfb26ad8749ae8acdd6864c1d3ab4",
     "f76b521a29ac6ac3c892ae8929567fbfd1fa621ebd4ac5f395fdb4be182c20e4"),
    (0x646650, 0x6466EF, "../build/intel/mt_obj/atan.obj",
     "dd00a43f947c8df0575e475b217dd14f81fbb97c3db1d64b3e65441a1d133824",
     "8665a57fef36d248b4d4da04b96231e8ae04e237770f80eb238a2f32d506101d"),
    (0x646A80, 0x646C9D, "../build/intel/mt_obj/pow.obj",
     "5b4aea1140036e7a6134935126bc896038a5097cc41ad9b8a4e86126ef459549",
     "ae6f959ad355a1a0ee5a416e55a96123dfef6581ac3847af4f905bfbcbd7126c"),
    (0x646DF0, 0x646ED0, "build/intel/mt_obj/exsup3.obj",
     "f0f21858bc4b6b923c2b0921bcaa58dbe4ad14ebf7f454c882ff69d4a25b34d6",
     "c4dfbcbbde3283aca4ad0e89fb755f2a6b4a8f458a02182b80ba8c2c8505a843"),
    (0x647AB0, 0x647B7B, "../build/intel/mt_obj/acos.obj",
     "5a1f8676194500681024f322e91962d130a5dadfc181483511f3382da18c6121",
     "0adabca58263e80693c802d583389a175adf3337e37edfcfcef06d37b2a9ca60"),
    (0x649110, 0x64915C, "build/intel/mt_obj/lowhelpr.obj",
     "9ed7a514f7739a576c0984975cc7ecea319015dedf33fbcef2a1bcd58bc91ad7",
     "e998bac163230900a6eeeb39676591c44a5e2cea84dfe7b4b1c974c787c535d4"),
    (0x64A610, 0x64A7DA, "../build/intel/mt_obj/87disp.obj",
     "3c2b8d2f775f4eb8b0bb71cf7204e0f5b84279f585d0cc4e6b14e318aac71743",
     "76c4e71831d881125972a5bf115f50670cac1396c8ef9d0d5c0fbafc2b5b24c9"),
    (0x64A7E0, 0x64A96C, "../build/intel/mt_obj/common.obj",
     "3b496f0a0a3a62142275eb7372be5fbf1eeb6c411e28aac708d51f5beda1e7b3",
     "61ca79142e6c78574c4dc0cbb69284abe24ee0de78af080d616772cc3205d60f"),
    (0x64A970, 0x64A9C3, "../build/intel/mt_obj/genexcep.obj",
     "186de7cd6dabc979883b661c9e61c089bb7deca6f338138092cfd9aca1ab8e91",
     "6df77e4c47c71625a427b7f1d3294bceef86c9c99f4dc4ab87aca480a9e5c699"),
    (0x64B260, 0x64B467, "../build/intel/mt_obj/87tran.obj",
     "b4841d400f0ffb8c851b4d5ef2dc05b58f847bb9c25ea987de9eee886661e45d",
     "da94e4fdea6266770b02cf37bc2c006a1ad699fb8639e8639886850e38a1c19e"),
    (0x64DDC0, 0x64EB4B, "../build/intel/mt_obj/adj_fdiv.obj",
     "01857e57cd623d64959be7b6c61b88df8b9fcc7747f80413ab9c87e4cabd2e45",
     "a5a1f52ee0fa14ac7ad32c71c69f7bcbe083b28f2013100f76526a71e247de43"),
    (0x64FD20, 0x64FDF0, "build/intel/mt_obj/stricmp.obj",
     "0dcd73fe65b95031b5c5c8fd1fb62b15ddde5e6286a51c66962e46a2efdcc175",
     "37f61b307a04ab04fd46e95dc8d7b9ba530ad796b2a36996b1b9dcbc2f5d929c"),
)


@pytest.fixture(scope="module")
def inputs():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if "OPENSMACX_IMAGE" in os.environ:
        assert image_path.is_file(), "explicit pinned original image is unavailable"
    if not image_path.is_file() or not ARCHIVE_PATH.is_file():
        pytest.skip("external pinned original image and LIBCMT archive required")
    manifest = parse_component((ROOT / SOURCE).read_text())
    image, archive = image_path.read_bytes(), ARCHIVE_PATH.read_bytes()
    assert hashlib.sha256(image).hexdigest() == IMAGE_SHA256
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_SHA256 == manifest.archive_sha256
    return manifest, image, archive


def test_seventeen_full_contributions_preserve_the_complete_prior_manifest(inputs):
    manifest, _, _ = inputs
    historical_text = subprocess.check_output(
        ["git", "show", f"{BASE}:{SOURCE}"], cwd=ROOT, text=True)
    historical = parse_component(historical_text)
    assert len(historical.bindings) == 254 and not historical.code_sections
    assert replace(manifest, code_sections=historical.code_sections) == historical
    assert parse_component(render_component(manifest)) == manifest

    current_text = (ROOT / SOURCE).read_text()
    begin, end = "// RECOVERY_COMPONENT_BEGIN v1", "// RECOVERY_COMPONENT_END"
    assert current_text.split(begin, 1)[0] == historical_text.split(begin, 1)[0]
    assert current_text.split(end, 1)[1] == historical_text.split(end, 1)[1]

    sections = manifest.code_sections
    assert len(sections) == len(EXPECTED) == 17
    assert [(item.span.start, item.span.end, item.unit) for item in sections] == [
        (start, end, unit) for start, end, unit, *_ in EXPECTED]
    assert all(item.section == 1 and item.section_name == ".text" for item in sections)
    assert all(item.origin.image_sha256 == IMAGE_SHA256 for item in sections)
    assert all("original archive-member selection" in item.evidence.casefold()
               and "final-link retention/allocation" in item.evidence.casefold()
               and "relocation-target closure" in item.evidence.casefold()
               and "runtime version" in item.evidence.casefold()
               and "not a callable identity" in item.evidence.casefold()
               and "per-symbol extent" in item.evidence.casefold()
               for item in sections)
    genexcep = next(item for item in sections if item.unit.endswith("/genexcep.obj"))
    assert "__87except relocation is resolved through an existing source binding" in genexcep.evidence
    intervals = sorted((item.span.start, item.span.end) for item in sections)
    assert all(end <= start for (_, end), (start, _) in zip(intervals, intervals[1:]))


def test_physical_sections_and_native_verification_keep_reference_uncertainty(inputs):
    manifest, image_data, archive_data = inputs
    archive = CoffArchive(archive_data)
    image = PeImage(image_data, IMAGE_SHA256)
    assert len(manifest.code_sections) == 17
    no_relocation_count = 0
    for binding, (start, end, unit, member_sha, section_sha) in zip(
            manifest.code_sections, EXPECTED, strict=True):
        assert (binding.span.start, binding.span.end, binding.unit) == (start, end, unit)
        member = archive.member(unit)
        assert hashlib.sha256(member).hexdigest() == member_sha
        obj = Coff(member)
        section = obj.sections[binding.section - 1]
        assert section.name == binding.section_name
        code, relocations, _alignment = obj.section_subject(binding.section)
        assert len(code) == end - start
        assert hashlib.sha256(code).hexdigest() == section_sha
        if not relocations:
            assert image.read(binding.span) == code
        if not relocations:
            no_relocation_count += 1
        physical_symbols = {
            (symbol.name, symbol.value) for symbol in obj.symbols.values()
            if symbol.section == binding.section and symbol.type & 0x20
            and symbol.storage in (2, 3)
        }
        assert physical_symbols
        assert all(0 <= offset < len(code) for _, offset in physical_symbols)
    assert no_relocation_count == 2

    report = verify_toolchain(manifest, image_data, archive_data)
    assert report["archive"]["result"] == "pass"
    assert report["result"] == "unknown"
    assert report["ready_for_collection"] is False
    assert report["code_section_counts"] == {"pass": 3, "unknown": 14}
    rows = report["code_sections"]
    assert len(rows) == 17 and not any(row["result"] == "fail" for row in rows)
    assert all(not row["callable"] and not row["link_target"] for row in rows)
    assert all(row["nonrelocation_bytes"] == "pass" for row in rows)
    assert sum(row["references"] == "unknown" for row in rows) == 14


def test_nonrelocation_damage_fails_the_native_whole_section_check(inputs):
    manifest, original, archive = inputs
    target = next(item for item in manifest.code_sections
                  if item.unit == "build/intel/mt_obj/chkstk.obj")
    obj = Coff(CoffArchive(archive).member(target.unit))
    _code, relocations, _alignment = obj.section_subject(target.section)
    assert not relocations

    image = PeImage(original, IMAGE_SHA256)
    region, file_offset, raw_size = next(
        (region, file_offset, raw_size)
        for region, file_offset, raw_size in image.sections
        if region.start <= target.span.start and target.span.end <= region.end)
    changed_at = file_offset + target.span.start - region.start
    assert changed_at < file_offset + raw_size
    damaged_image = bytearray(original)
    damaged_image[changed_at] ^= 1
    damaged_image = bytes(damaged_image)
    damaged_pin = hashlib.sha256(damaged_image).hexdigest()
    changed_target = replace(target,
        origin=Origin(damaged_pin, "pe:va", target.span.start))
    scoped = replace(manifest, bindings=(), absolute_symbols=(), data_sections=(),
                     code_sections=(changed_target,))

    report = verify_toolchain(scoped, damaged_image, archive)
    row, = report["code_sections"]
    assert row["boundary"] == "pass"
    assert row["bytes"] == row["nonrelocation_bytes"] == "fail"
    assert row["result"] == "fail" and report["result"] == "fail"
