"""Real paired code/storage closure: native physical proofs, never PASS mocks."""

import struct
import json

import pytest

from match_recovery.generated_code_residual import _compare_contexts
from test_common_request_coverage import common_context, append_section
from test_linked_initializers import update_artifact
from test_projected_initializer_order import selection
from test_slot_local_initializer_order import replace_object


CHANGES = [selection(unit="src/unit0.cpp", version=version) for version in ("baseline", "current")]


@pytest.fixture(autouse=True)
def configured_sources(monkeypatch):
    # common_context's native receipt fixture has build configuration only.
    # Add genuine source/pin configuration before its source epoch is sealed.
    import test_startup_linked_consumption as startup
    original = startup.CommandContext
    def commands(root, names):
        result = original(root, names)
        path = root / "recovery/project.json"
        config = json.loads(path.read_bytes())
        config.update(schema_version=1, source={"image_pin": "recovery/image.pin",
                       "address_space": "synthetic:va", "roots": ["src"]})
        path.write_text(json.dumps(config))
        pin = root / "recovery/image.pin"
        pin.write_text("sha256 " + "a" * 64 + "\n")
        before = {item: item.read_bytes() for item in (path, pin)}
        check = result.check_current
        def fresh():
            check()
            if any(item.read_bytes() != raw for item, raw in before.items()):
                raise ValueError("source configuration epoch changed")
        result.check_current = fresh
        return result
    monkeypatch.setattr(startup, "CommandContext", commands)


def add_read(context, addend):
    raw = bytearray(context.objects[0].read_bytes())
    start, = struct.unpack_from("<I", raw, 20 + 20)
    struct.pack_into("<i", raw, start + 1, addend)
    replace_object(context, bytes(raw), index=0)
    image = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", image, 513, 0x402080 + addend)
    update_artifact(context, "app.exe", bytes(image))


def common_pair(root, *, size=4, missing=False, nonzero=False, addend=0):
    def damage(context):
        if missing:
            mapping = (context.directory / "app.map").read_text()
            update_artifact(context, "app.map", "".join(row for row in mapping.splitlines(keepends=True)
                                                       if " _g " not in row).encode())
        if nonzero:
            raw = bytearray((context.directory / "app.exe").read_bytes())
            raw[0x480] = 1
            update_artifact(context, "app.exe", bytes(raw))
    old = common_context(root / "old", request=True, damage=lambda context: add_read(context, addend))[0]
    new = common_context(root / "new", witness_size=size, damage=damage)[0]
    return old, new


def test_common_used_only_by_removed_old_root_is_retained_and_verified_both_sides(tmp_path):
    result = _compare_contexts(*common_pair(tmp_path), CHANGES)
    assert result["result"] == "pass", result.get("reason", result)
    storage = result["storage_residual"]
    assert storage["result"] == storage["input_consistency"] == "pass"
    assert storage["sections"] == 1 and result["omitted_bodies"] == {"baseline": 1, "current": 1}
    for version in ("baseline", "current"):
        row, = storage["dependencies"][version].values()
        assert row["canonical"] == {"kind": "common", "size": 4, "initialization": "zero", "flags": 0xC0000040}
    assert storage["allocation_isolation"] == storage["remaining_linked_effects"] == "unknown"
    assert not storage["ready_for_collection"]


def test_independently_valid_changed_common_extent_is_cross_build_failure_not_corruption(tmp_path):
    result = _compare_contexts(*common_pair(tmp_path, size=8), CHANGES)
    assert result["result"] == "pass", result
    storage = result["storage_residual"]
    assert storage["result"] == "fail" and storage["input_consistency"] == "pass"
    assert storage["equal"] is False


def test_absent_current_common_is_unknown_not_inferred_discard(tmp_path):
    result = _compare_contexts(*common_pair(tmp_path, missing=True), CHANGES)
    assert result["result"] == "pass", result
    assert result["storage_residual"]["result"] == "unknown"


