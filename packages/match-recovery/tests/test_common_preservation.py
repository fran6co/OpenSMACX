"""Physical cross-build storage controls; not application acceptance."""

import json
import struct

import pytest

from match_recovery.common_preservation import compare_common_builds
from test_coff_inventory import common_object, imported
from test_common_allocation import fixture, replace_library
from test_link_file_evidence import stamp
from test_linked_initializers import Context
from test_toolchain_components import archive


def build(root, *, size=4, offset=0, extra=False, damage=None):
    root.mkdir()
    evidence, log, image, mapping = fixture(root)
    library = root / "runtime.lib"
    library.write_bytes(archive([("runtime.obj/", common_object(size)), ("DEMO.dll/", imported()),
                                ("DEMO.dll/", imported("_Other@8"))]))
    evidence["files"][0]["before"] = evidence["files"][0]["after"] = stamp(library.read_bytes())
    if extra:
        path = root / "added.obj"
        path.write_bytes(common_object(size))
        evidence["explicit_inputs"].append(str(path))
        evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()),
                                  "after": stamp(path.read_bytes())})
    mapping = mapping.replace("0002:00000000 _g 00402000", f"0002:{offset:08x} _g {0x402000 + offset:08x}")
    if damage == "conflicting_size":
        replace_library(root, evidence, [("other.obj/", common_object(size + 4))])
    elif damage == "missing":
        mapping = mapping.replace(" _g ", " _else ")
    elif damage == "overlap":
        mapping = mapping.replace(" entry point", f" 0002:{offset:08x} _alias {0x402000 + offset:08x} <common>\n entry point")
    elif damage == "policy":
        evidence["arguments"].append("/FORCE")
    elif damage in {"nonzero", "highlow"}:
        image = bytearray(image)
        if damage == "nonzero":
            at = len(image)
            image.extend(b"\x01" + bytes(63))
            struct.pack_into("<II", image, 0x1A0 + 16, 64, at)
        else:
            struct.pack_into("<I", image, 0x98 + 92, 16)
            struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1040, 12)
            struct.pack_into("<IIHH", image, 512 + 0x40, 0x2000, 12, 0x3000, 0)
        image = bytes(image)
    directory = root / "product"
    directory.mkdir()
    products = [("app.exe", "pe32-i386", image), ("app.map", "msvc-map", mapping.encode()),
                ("link.log", "msvc-link-log", log.encode()),
                ("link.json", "link-input-files", json.dumps(evidence).encode())]
    rows = []
    for name, kind, raw in products:
        (directory / name).write_bytes(raw)
        rows.append({"path": name, "format": kind, "result": "pass", **stamp(raw)})
    context = Context(root, directory, {"result": "pass", "compile_link": {"result": "pass"}, "outputs": rows})
    # Model the native ApplicationBuild guard's physical input epoch too.
    context.expected.update({root / name: stamp((root / name).read_bytes())
                             for name in ("runtime.lib", "app.obj", "app.exp", "linker")})
    if extra:
        context.expected[root / "added.obj"] = stamp((root / "added.obj").read_bytes())
    return context


@pytest.mark.parametrize("offset", [0, 16])
@pytest.mark.parametrize("extra", [False, True])
def test_same_storage_with_relocation_and_additional_uniform_requests(tmp_path, offset, extra):
    old, new = build(tmp_path / "old"), build(tmp_path / "new", offset=offset, extra=extra)
    report = compare_common_builds(old, new, ("_g",))
    assert report["result"] == "pass", report
    row, = report["allocations"]
    assert (row["baseline_address"], row["current_address"]) == (0x402000, 0x402000 + offset)
    assert row["baseline_size"] == row["current_size"] == 4
    assert len(report["current"]["allocations"][0]["requests"]) == 1 + extra
    assert report["remaining_linked_effects"] == report["original_verification"] == report["runtime_verification"] == "unknown"
    assert report["allocation_isolation"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]
    assert old.checks == new.checks and old.checks >= 2


@pytest.mark.parametrize("size", [1, 8])
def test_changed_extent_is_a_measured_failure_even_when_both_zero(tmp_path, size):
    report = compare_common_builds(build(tmp_path / "old"), build(tmp_path / "new", size=size), ("_g",))
    assert report["baseline"]["result"] == report["current"]["result"] == "pass"
    assert report["result"] == "fail"
    assert report["allocations"][0]["reason"] == "selected common extent changed"


@pytest.mark.parametrize("version", ["old", "new"])
@pytest.mark.parametrize("damage", ["conflicting_size", "missing", "overlap", "policy", "highlow", "nonzero"])
def test_either_side_requires_independent_complete_storage_proof(tmp_path, version, damage):
    old = build(tmp_path / "old", damage=damage if version == "old" else None)
    new = build(tmp_path / "new", damage=damage if version == "new" else None)
    report = compare_common_builds(old, new, ("_g",))
    assert report["result"] == ("fail" if damage == "nonzero" else "unknown"), report
    assert old.checks == new.checks


