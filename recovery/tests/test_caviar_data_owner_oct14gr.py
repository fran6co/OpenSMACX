from pathlib import Path
import re
import subprocess

from match_recovery.source.reader import read


ROOT = Path(__file__).resolve().parents[2]


def test_caviar_data_method_family_has_one_out_of_line_owner():
    header = (ROOT / "src/caviar.h").read_text()
    renderer = (ROOT / "src/caviar.cpp").read_text()
    owner = (ROOT / "src/caviar_data.cpp").read_text()

    methods = (
        r"CaviarData::CaviarData\s*\(",
        r"void CaviarData::close\s*\(",
        r"int CaviarData::init\s*\(",
        r"int CaviarData::apply_palette\s*\(",
        r"void CaviarData::release_palette_chunk\s*\(",
    )
    for method in methods:
        assert len(re.findall(method, owner)) == 1
        assert not re.search(method, renderer)

    assert re.search(r"\bCaviarData\(\);", header)
    assert re.search(r"~CaviarData\(\)\s*\{\s*close\(\);\s*\}", header)
    assert "MEASURED CaviarData" not in header
    assert "const CvrRecord *record() const { return record_; }" in header
    assert "static_assert(sizeof(CaviarData) == 0xC" in header


def test_caviar_data_identity_and_global_definition_order_are_preserved():
    renderer = (ROOT / "src/caviar.cpp").read_text()
    objects = (ROOT / "src/caviar_objects.cpp").read_text()
    renderer_objects = (ROOT / "src/caviar_renderer_objects.cpp").read_text()
    owner = (ROOT / "src/caviar_data.cpp").read_text()
    before = subprocess.check_output(
        ["git", "show", "69759b7d1c8664a463d562eaa3b5df2f6b6400a1:src/caviar.cpp"],
        cwd=ROOT, text=True,
    )
    definitions = r"^(?:CaviarData|Caviar) \w+(?:\[\d+\])?;[^\n]*$"
    original_definitions = re.findall(definitions, before, re.MULTILINE)
    assert len(original_definitions) == 99
    assert re.findall(definitions, objects, re.MULTILINE) == original_definitions[:-1]
    assert re.findall(definitions, renderer_objects, re.MULTILINE) == original_definitions[-1:]
    assert not re.findall(definitions, renderer, re.MULTILINE)
    assert not re.findall(definitions, owner, re.MULTILINE)
    def object_bindings(source):
        lines = source.splitlines()
        return [
            (lines[index], lines[index + 1])
            for index, line in enumerate(lines[:-1])
            if line.startswith("// RECOVERY_OBJECT: ")
            and re.match(r"^(?:CaviarData \w+(?:\[\d+\])?|Caviar g_VEHDRAW_CAVIAR);",
                         lines[index + 1])
        ]
    assert object_bindings(objects) + object_bindings(renderer_objects) == object_bindings(before)

    # Existing original identities/claims moved with the out-of-line method
    # family; its formerly header-defined constructor keeps its original span.
    for identity in (
        "0x00616BC0 ??0CaviarData@@QAE@XZ 0x00616BC0-0x00616BCD BYTE_EXACT",
        "0x00616BD0 ?init@CaviarData@@QAEHPAD@Z 0x00616BD0-0x00616C55 FILE BYTE_EXACT",
        "0x00616C60 ?close@CaviarData@@QAEXXZ 0x00616C60-0x00616C7C BYTE_EXACT",
        "0x00616D40 sub_616d40 0x00616D40-0x00616D66 BYTE_EXACT",
    ):
        assert owner.count(identity) == 1

    assert objects.count(
        "0x00456100 ??1CaviarData@@QAE@XZ 0x00456100-0x00456105 FILE BYTE_EXACT"
    ) == 1
    assert "0x00456100 ??1CaviarData@@QAE@XZ 0x00456100-0x00456105 FILE BYTE_EXACT" not in renderer
    assert "Caviar g_VEHDRAW_CAVIAR;" not in objects
    assert renderer_objects.count("Caviar g_VEHDRAW_CAVIAR;") == 1
    assert objects.count("CaviarData g_") == 98
    assert "Caviar g_VEHDRAW_CAVIAR;" not in renderer
    assert "Caviar::Caviar()" in renderer
    assert "VoxInstance::VoxInstance()" in renderer


def test_build_lists_application_caviar_object_owner_before_engine():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    project = (ROOT / "recovery/project.json").read_text()

    assert cmake.index("    src/caviar_objects.cpp") < cmake.index(
        "    src/caviar.cpp")
    assert "    src/caviar_data.cpp\n" in cmake
    assert '"src/caviar.cpp"' not in project
    assert '"src/caviar_objects.cpp": {' in project
    assert (cmake.index("    src/caviar_objects.cpp")
            < cmake.index("    src/texture_objects.cpp")
            < cmake.index("    src/sprite_objects.cpp")
            < cmake.index("    src/caviar_renderer_objects.cpp"))


def test_native_reader_follows_moved_method_identities_and_claims():
    records = (read(ROOT / "src/caviar.cpp") + read(ROOT / "src/caviar_data.cpp")
               + read(ROOT / "src/caviar_objects.cpp")
               + read(ROOT / "src/caviar_renderer_objects.cpp"))
    expected = {
        0x456100: ("src/caviar_objects.cpp", "src/caviar.h", True),
        0x44D170: ("src/caviar_objects.cpp", "", True),
        0x44D190: ("src/caviar_objects.cpp", "", True),
        0x4BF6E0: ("src/caviar_renderer_objects.cpp", "", True),
        0x4BF700: ("src/caviar_renderer_objects.cpp", "", True),
        0x616BC0: ("src/caviar_data.cpp", "", True),
        0x616BD0: ("src/caviar_data.cpp", "", True),
        0x616C60: ("src/caviar_data.cpp", "", True),
        0x616C80: ("src/caviar_data.cpp", "", False),
        0x616D40: ("src/caviar_data.cpp", "", True),
    }
    for address, (path, body, exact) in expected.items():
        matches = [record for record in records if record.address == address]
        assert len(matches) == 1
        record = matches[0]
        assert record.path.relative_to(ROOT).as_posix() == path
        assert record.body == body
        assert record.byte_exact is exact
