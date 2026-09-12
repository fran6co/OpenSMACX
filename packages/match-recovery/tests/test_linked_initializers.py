"""Physical synthetic COFF/PE/link evidence, not a compiler or runtime pilot."""

import json
from pathlib import Path
import struct

import pytest

from test_binary import coff_fixture
from test_link_file_evidence import stamp
from test_link_map import executable


def initializer_object(code, *, reference=None, table_addend=0, table_kind=6, name="_$E1"):
    """One local code COMDAT and one real initializer pointer contribution."""
    code = code + b"\xcc" * (-len(code) % 16)
    code_relocations = ((1, 2, 20),) if reference else ()
    specs = [(b".text", code, code_relocations, 0x60501020),
             (b".CRT$XCU", struct.pack("<I", table_addend), ((0, 0, table_kind),), 0xC0300040)]
    headers, payload = [], bytearray()
    for label, data, relocations, flags in specs:
        at = 100 + len(payload)
        payload.extend(data)
        relocation_at = 100 + len(payload) if relocations else 0
        for relocation in relocations:
            payload.extend(struct.pack("<IIH", *relocation))
        headers.append(label.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(data), at,
                                                        relocation_at, 0, len(relocations), 0, flags))
    symbols = [(name.encode(), 0, 1, 0x20, 3), (b"_$S1", 0, 2, 0, 3)]
    if reference:
        symbols.append((reference.encode(), 0, 0, 0x20, 2))
    records = b"".join(label.ljust(8, b"\0") + struct.pack("<IhHBB", value, section, kind, storage, 0)
                       for label, value, section, kind, storage in symbols)
    return (struct.pack("<HHIIIHH", 0x14C, 2, 0, 100 + len(payload), len(symbols), 0, 0)
            + b"".join(headers) + payload + records + struct.pack("<I", 4))


class Context:
    """Test-owned live artifact epoch; object freshness remains native."""

    def __init__(self, root, directory, report):
        self.root, self.directory, self.report = root, directory, report
        self.alive, self.checks, self.on_check = True, 0, None
        self.pin_outputs()

    def pin_outputs(self):
        self.expected = {self.directory / row["path"]: stamp((self.directory / row["path"]).read_bytes())
                         for row in self.report["outputs"]}

    def check_current(self):
        self.checks += 1
        if self.on_check:
            self.on_check(self.checks)
        if not self.alive:
            raise ValueError("synthetic context expired")
        if any(not path.is_file() or stamp(path.read_bytes()) != wanted for path, wanted in self.expected.items()):
            raise ValueError("synthetic context artifact changed")


