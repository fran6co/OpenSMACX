"""Physical slot-local root controls; no compiler or runtime acceptance.

Only the compiler-receipt fixture's input construction is wrapped. Native COFF,
PE, link evidence, source binding and order verification remain real.
"""

import json
import struct

import pytest

from match_recovery.linked_initializers import compare_build_initializers, observe_build_initializers
from match_recovery.pe_coff import Coff
from test_build_graph_commands import Context as CommandContext, stamp
from test_linked_code_graph import function, object_bytes
from test_linked_initializers import update_artifact
from test_projected_initializer_order import repeated_root, swap_slots
import test_startup_linked_consumption as native


def physical_context(monkeypatch, root, *, source_files=None, configured=True, **kwargs):
    """Establish source/configuration inputs before sealing the native epoch."""
    source_files = source_files or {}

    def commands(root, names):
        result = CommandContext(root, names)
        if configured:
            project = root / "recovery/project.json"
            value = json.loads(project.read_text())
            value.update(schema_version=1, source={"image_pin": "recovery/image.pin",
                         "address_space": "synthetic:va", "roots": ["src"],
                         "seed_roots": ["src/seeds"]})
            project.write_text(json.dumps(value))
            (root / "recovery/image.pin").write_text("sha256 " + "a" * 64 + "\n")
        for name, data in source_files.items():
            path = root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        for row in result.proof["compilations"]:
            for entry in row["compilation"]["inputs"]:
                path = root / entry["path"]
                entry["before"] = entry["after"] = stamp(path.read_bytes())
        return result

    with monkeypatch.context() as construction:
        construction.setattr(native, "CommandContext", commands)
        linked, _ = native.context(root, **kwargs)
    paths = [root / "recovery/project.json", *sorted((root / "src").rglob("*"))]
    if configured:
        paths.append(root / "recovery/image.pin")
    pins = {path: stamp(path.read_bytes()) for path in paths if path.is_file()}
    source_names = {path.relative_to(root) for path in (root / "src").rglob("*") if path.is_file()}
    native_guard = linked.check_current

    def current():
        native_guard()
        if (any(not path.is_file() or stamp(path.read_bytes()) != expected for path, expected in pins.items())
                or source_names != {path.relative_to(root) for path in (root / "src").rglob("*") if path.is_file()}):
            raise ValueError("synthetic source/configuration epoch changed")

    linked.check_current = current
    return linked