def test_nonzero_current_common_is_a_retained_physical_counterexample(tmp_path):
    result = _compare_contexts(*common_pair(tmp_path, nonzero=True), CHANGES)
    assert result["result"] == "pass", result
    storage = result["storage_residual"]
    assert storage["result"] == storage["input_consistency"] == "fail"


def ordinary_context(root, *, contents=b"ordinary-storage", mismatch=False, callback=False,
                     cycle=False, missing=False, callback_code=b"\xc3", read_addend=0,
                     data_addend=0, missing_callback_definition=False, unsupported=False):
    def configure(context):
        raw = bytearray(context.objects[1].read_bytes())
        symbols_at, = struct.unpack_from("<I", raw, 8)
        # Existing dependency _g becomes a genuine independently placed data
        # definition. Its generated referring root retains an undefined _g.
        data = (struct.pack("<I", data_addend) + contents[4:]) if callback or cycle else contents
        if cycle:
            count, = struct.unpack_from("<I", raw, 12)
            end = symbols_at + 18 * count
            raw[end:end] = struct.pack("<8sIhHBB", b"_h", 0, 0, 0, 2, 0)
            struct.pack_into("<I", raw, 12, count + 1)
        raw = append_section(bytes(raw), data=data, references=((0, 0 if callback else 2, 6),)
                             if callback or cycle else ())
        if cycle:
            raw = bytearray(append_section(raw, data=bytes(8), references=((0, 1, 6),)))
            raw[100:108] = b".rdata\0\0"
        raw = bytearray(raw)
        symbols_at, = struct.unpack_from("<I", raw, 8)
        struct.pack_into("<h", raw, symbols_at + 18 + 12, 2)
        if cycle:
            struct.pack_into("<h", raw, symbols_at + 36 + 12, 3)
        if unsupported:
            raw[60:68] = b".tls\0\0\0\0"
        if callback_code != b"\xc3":
            from match_recovery.pe_coff import Coff
            parsed = Coff(bytes(raw))
            at, = struct.unpack_from("<I", raw, 20 + 20)
            raw = bytearray(raw)
            raw[at:at + len(parsed.sections[0].data)] = callback_code.ljust(len(parsed.sections[0].data), b"\xcc")
        if missing_callback_definition:
            assert callback and not cycle
            count, = struct.unpack_from("<I", raw, 12)
            raw[symbols_at:symbols_at + 8] = b"_lost\0\0\0"
            end = symbols_at + count * 18
            raw[end:end] = struct.pack("<8sIhHBB", b"_dep", 0, 0, 0x20, 2, 0)
            struct.pack_into("<I", raw, 12, count + 1)
            relocation_at, = struct.unpack_from("<I", raw, 60 + 24)
            struct.pack_into("<I", raw, relocation_at + 4, count)
        replace_object(context, bytes(raw), index=1)
        mapping = (context.directory / "app.map").read_text().replace("_g 00402080 <common>", "_g 00402080 dependency.obj")
        if cycle:
            mapping = mapping.replace(" entry point", " 0002:000000a0 _h 004020a0 dependency.obj\n entry point")
        if missing:
            mapping = "".join(row for row in mapping.splitlines(keepends=True) if " _g " not in row)
        update_artifact(context, "app.map", mapping.encode())
        image = bytearray((context.directory / "app.exe").read_bytes())
        image[0x480:0x480 + len(data)] = data
        data_sites = [0x3004]
        if callback or cycle:
            struct.pack_into("<I", image, 0x480, (0x4011E0 if callback else 0x4020A0) + data_addend)
            data_sites.append(0x3080)
        if cycle:
            image[0x4A0:0x4A8] = struct.pack("<I", 0x402080) + bytes(4)
            data_sites.append(0x30A0)
        if callback_code != b"\xc3":
            image[0x3E0:0x3F0] = callback_code.ljust(16, b"\xcc")
        if mismatch:
            image[0x48F] ^= 1
        data_sites += [0] * (len(data_sites) % 2)
        relocations = (struct.pack("<II2H", 0x1000, 12, 0x3001, 0)
                       + struct.pack("<II", 0x2000, 8 + 2 * len(data_sites))
                       + struct.pack("<" + "H" * len(data_sites), *data_sites))
        image[0x380:0x380 + len(relocations)] = relocations
        struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
        update_artifact(context, "app.exe", bytes(image))
        if read_addend:
            add_read(context, read_addend)
    return common_context(root, request=True, request_size=0, witness=False, damage=configure)[0]