def build(root, codes=(b"\xc3",), *, owners=None, references=None, displacement=0,
          table_addend=0, table_kind=6):
    root.mkdir()
    directory = root / "build/product"
    directory.mkdir(parents=True)
    owners = owners or tuple(f"unit{index}.obj" for index in range(len(codes)))
    references = references or (None,) * len(codes)
    objects = []
    raw = bytearray(executable())
    raw.extend(bytes(512))
    struct.pack_into("<H", raw, 0x84 + 2, 2)
    struct.pack_into("<I", raw, 0x98 + 16, 0x11E0)
    raw[0x1A0:0x1C8] = struct.pack("<8sIIIIIIHHI", b".data\0\0\0", 512, 0x2000, 512, 1024,
                                         0, 0, 0, 0, 0xC0000040)
    raw[512:1024] = b"\xcc" * 512
    raw[512 + 0x1E0] = 0xC3
    addresses, statics = [], []
    for index, (code, owner, reference) in enumerate(zip(codes, owners, references, strict=True)):
        path = directory / owner
        path.write_bytes(initializer_object(code, reference=reference, table_addend=table_addend,
                                            table_kind=table_kind))
        objects.append(path)
        offset = displacement + index * 0x20
        address = 0x401000 + offset
        addresses.append(address)
        linked = bytearray(code + b"\xcc" * (-len(code) % 16))
        if reference:
            struct.pack_into("<i", linked, 1, 0x4011E0 - (address + 5))
        raw[512 + offset:512 + offset + len(linked)] = linked
        struct.pack_into("<I", raw, 1024 + 4 + index * 4, address)
        statics += [f" 0001:{offset:08x} _$E1 {address:08x} f {owner}\n",
                    f" 0002:{4 + index * 4:08x} _$S1 {0x402004 + index * 4:08x} {owner}\n"]
    # Public dependency placement is independent of operands under comparison.
    dependency = directory / "dependency.obj"
    dependency.write_bytes(coff_fixture().replace(b"_f\0\0\0\0\0\0", b"_dep\0\0\0\0"))
    objects.append(dependency)
    table_size = 4 * len(codes)
    entries = [0x3004 + 4 * index for index in range(len(codes))]
    if len(entries) % 2:
        entries.append(0)
    relocations = struct.pack("<II", 0x2000, 8 + 2 * len(entries)) + struct.pack("<" + "H" * len(entries), *entries)
    raw[512 + 0x180:512 + 0x180 + len(relocations)] = relocations
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
    mapping = ("synthetic\n Preferred load address is 00400000\n"
               " Start         Length     Name                   Class\n"
               " 0001:00000000 00000200H .text CODE\n"
               " 0002:00000000 00000004H .CRT$XCA DATA\n"
               f" 0002:00000004 {table_size:08x}H .CRT$XCU DATA\n"
               f" 0002:{4 + table_size:08x} 00000004H .CRT$XCZ DATA\n"
               " Address         Publics by Value              Rva+Base     Lib:Object\n"
               " 0001:000001e0 _dep 004011e0 f dependency.obj\n"
               " 0001:000001e0 _other 004011e0 f dependency.obj\n"
               " entry point at 0001:000001e0\n Static symbols\n" + "".join(statics))
    log = "Start Pass1\nEnd Pass1\nStart Pass2\n" + "".join(path.name + "\n" for path in objects) + "End Pass2\n"
    tool = root / "linker"
    tool.write_bytes(b"synthetic identity only; never invoked")
    receipt = {"schema_version": 2, "link_log_sha256": stamp(log.encode())["sha256"],
               "cwd": str(directory), "tool": str(tool), "arguments": [str(path) for path in objects],
               "explicit_inputs": [str(path) for path in objects],
               "files": [{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())}
                         for path in [tool, *objects]], "library_searches": [], "generated_exports": [], "capture_gaps": []}
    products = [("app.exe", "pe32-i386", bytes(raw)), ("app.map", "msvc-map", mapping.encode()),
                ("link.log", "msvc-link-log", log.encode()), ("link.json", "link-input-files", json.dumps(receipt).encode())]
    rows = []
    for name, kind, data in products:
        (directory / name).write_bytes(data)
        rows.append({"path": name, "format": kind, "result": "pass", **stamp(data)})
    result = Context(root, directory, {"result": "pass", "compile_link": {"result": "pass"}, "outputs": rows})
    result.objects, result.addresses = objects, addresses
    return result


def compare(old, new):
    from match_recovery.linked_initializers import compare_build_initializers
    return compare_build_initializers(old, new)


def update_artifact(context, name, raw):
    """Establish different coherent synthetic build evidence, not freshness damage."""
    (context.directory / name).write_bytes(raw)
    next(row for row in context.report["outputs"] if row["path"] == name).update(stamp(raw))
    context.pin_outputs()


@pytest.mark.parametrize("moved", [False, True])
def test_exact_ordered_initializer_bodies_survive_tu_and_address_moves(tmp_path, moved):
    old = build(tmp_path / "old", (b"\xc3", b"\x90\xc3"))
    new = build(tmp_path / "new", (b"\xc3", b"\x90\xc3"),
                owners=("moved0.obj", "moved1.obj") if moved else None, displacement=0x40 if moved else 0)
    report = compare(old, new)
    assert report["check"] == "application_linked_initializers"
    assert report["result"] == "pass"
    assert report["remaining_linked_effects"] == "unknown"
    assert old.checks >= 2 and new.checks >= 2
    assert not report.get("ready_for_collection", False)


@pytest.mark.parametrize("codes", [(b"\xc3", b"\xc3"), (b"\x90\xc3", b"\xc3"), (b"\xc3",)])
def test_duplicate_reorder_and_removal_are_measured_sequence_failures(tmp_path, codes):
    old = build(tmp_path / "old", (b"\xc3", b"\x90\xc3"))
    new = build(tmp_path / "new", codes)
    assert compare(old, new)["result"] == "fail"