@pytest.mark.parametrize("names", [None, "_g", (), ("_g", "_g"), ("",), (True,), ([],),
                                    ("bad name",), ("_g\0",), tuple(f"s{i}" for i in range(4097))])
def test_invalid_or_unbounded_requests_refuse_without_compiling(tmp_path, names):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    report = compare_common_builds(old, new, names)
    assert report["result"] == "unknown" and "unique nonempty" in report["reason"]
    assert old.checks == new.checks == 2


@pytest.mark.parametrize("version", ["old", "new"])
@pytest.mark.parametrize("when", [1, 4])
@pytest.mark.parametrize("damage", ["expired", "artifact", "input"])
def test_both_epochs_checked_on_entry_and_after_observation(tmp_path, version, when, damage):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    context = old if version == "old" else new
    def mutate(count):
        if count != when:
            return
        if damage == "expired":
            context.alive = False
        else:
            path = context.directory / "app.exe" if damage == "artifact" else context.root / "runtime.lib"
            path.write_bytes(b"changed after build")
    context.on_check = mutate
    with pytest.raises(ValueError, match="synthetic context"):
        compare_common_builds(old, new, ("_g",))
    assert old.checks == new.checks


def test_early_observation_refusal_still_checks_both_epochs(tmp_path):
    old, new = build(tmp_path / "old"), build(tmp_path / "new")
    old.report["outputs"] = []
    def expire(count):
        if count == 3:
            new.alive = False
    new.on_check = expire
    with pytest.raises(ValueError, match="expired"):
        compare_common_builds(old, new, ("_g",))
    assert old.checks == new.checks == 3


@pytest.mark.parametrize("damage", ["empty", "identity", "bool-size", "bool-address", "initialization", "aggregate"])
def test_private_algebra_does_not_upgrade_incomplete_observations(tmp_path, damage):
    from match_recovery.common_preservation import _comparison
    report = compare_common_builds(build(tmp_path / "old"), build(tmp_path / "new"), ("_g",))
    observations = {version: report[version] for version in ("baseline", "current")}
    if damage == "empty":
        for value in observations.values():
            value["allocations"] = []
    elif damage == "aggregate":
        observations["baseline"]["result"] = "unknown"
    else:
        row = observations["current"]["allocations"][0]
        key, value = {"identity": ("symbol", "_other"), "bool-size": ("size", True),
                      "bool-address": ("address", True), "initialization": ("initialization", "unknown")}[damage]
        row[key] = value
    if damage in {"empty", "aggregate"}:
        assert _comparison(observations)["result"] == "unknown"
    else:
        with pytest.raises(ValueError):
            _comparison(observations)


@pytest.mark.parametrize("size,expected", [(4, "pass"), (8, "fail")])
def test_shared_parsed_build_path_matches_independent_bytes_path(tmp_path, size, expected):
    from match_recovery.common_preservation import _compare_linked
    from match_recovery.linked_context import linked_build
    old, new = build(tmp_path / "old"), build(tmp_path / "new", size=size, offset=16, extra=True)
    wrapped = compare_common_builds(old, new, ("_g",))
    with linked_build(old) as old_linked, linked_build(new) as new_linked:
        report = _compare_linked(old_linked, new_linked, ("_g",))
        assert report["result"] == expected
        assert all(wrapped[key] == value for key, value in report.items())
    with pytest.raises(ValueError, match="expired"):
        _compare_linked(old_linked, new_linked, ("_g",))


def test_a_prior_start_overlap_is_not_allocation_isolation_proof(tmp_path):
    from test_linked_initializers import update_artifact
    old, new = build(tmp_path / "old", offset=4), build(tmp_path / "new", offset=4)
    # A different allocation starts before the requested span and extends
    # into it. Querying only _g can prove its bytes/size, never isolation.
    path = new.root / "overlap.obj"
    path.write_bytes(common_object(8).replace(b"_g\0", b"_h\0"))
    evidence = json.loads((new.directory / "link.json").read_bytes())
    evidence["explicit_inputs"].append(str(path))
    evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    mapping = (new.directory / "app.map").read_text().replace(
        " entry point", " 0002:00000000 _h 00402000 <common>\n entry point")
    update_artifact(new, "link.json", json.dumps(evidence).encode())
    update_artifact(new, "app.map", mapping.encode())
    result = compare_common_builds(old, new, ("_g",))
    assert result["result"] == "pass"  # Only the expressly bounded dimensions.
    assert result["allocation_isolation"] == result["remaining_linked_effects"] == "unknown"
    assert not result["ready_for_collection"]