def replace_object(linked, raw, index=0):
    path = linked.objects[index]
    path.write_bytes(raw)
    receipt = json.loads((linked.directory / "link.json").read_text())
    row = next(row for row in receipt["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    update_artifact(linked, "link.json", json.dumps(receipt).encode())


def rename_root(linked, index=0):
    path = linked.objects[index]
    raw = path.read_bytes()
    assert raw.count(b"_$E1") == 1
    replace_object(linked, raw.replace(b"_$E1", b"_$E9"), index)
    lines = (linked.directory / "app.map").read_text().splitlines(keepends=True)
    matches = [i for i, line in enumerate(lines) if " _$E1 " in line and line.rstrip().endswith(" " + path.name)]
    assert len(matches) == 1
    lines[matches[0]] = lines[matches[0]].replace(" _$E1 ", " _$E9 ")
    update_artifact(linked, "app.map", "".join(lines).encode())


def make_pair(monkeypatch, tmp_path, *, old_damage=None, new_damage=None, old_sources=None,
              new_sources=None, configured=True, codes=(b"\xc3",), references=None):
    def changed(linked):
        if new_damage:
            new_damage(linked)
        rename_root(linked)

    old = physical_context(monkeypatch, tmp_path / "old", configured=configured, codes=codes,
                           references=references, damage=old_damage, source_files=old_sources)
    new = physical_context(monkeypatch, tmp_path / "new", configured=configured, codes=codes,
                           references=references, damage=changed, source_files=new_sources)
    return old, new


def compare(pair):
    return compare_build_initializers(*pair, generated_changes=[])


def assert_observed(pair):
    for linked in pair:
        result = observe_build_initializers(linked)
        assert result["result"] == "pass", result
        assert result["code_dependencies"]["result"] == "pass", result


@pytest.mark.parametrize("code,reference", [(b"\xc3", None), (b"\xe8" + bytes(4) + b"\xc3", "_dep")])
def test_native_slot_local_rename_preserves_only_bounded_order(monkeypatch, tmp_path, code, reference):
    pair = make_pair(monkeypatch, tmp_path, codes=(code,), references=(reference,))
    assert_observed(pair)
    result = compare(pair)
    order = result["projected_order"]
    assert order["result"] == "pass", order
    before, = order["baseline"]["sequence"]
    after, = order["current"]["sequence"]
    assert before["identity"]["symbol"] == "_$E1" and after["identity"]["symbol"] == "_$E9"
    assert order["root_renamings"] == [{"index": 0, "unit": "src/unit0.cpp", "baseline_symbol": "_$E1",
                                       "current_symbol": "_$E9", "body_fingerprint": before["body_fingerprint"]}]
    assert result["remaining_linked_effects"] == "unknown"
    assert not result["ready_for_collection"]


def test_missing_source_configuration_keeps_exact_symbol_mismatch(monkeypatch, tmp_path):
    result = compare(make_pair(monkeypatch, tmp_path, configured=False))["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


def test_equal_bodies_cannot_exchange_source_owners(monkeypatch, tmp_path):
    pair = make_pair(monkeypatch, tmp_path, codes=(b"\xc3", b"\xc3"), new_damage=swap_slots)
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


@pytest.mark.parametrize("path", ["src/unit0.cpp", "src/owner.h", "src/seeds/owner.cpp"])
@pytest.mark.parametrize("version", ["baseline", "current"])
@pytest.mark.parametrize("annotation", [
    b"// ORIGINAL: 0x00401000 _$E9 0x00401000-0x00401010\nvoid function() {}\n",
    b"// RECOVERY_OBJECT: v1 0x00401000 _$E9\nint object;\n",
    b"// RECOVERY_OBJECT: v1 0x00402004 _$S1\nint slot;\n",
])
def test_original_function_or_object_identity_blocks_local_renaming(monkeypatch, tmp_path, path, version, annotation):
    sources = {path: annotation.replace(b"_$E9", b"_$E1") if version == "baseline" else annotation}
    pair = make_pair(monkeypatch, tmp_path, **{("old_sources" if version == "baseline" else "new_sources"): sources})
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


@pytest.mark.parametrize("public", [False, True])
def test_linked_public_or_other_owner_alias_prevents_normalization(monkeypatch, tmp_path, public):
    def alias(linked):
        mapping = (linked.directory / "app.map").read_text()
        line = " 0001:00000000 _alias 00401000 f dependency.obj\n"
        mapping = mapping.replace(" entry point", line + " entry point") if public else mapping + line
        update_artifact(linked, "app.map", mapping.encode())

    pair = make_pair(monkeypatch, tmp_path, new_damage=alias)
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


def test_malformed_export_directory_prevents_normalization(monkeypatch, tmp_path):
    def export(linked):
        image = bytearray((linked.directory / "app.exe").read_bytes())
        struct.pack_into("<II", image, 0x98 + 96, 0x2040, 40)
        update_artifact(linked, "app.exe", image)

    pair = make_pair(monkeypatch, tmp_path, new_damage=export)
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


def export_target(linked, target, *, named):
    image = bytearray((linked.directory / "app.exe").read_bytes())
    struct.pack_into("<II", image, 0x98 + 96, 0x2040, 0xC0)
    struct.pack_into("<IIHHIIIIIII", image, 1024 + 0x40,
                     0, 0, 0, 0, 0x20B0, 1, 1, int(named),
                     0x2080, 0x2084 if named else 0, 0x2088 if named else 0)
    struct.pack_into("<I", image, 1024 + 0x80, target)
    if named:
        struct.pack_into("<IH", image, 1024 + 0x84, 0x20C0, 0)
        image[1024 + 0xC0:1024 + 0xC0 + 6] = b"entry\0"
    image[1024 + 0xB0:1024 + 0xB0 + 8] = b"app.exe\0"
    image[1024 + 0xD0:1024 + 0xD0 + 12] = b"other.entry\0"
    update_artifact(linked, "app.exe", image)


@pytest.mark.parametrize("version", ["baseline", "current"])
@pytest.mark.parametrize("named", [False, True])
@pytest.mark.parametrize("target", [0x1000, 0x1001, 0x100F, 0x1010, 0x11E0, 0x20D0])
def test_complete_exports_protect_only_targeted_root_span(monkeypatch, tmp_path, version, named, target):
    def exported(linked):
        export_target(linked, target, named=named)

    pair = make_pair(monkeypatch, tmp_path,
                     **{("old_damage" if version == "baseline" else "new_damage"): exported})
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    overlaps = 0x1000 <= target < 0x1010
    assert order["result"] == ("fail" if overlaps else "pass"), order
    assert bool(order.get("root_renamings")) is not overlaps
    if overlaps:
        assert any("exported entry" in row["reason"] for row in order["root_renaming_refusals"])


@pytest.mark.parametrize("version", ["baseline", "current"])
@pytest.mark.parametrize("target", [0x2000, 0x2003, 0x2004, 0x2005, 0x2007, 0x2008, 0x200B, 0x200C, 0x200F])
def test_exported_table_address_blocks_locality_even_in_another_owners_slot(monkeypatch, tmp_path, version, target):
    def exported(linked):
        export_target(linked, target, named=False)

    pair = make_pair(monkeypatch, tmp_path, codes=(b"\xc3", b"\xc3"),
                     **{("old_damage" if version == "baseline" else "new_damage"): exported})
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    assert order["result"] == "fail", order
    assert not order.get("root_renamings")
    assert any("table has an exported address" in row["reason"] for row in order["root_renaming_refusals"])


@pytest.mark.parametrize("target", [0x2004, 0x2007, 0x2008])
def test_public_table_alias_prevents_normalization(monkeypatch, tmp_path, target):
    def alias(linked):
        mapping = (linked.directory / "app.map").read_text()
        line = f" 0002:{target - 0x2000:08x} _table_alias {0x400000 + target:08x} dependency.obj\n"
        update_artifact(linked, "app.map", mapping.replace(" entry point", line + " entry point").encode())

    pair = make_pair(monkeypatch, tmp_path, codes=(b"\xc3", b"\xc3"), new_damage=alias)
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    assert order["result"] == "fail", order
    assert not order.get("root_renamings")
    assert any("table has a public linked alias" in row["reason"] for row in order["root_renaming_refusals"])


def test_repeated_registration_of_same_root_does_not_lose_address_identity(monkeypatch, tmp_path):
    pair = make_pair(monkeypatch, tmp_path, old_damage=repeated_root, new_damage=repeated_root)
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


def escaped_reference(linked, *, self_reference):
    code = b"\x68" + bytes(4) + b"\xc3"
    rows = ([function("_$E1", 0, code, edges=((1, "_$E1", 6),))] if self_reference else
            [function("_$E1", 0), function("_holder", 0x40, code, edges=((1, "_$E1", 6),))])
    replace_object(linked, object_bytes(rows, ("_$E1",), {}))
    image = bytearray((linked.directory / "app.exe").read_bytes())
    at = 0 if self_reference else 0x40
    image[512 + at:512 + at + 16] = code + b"\xcc" * (16 - len(code))
    struct.pack_into("<I", image, 512 + at + 1, 0x401000)
    relocation = struct.pack("<IIHHIIHH", 0x1000, 12, 0x3001 + at, 0, 0x2000, 12, 0x3004, 0)
    image[512 + 0x180:512 + 0x180 + len(relocation)] = relocation
    struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, len(relocation))
    update_artifact(linked, "app.exe", image)
    mapping = (linked.directory / "app.map").read_text()
    # The new object's table intentionally has no named slot anchor.
    mapping = "".join(line for line in mapping.splitlines(keepends=True) if " _$S1 " not in line)
    if not self_reference:
        mapping += " 0001:00000040 _holder 00401040 f unit0.obj\n"
    update_artifact(linked, "app.map", mapping.encode())


@pytest.mark.parametrize("self_reference", [False, True])
def test_self_address_or_other_section_reference_blocks_local_root_identity(monkeypatch, tmp_path, self_reference):
    def escape(linked):
        escaped_reference(linked, self_reference=self_reference)

    pair = make_pair(monkeypatch, tmp_path, old_damage=escape, new_damage=escape)
    assert_observed(pair)
    result = compare(pair)["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


def test_equal_code_with_different_exact_reference_identity_still_fails(monkeypatch, tmp_path):
    code = b"\xe8" + bytes(4) + b"\xc3"

    def renamed_dependency(linked):
        raw = linked.objects[1].read_bytes()
        replace_object(linked, raw.replace(b"_dep\0\0\0\0", b"_other\0\0"), index=1)
        rename_root(linked)

    old = physical_context(monkeypatch, tmp_path / "old", codes=(code,), references=("_dep",))
    new = physical_context(monkeypatch, tmp_path / "new", codes=(code,), references=("_other",), damage=renamed_dependency)
    assert_observed((old, new))
    result = compare((old, new))["projected_order"]
    assert result["result"] == "fail", result
    assert not result.get("root_renamings")


@pytest.mark.parametrize("damage", ["receipt", "source", "header", "config", "pin", "expired"])
def test_native_epoch_mutation_or_expiry_never_yields_renaming_permission(monkeypatch, tmp_path, damage):
    pair = make_pair(monkeypatch, tmp_path)
    current = pair[1]
    if damage == "receipt":
        current.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(b"stale")
    elif damage == "expired":
        current.alive = False
    else:
        path = {"source": "src/unit0.cpp", "header": "src/new.h", "config": "recovery/project.json",
                "pin": "recovery/image.pin"}[damage]
        (current.root / path).write_bytes(b"changed outside immutable epoch")
    with pytest.raises(ValueError, match="changed|expired"):
        compare(pair)


def test_final_epoch_guard_still_checks_after_native_renaming(monkeypatch, tmp_path):
    pair = make_pair(monkeypatch, tmp_path)
    # The first guard passes. Invalidation during observation cannot turn the
    # physically proved local root into a reusable persisted capability.
    pair[1].on_check = lambda count: setattr(pair[1], "alive", count < 4)
    with pytest.raises(ValueError, match="expired"):
        compare(pair)


def test_matched_exact_names_do_not_require_new_local_rename_authority(monkeypatch, tmp_path):
    pair = tuple(physical_context(monkeypatch, tmp_path / name, configured=False) for name in ("old", "new"))
    result = compare(pair)["projected_order"]
    assert result["result"] == "pass", result
    assert not result.get("root_renamings")


def associated_fpo(linked, *, damage=None):
    """Append a physical associated FPO_DATA contribution before receipt seal.

    Its presence in COFF is preserved evidence. An absent PE debug directory
    does not establish whether the linker discarded or otherwise treated it.
    """
    raw = linked.objects[0].read_bytes()
    obj = Coff(raw)
    root = obj.function("_$E1")
    root_size = len(obj.sections[root.section - 1].data)
    metadata = bytearray(struct.pack("<IIII", 0, root_size, 0, 0))
    if damage == "frame":
        struct.pack_into("<I", metadata, 12, 1)
    elif damage == "addend":
        struct.pack_into("<I", metadata, 0, 4)
    elif damage == "size":
        struct.pack_into("<I", metadata, 4, root_size + 1)
    elif damage == "truncated":
        metadata = metadata[:12]
    relocations = [(0, root.index, 7)]
    if damage == "relocation-kind":
        relocations[0] = (0, root.index, 6)
    elif damage == "relocation-target":
        relocations[0] = (0, 1, 7)  # Named startup-table cell, not this root.
    elif damage == "additional-reference":
        relocations.append((4, root.index, 7))
    specs = [(section.name, section.data, section.relocations, section.flags) for section in obj.sections]
    child = len(specs) + 1
    specs.append((".debug$F", bytes(metadata), tuple(relocations), 0x42101048))
    original_table, symbol_count = struct.unpack_from("<II", raw, 8)
    child_symbol = symbol_count + 2  # Root section definition and its auxiliary precede it.
    if damage in ("incoming-root", "incoming-metadata"):
        target = root.index if damage == "incoming-root" else child_symbol
        specs.append((".rdata", bytes(4), ((0, target, 6),), 0x40300040))
    parent = 2 if damage == "association" else root.section
    auxiliary = struct.pack("<IHHIHB3x", len(metadata), len(relocations), 0, 0, parent, 5)
    root_section = obj.sections[root.section - 1]
    root_auxiliary = struct.pack("<IHHIHB3x", root_size, len(root_section.relocations), 0, 0, 0, 2)
    symbols = (raw[original_table:original_table + 18 * symbol_count]
               + root_section.name.encode().ljust(8, b"\0")
               + struct.pack("<IhHBB", 0, root.section, 0, 3, 1) + root_auxiliary
               + b".debug$F" + struct.pack("<IhHBB", 0, child, 0, 3, 1) + auxiliary)
    strings = raw[original_table + 18 * symbol_count:]
    start = 20 + 40 * len(specs)
    headers, payload = [], bytearray()
    for name, data, references, flags in specs:
        at = start + len(payload)
        payload.extend(data)
        relocation_at = start + len(payload) if references else 0
        for reference in references:
            payload.extend(struct.pack("<IIH", *reference))
        headers.append(name.encode().ljust(8, b"\0") + struct.pack(
            "<IIIIIIHHI", 0, 0, len(data), at, relocation_at, 0, len(references), 0, flags))
    output = (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), symbol_count + 4, 0, 0)
              + b"".join(headers) + payload + symbols + strings)
    replace_object(linked, output)


@pytest.mark.parametrize("size", [16, 32])
def test_exact_retained_fpo_metadata_supports_only_bounded_root_order(monkeypatch, tmp_path, size):
    code = b"\x90" * (size - 1) + b"\xc3"
    pair = make_pair(monkeypatch, tmp_path, codes=(code,), old_damage=associated_fpo, new_damage=associated_fpo)
    assert_observed(pair)
    result = compare(pair)
    order = result["projected_order"]
    assert order["result"] == "pass", order
    assert order["root_metadata_comparisons"] == [{
        "index": 0, "result": "pass", "linked_treatment": "unknown",
        "metadata": [{"format": "fpo_data", "code_size": size,
                      "data": struct.pack("<IIII", 0, size, 0, 0).hex(),
                      "reference": {"offset": 0, "kind": 7, "target": "root"}}]}]
    before, = order["baseline"]["sequence"]
    assert order["root_renamings"] == [{"index": 0, "unit": "src/unit0.cpp", "baseline_symbol": "_$E1",
                                       "current_symbol": "_$E9", "body_fingerprint": before["body_fingerprint"]}]
    assert order["remaining_linked_effects"] == result["remaining_linked_effects"] == "unknown"
    assert not order["ready_for_collection"] and not result["ready_for_collection"]


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_fpo_child_presence_change_is_not_erased_by_root_renaming(monkeypatch, tmp_path, version):
    pair = make_pair(monkeypatch, tmp_path, **{("old_damage" if version == "baseline" else "new_damage"): associated_fpo})
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    assert order["result"] == "fail", order
    assert not order.get("root_renamings")
    assert not order.get("root_metadata_comparisons")


@pytest.mark.parametrize("damage", ["frame", "addend", "size", "truncated", "relocation-kind",
                                     "relocation-target", "additional-reference", "incoming-root", "incoming-metadata",
                                     "association"])
def test_unsupported_or_escaping_fpo_cannot_authorize_a_rename(monkeypatch, tmp_path, damage):
    def changed(linked):
        associated_fpo(linked, damage=damage)

    pair = make_pair(monkeypatch, tmp_path, old_damage=associated_fpo, new_damage=changed)
    if damage != "association":
        assert_observed(pair)  # The refusal belongs to metadata/locality, not a broken code observation.
    order = compare(pair)["projected_order"]
    assert order["result"] != "pass", order
    assert not order.get("root_renamings")
    assert not order.get("root_metadata_comparisons")


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_source_owned_fpo_alias_is_not_anonymous_metadata(monkeypatch, tmp_path, version):
    sources = {"src/metadata.h": b"// RECOVERY_OBJECT: v1 0x00403000 .debug$F\nint metadata;\n"}
    pair = make_pair(monkeypatch, tmp_path, old_damage=associated_fpo, new_damage=associated_fpo,
                     **{("old_sources" if version == "baseline" else "new_sources"): sources})
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    assert order["result"] == "fail", order
    assert not order.get("root_renamings")
    assert not order.get("root_metadata_comparisons")


@pytest.mark.parametrize("version", ["baseline", "current"])
@pytest.mark.parametrize("alias", ["root", "table", "fpo"])
@pytest.mark.parametrize("kind", ["shared", "generated", "import"])
def test_every_source_symbol_binding_kind_blocks_anonymous_root_normalization(monkeypatch, tmp_path, version, alias, kind):
    from match_recovery.generated_source import parse_generated
    from match_recovery.import_source import parse_imports
    from match_recovery.shared_source import parse_shared

    symbol = {"root": "_$E1" if version == "baseline" else "_$E9", "table": "_$S1", "fpo": ".debug$F"}[alias]
    start = {"root": 0x401000, "table": 0x402004, "fpo": 0x403000}[alias]
    annotation = {
        "shared": f"// RECOVERY_SHARED: v1 0x{start:08X} 0x{start + 4:08X} {symbol} 0x00401100\nvoid mapped() {{}}\n",
        "generated": f"// RECOVERY_GENERATED: v1 vtable 0x{start:08X} ? {symbol} 0x00401100\nclass Mapped {{}};\n",
        "import": f"// RECOVERY_IMPORT: v1 {symbol} example.dll name:entry\nextern int mapped;\n",
    }[kind]
    path = "src/other_binding.h"
    # Prove valid annotations are responsible: malformed parser input is not
    # an adequate negative control for a silently omitted binding kind.
    if kind == "import":
        bindings = parse_imports(annotation, path)
    else:
        parser = parse_shared if kind == "shared" else parse_generated
        bindings = parser(annotation, path, "a" * 64, address_space="synthetic:va")
    binding, = bindings
    assert binding.symbol == symbol
    if kind == "shared":
        assert binding.body_error is None
    inputs = {("old_sources" if version == "baseline" else "new_sources"): {path: annotation.encode()}}
    if alias == "fpo":
        inputs.update(old_damage=associated_fpo, new_damage=associated_fpo)
    pair = make_pair(monkeypatch, tmp_path, **inputs)
    assert_observed(pair)
    order = compare(pair)["projected_order"]
    assert order["result"] == "fail", order
    assert not order.get("root_renamings")
    assert not order.get("root_metadata_comparisons")
    assert any("source binding" in row["reason"] for row in order["root_renaming_refusals"])