def test_added_identical_initializer_is_not_silently_deduplicated(tmp_path):
    assert compare(build(tmp_path / "old"), build(tmp_path / "new", (b"\xc3", b"\xc3")))["result"] == "fail"


def test_external_reference_identity_is_not_hidden_by_equal_addresses(tmp_path):
    code = b"\xe8\0\0\0\0\xc3"
    old = build(tmp_path / "old", (code,), references=("_dep",))
    same = build(tmp_path / "same", (code,), references=("_dep",), displacement=0x40)
    changed = build(tmp_path / "changed", (code,), references=("_other",))
    assert compare(old, same)["result"] == "pass"
    assert compare(old, changed)["result"] == "fail"


@pytest.mark.parametrize("damage", ["code", "pointer", "missing_highlow"])
def test_actual_linked_bytes_and_loader_fixups_must_agree_with_object_evidence(tmp_path, damage):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    raw = bytearray((new.directory / "app.exe").read_bytes())
    if damage == "code":
        raw[512] = 0x90
    elif damage == "pointer":
        struct.pack_into("<I", raw, 1028, new.addresses[0] + 1)
    else:
        struct.pack_into("<H", raw, 512 + 0x180 + 8, 0)
    update_artifact(new, "app.exe", bytes(raw))
    assert compare(old, new)["result"] == "fail"


@pytest.mark.parametrize("damage", ["statics", "contributions", "xcu", "map_output", "image_output", "log_output", "receipt_output"])
def test_missing_linked_placement_or_table_evidence_cannot_pass(tmp_path, damage):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    if damage in {"statics", "contributions", "xcu"}:
        text = (new.directory / "app.map").read_text()
        if damage == "statics":
            text = text.split(" Static symbols")[0]
        elif damage == "contributions":
            start, end = text.index(" Start"), text.index(" Address")
            text = text[:start] + text[end:]
        else:
            text = "\n".join(line for line in text.split("\n") if ".CRT$XCU" not in line)
        update_artifact(new, "app.map", text.encode())
    else:
        name = {"map_output": "app.map", "image_output": "app.exe", "log_output": "link.log", "receipt_output": "link.json"}[damage]
        new.report["outputs"] = [row for row in new.report["outputs"] if row["path"] != name]
    assert compare(old, new)["result"] == "unknown"


@pytest.mark.parametrize("keyword,value", [("table_addend", 4), ("table_kind", 20)])
def test_unsupported_initializer_pointer_encoding_stays_unknown(tmp_path, keyword, value):
    assert compare(build(tmp_path / "old"), build(tmp_path / "new", **{keyword: value}))["result"] == "unknown"


@pytest.mark.parametrize("damage", ["expired", "artifact", "object", "tool", "alias"])
def test_stale_or_unobserved_live_inputs_never_reuse_a_previous_proof(tmp_path, damage):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    assert compare(old, new)["result"] == "pass"
    if damage == "expired":
        new.alive = False
    elif damage == "artifact":
        path = new.directory / "app.exe"
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "alias":
        path = new.objects[0]
        target = path.with_suffix(".original")
        path.rename(target)
        path.symlink_to(target)
    else:
        path = new.objects[0] if damage == "object" else new.root / "linker"
        path.write_bytes(path.read_bytes() + b"changed")
    try:
        report = compare(old, new)
    except ValueError:
        assert damage in {"expired", "artifact"}
    else:
        assert report["result"] != "pass"


def test_context_expiry_after_observation_invalidates_the_comparison(tmp_path):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    def expire(check):
        if check >= 2:
            new.alive = False
    new.on_check = expire
    try:
        report = compare(old, new)
    except ValueError as error:
        assert "expired" in str(error)
    else:
        assert report["result"] != "pass"