@pytest.mark.parametrize("damage", [None, "contents", "image", "missing"])
def test_ordinary_data_behind_unchanged_code_is_consumed_independently(tmp_path, damage):
    before = ordinary_context(tmp_path / "old")
    after = ordinary_context(tmp_path / "new", contents=b"changed-storage!" if damage == "contents" else b"ordinary-storage",
                             mismatch=damage == "image", missing=damage == "missing")
    result = _compare_contexts(before, after, [])
    if damage == "missing":
        assert result["result"] == "unknown", result
        return
    assert result["result"] == "pass", result
    storage = result["storage_residual"]
    assert storage["result"] == ("pass" if damage is None else "fail"), storage
    assert storage["input_consistency"] == ("fail" if damage == "image" else "pass")
    assert storage["sections"] == 1


@pytest.mark.parametrize("changed", [False, True])
def test_callback_pointer_extends_actual_code_dependency_union(tmp_path, changed):
    before = ordinary_context(tmp_path / "old", callback=True)
    after = ordinary_context(tmp_path / "new", callback=True, callback_code=b"\x90\xc3" if changed else b"\xc3")
    result = _compare_contexts(before, after, CHANGES)
    assert result["result"] == ("fail" if changed else "pass"), result
    storage = result["storage_residual"]
    assert storage["result"] == storage["code_coverage"] == "pass"
    assert len(storage["required_code"]) == 1
    assert result["preserved_producers"] == 1


def test_two_physical_data_sections_form_a_finite_preserved_cycle(tmp_path):
    result = _compare_contexts(ordinary_context(tmp_path / "old", cycle=True),
                               ordinary_context(tmp_path / "new", cycle=True), CHANGES)
    assert result["result"] == "pass", result
    storage = result["storage_residual"]
    assert storage["result"] == storage["input_consistency"] == "pass", storage
    assert storage["sections"] == 2
    assert all(len(rows) == 2 for rows in storage["dependencies"].values())


@pytest.mark.parametrize("kind", ["common", "ordinary", "data_to_data", "data_to_code"])
def test_effective_reference_outside_proved_target_extent_remains_unknown(tmp_path, kind):
    if kind == "common":
        values = common_pair(tmp_path, addend=32)
    else:
        options = ({"read_addend": 32} if kind == "ordinary" else
                   {"cycle": True, "data_addend": 32} if kind == "data_to_data" else
                   {"callback": True, "data_addend": 32})
        values = [ordinary_context(tmp_path / version, **options) for version in ("old", "new")]
    result = _compare_contexts(*values, CHANGES)
    assert result["result"] == "pass", result
    assert result["storage_residual"]["result"] == "unknown", result["storage_residual"]
    assert "extent" in result["storage_residual"]["reason"]


def test_missing_callback_cannot_erase_already_observed_physical_data_failure(tmp_path):
    before = ordinary_context(tmp_path / "old", callback=True)
    after = ordinary_context(tmp_path / "new", callback=True, mismatch=True, missing_callback_definition=True)
    result = _compare_contexts(before, after, CHANGES)
    assert result["result"] == "unknown", result
    assert result["storage_residual"]["result"] == result["storage_residual"]["input_consistency"] == "fail", result


def test_unsupported_data_does_not_pass_as_an_ordinary_closed_dependency(tmp_path):
    result = _compare_contexts(ordinary_context(tmp_path / "old", unsupported=True),
                               ordinary_context(tmp_path / "new", unsupported=True), CHANGES)
    assert result["result"] == "pass", result
    assert result["storage_residual"]["result"] == "unknown", result["storage_residual"]
