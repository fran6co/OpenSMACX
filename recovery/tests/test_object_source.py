from dataclasses import replace

import pytest

from match_recovery.object_source import read_objects, object_targets
from match_recovery.pe_coff import Coff, Section
from test_binary import coff_fixture


PIN = "a" * 64


@pytest.fixture
def source(tmp_path, source_configuration):
    (tmp_path / "src").mkdir()
    (tmp_path / "src/object.cpp").write_text(
        "// RECOVERY_OBJECT: v1 0x00402000 _g\nint g;\n")
    return tmp_path


def test_source_object_annotation_owns_start_not_extent_or_current_verdict(source):
    binding, = read_objects(source, PIN)
    assert binding.origin.address == 0x402000
    assert binding.origin.image_sha256 == PIN
    assert binding.symbol == "_g" and binding.unit == "src/object.cpp"
    assert not hasattr(binding, "result") and not hasattr(binding, "size")


@pytest.mark.parametrize("text", ["// RECOVERY_OBJECT: v2 0x00402000 _g",
                                   "// RECOVERY_OBJECT: v1 420000 _g",
                                   "// RECOVERY_OBJECT: v1 0x00402000 _g pass"])
def test_invalid_or_verdict_bearing_annotations_are_refused(source, text):
    (source / "src/object.cpp").write_text(text)
    with pytest.raises(ValueError):
        read_objects(source, PIN)


@pytest.mark.parametrize("extra", ["0x00402000 _other", "0x00403000 _g"])
def test_duplicate_objects_cannot_silently_alias_storage(source, extra):
    (source / "src/other.cpp").write_text("// RECOVERY_OBJECT: v1 " + extra)
    with pytest.raises(ValueError):
        read_objects(source, PIN)


def test_seed_annotations_do_not_override_the_product_object(source):
    (source / "src/unrecovered").mkdir()
    (source / "src/unrecovered/seed.cpp").write_text("// RECOVERY_OBJECT: v1 0x00402000 _g")
    assert len(read_objects(source, PIN)) == 1


def test_object_needs_its_owners_actual_compiled_data_definition(source):
    bindings = read_objects(source, PIN)
    assert object_targets(bindings, {}) == ({}, [])
    obj = Coff(coff_fixture())  # _g is only an undefined external here
    targets, checks = object_targets(bindings, {"src/object.cpp": obj})
    assert not targets and checks[0]["definition"] == "unknown"
    obj.sections += (Section(bytes(4), (), 0xC0300040),)
    obj.symbols[1] = replace(obj.symbols[1], section=2)
    targets, checks = object_targets(bindings, {"src/object.cpp": obj})
    assert targets == {"_g": 0x402000} and checks[0]["definition"] == "pass"
    obj.symbols[1] = replace(obj.symbols[1], type=0x20)
    assert not object_targets(bindings, {"src/object.cpp": obj})[0]


def test_object_mapping_remains_visible_to_storage_audit(source):
    from match_recovery.cpp_storage_audit import storage_findings
    (source / "src/other.cpp").write_text("int Other; // 0x00402000\n")
    files = {p.relative_to(source).as_posix(): p.read_text() for p in (source / "src").glob("*.cpp")}
    counts, _, rows = storage_findings(files, pin=PIN, address_space="pe:va", symbol_format="msvc")
    assert counts["duplicate global identities"] == 1
    assert rows[0]["symbols"] == ["Other", "g"]