@pytest.mark.parametrize("damage", ["wrong_owner", "folded", "unplaced", "not_function"])
def test_local_initializers_need_unambiguous_independent_owner_placements(tmp_path, damage):
    old = build(tmp_path / "old", (b"\xc3", b"\x90\xc3"))
    new = build(tmp_path / "new", (b"\xc3", b"\x90\xc3"))
    text = (new.directory / "app.map").read_text()
    if damage == "wrong_owner":
        text = text.replace("_$E1 00401000 f unit0.obj", "_$E1 00401000 f dependency.obj")
    elif damage == "folded":
        text = text.replace("0001:00000020 _$E1 00401020", "0001:00000000 _$E1 00401000")
    elif damage == "unplaced":
        text = text.replace("0001:00000000 _$E1 00401000", "0000:00000000 _$E1 00000000")
    else:
        text = text.replace("_$E1 00401000 f unit0.obj", "_$E1 00401000 unit0.obj")
    update_artifact(new, "app.map", text.encode())
    assert compare(old, new)["result"] == "unknown"


def test_external_target_cannot_be_inferred_from_the_linked_call_operand(tmp_path):
    code = b"\xe8\0\0\0\0\xc3"
    old = build(tmp_path / "old", (code,), references=("_dep",))
    new = build(tmp_path / "new", (code,), references=("_dep",))
    text = (new.directory / "app.map").read_text()
    text = "\n".join(line for line in text.split("\n") if " _dep " not in line)
    update_artifact(new, "app.map", text.encode())
    assert compare(old, new)["result"] == "unknown"


def test_linked_call_operand_must_match_its_independent_public_target(tmp_path):
    code = b"\xe8\0\0\0\0\xc3"
    old = build(tmp_path / "old", (code,), references=("_dep",))
    new = build(tmp_path / "new", (code,), references=("_dep",))
    raw = bytearray((new.directory / "app.exe").read_bytes())
    raw[513] ^= 1
    update_artifact(new, "app.exe", bytes(raw))
    assert compare(old, new)["result"] == "fail"


def test_empty_initializer_sequence_requires_complete_inputs_and_explicit_contribution_inventory(tmp_path):
    old, new = build(tmp_path / "old", ()), build(tmp_path / "new", ())
    for context in (old, new):
        text = (context.directory / "app.map").read_text()
        text = "\n".join(line for line in text.split("\n") if ".CRT$XCU" not in line)
        update_artifact(context, "app.map", text.encode())
    report = compare(old, new)
    assert report["result"] == "pass"
    assert report["baseline"]["sequence"] == report["current"]["sequence"] == []
    assert report["remaining_linked_effects"] == "unknown"
    text = (new.directory / "app.map").read_text()
    begin, end = text.index(" Start"), text.index(" Address")
    update_artifact(new, "app.map", (text[:begin] + text[end:]).encode())
    assert compare(old, new)["result"] == "unknown"


@pytest.mark.parametrize("table_candidate", [None, 0, 1, "malformed"])
def test_repeated_ordinary_archive_members_need_absence_in_every_candidate(tmp_path, table_candidate):
    from match_recovery.link_input_coverage import input_coverage
    from test_toolchain_components import archive

    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    candidates = [coff_fixture(b"\xc3"), coff_fixture(b"\x90\xc3")]
    if type(table_candidate) is int:
        candidates[table_candidate] = initializer_object(b"\xc3")
    elif table_candidate == "malformed":
        candidates[1] = b"not a parseable COFF record"
    library = new.directory / "runtime.lib"
    library.write_bytes(archive([("repeat.obj/", raw) for raw in candidates]))
    log = (new.directory / "link.log").read_text()
    log = log.replace("Start Pass1\n", f"Start Pass1\n Searching {library}:\n")
    log = log.replace("End Pass2\n", "RUNTIME.lib(repeat.obj)\nEnd Pass2\n")
    receipt = json.loads((new.directory / "link.json").read_text())
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    receipt["library_searches"].append({"spelling": str(library), "path": str(library)})
    receipt["files"].append({"path": str(library), "before": stamp(library.read_bytes()),
                             "after": stamp(library.read_bytes())})
    update_artifact(new, "link.log", log.encode())
    update_artifact(new, "link.json", json.dumps(receipt).encode())
    coverage = input_coverage(json.dumps(receipt).encode(), log.encode())
    selected = next(row for row in coverage["inputs"] if row["library"] is not None)
    assert selected["candidate_count"] == 2 and selected["selection"] == "unknown"
    if table_candidate != "malformed":
        assert coverage["result"] == "pass"
        assert coverage["format_counts"] == {"coff": 4}
    else:
        assert coverage["result"] == "unknown" and coverage["unsupported_records"]
    report = compare(old, new)
    assert report["result"] == ("pass" if table_candidate is None else "unknown")
    assert report["remaining_linked_effects"] == "unknown"
    if table_candidate is None:
        assert len(report["current"]["entries"]) == 1
        assert report["current"]["entries"][0]["input"]["path"] == str(new.objects[0])


@pytest.mark.parametrize("kind,target", [("msvc-map", "app.exe"), ("msvc-link-log", "link.json")])
def test_distinct_artifact_formats_cannot_share_incompatible_physical_contents(tmp_path, kind, target):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    row = next(row for row in new.report["outputs"] if row["format"] == kind)
    row.update(path=target, **stamp((new.directory / target).read_bytes()))
    new.pin_outputs()
    assert compare(old, new)["result"] == "unknown"


@pytest.mark.parametrize("surviving", [False, True])
def test_explicit_initializer_object_cannot_disappear_from_final_log_and_map(tmp_path, surviving):
    old = build(tmp_path / "old", (b"\xc3",) if surviving else ())
    new = build(tmp_path / "new", (b"\xc3", b"\xc3") if surviving else (b"\xc3",))
    if not surviving:
        text = (old.directory / "app.map").read_text()
        update_artifact(old, "app.map", "\n".join(line for line in text.split("\n")
                                                  if ".CRT$XCU" not in line).encode())
    omitted = new.objects[1 if surviving else 0]
    log = (new.directory / "link.log").read_text().replace(omitted.name + "\n", "")
    receipt = json.loads((new.directory / "link.json").read_text())
    assert str(omitted) in receipt["explicit_inputs"]
    assert next(row for row in receipt["files"] if row["path"] == str(omitted))["after"] == stamp(omitted.read_bytes())
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(new, "link.log", log.encode())
    update_artifact(new, "link.json", json.dumps(receipt).encode())
    text = (new.directory / "app.map").read_text()
    if surviving:
        text = text.replace("00000008H .CRT$XCU", "00000004H .CRT$XCU")
    else:
        text = "\n".join(line for line in text.split("\n") if ".CRT$XCU" not in line)
    update_artifact(new, "app.map", text.encode())
    # The PE still has the missing object's pointer. Neither an omitted table
    # range nor a narrower surviving range can establish absence of that input.
    report = compare(old, new)
    assert report["result"] == "unknown"
    assert report["current"]["result"] == "unknown"


def long_table_name_object():
    raw = bytearray(initializer_object(b"\xc3"))
    raw[60:68] = b"/4".ljust(8, b"\0")
    text = b".CRT$XCU\0"
    raw[-4:] = struct.pack("<I", 4 + len(text)) + text
    return bytes(raw)


@pytest.mark.parametrize("archive_group", [False, True])
def test_long_section_name_encoding_cannot_supply_false_table_absence(tmp_path, archive_group):
    from test_toolchain_components import archive

    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    receipt = json.loads((new.directory / "link.json").read_text())
    if archive_group:
        path = new.directory / "runtime.lib"
        path.write_bytes(archive([("repeat.obj/", coff_fixture()), ("repeat.obj/", long_table_name_object())]))
        log = (new.directory / "link.log").read_text()
        log = log.replace("Start Pass1\n", f"Start Pass1\n Searching {path}:\n")
        log = log.replace("End Pass2\n", "RUNTIME.lib(repeat.obj)\nEnd Pass2\n")
        receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
        receipt["library_searches"].append({"spelling": str(path), "path": str(path)})
        receipt["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
        update_artifact(new, "link.log", log.encode())
    else:
        path = new.objects[0]
        path.write_bytes(long_table_name_object())
        row = next(row for row in receipt["files"] if row["path"] == str(path))
        row["before"] = row["after"] = stamp(path.read_bytes())
    update_artifact(new, "link.json", json.dumps(receipt).encode())
    report = compare(old, new)
    assert report["result"] == "unknown"
    assert report["current"]["result"] == "unknown"
