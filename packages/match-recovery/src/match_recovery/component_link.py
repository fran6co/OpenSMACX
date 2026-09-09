"""Pinned library code versus a freshly linked image, not original acceptance.

Map placements are independently checked against PE coordinates. The actual
image must agree with archive instructions, relocated operands and loader
fixups. Target storage, dependency closure and original equivalence remain
separate obligations; this module never exports application target bindings.
"""

from collections import Counter
import hashlib
from pathlib import Path, PurePosixPath

from .binary import compare_bound_code
from .coff_archive import CoffArchive
from .components import ToolchainManifest
from .import_source import import_targets
from .model import Result, Span, require_text
from .msvc_link_map import parse_link_map
from .pe_coff import Coff, PeImage


def verify_linked_toolchain(manifest: ToolchainManifest, archive_data: bytes, image_data: bytes,
                            map_data: bytes, *, library: str, import_bindings=()) -> dict:
    """Bounded one-COMDAT-per-function check at actual candidate placements.

    `library` is the explicit Lib:Object label, not inferred archive identity.
    Original source spans constrain length; neither neighboring map symbols nor
    candidate bytes redefine those spans. Supplied bytes alone prove no freshness;
    the public CLI obtains both outputs from one new configured build.
    """
    require_text(library, "link map library label")
    if any(char.isspace() or char in ":/\\" for char in library):
        raise ValueError("expected an unqualified link map library label")
    actual = hashlib.sha256(archive_data).hexdigest()
    archive_ok = actual == manifest.archive_sha256
    report = {"check": "toolchain_linked_code", "component_id": manifest.component.component_id,
              "archive": {"sha256": actual, "result": Result.PASS if archive_ok else Result.FAIL},
              "image_sha256": hashlib.sha256(image_data).hexdigest(),
              "map_sha256": hashlib.sha256(map_data).hexdigest(), "library": library,
              "functions": [], "import_bindings": [], "member_reads": 0,
              "original_verification": Result.UNKNOWN, "storage_verification": Result.UNKNOWN,
              "structure_verification": Result.UNKNOWN, "runtime_verification": Result.UNKNOWN,
              "persisted": False, "ready_for_collection": False,
              "limitations": ["bounded linked code correspondence, not full dependency admission or original equivalence",
                              "map owner labels and matching code do not prove the linker's archive path or whole-library selection",
                              "map targets do not prove target contents, data allocation extent, ownership or initialization",
                              "weak symbols, local data and associated code fragments remain unsupported",
                              "no application target bindings are published"]}
    if not archive_ok:
        return {**report, "result": Result.FAIL, "counts": {}}
    from .coff_inventory import inventory_archive
    inventory = inventory_archive(archive_data)
    # Surface the whole supplied archive's declarations without making parsing
    # success a code/selection verdict. Full record detail is available via the
    # inventory API; normal link reports retain common requests and format gaps.
    report["archive_inventory"] = {key: value for key, value in inventory.items() if key != "members"}
    image = PeImage(image_data, report["image_sha256"])
    mapping = parse_link_map(map_data)
    mapping.validate(image)
    publics = {item.symbol: item for item in mapping.publics}
    archive = CoffArchive(archive_data)
    objects, errors = {}, {}
    for name in sorted({item.unit for item in manifest.bindings}):
        try:
            objects[name] = Coff(archive.member(name))
        except ValueError as error:
            errors[name] = str(error)
    report["member_reads"] = len(objects) + len(errors)
    if {item.symbol for item in manifest.bindings} & {item.symbol for item in import_bindings}:
        raise ValueError("component symbol is bound as both function and import")
    imports, report["import_bindings"] = import_targets(import_bindings, objects, image)
    import_names = {item.symbol for item in import_bindings}
    import_slots = {item.address for item in image.imports()}
    for binding in manifest.bindings:
        row = {"symbol": binding.symbol, "unit": binding.unit, "original_address": binding.origin.address}
        try:
            if binding.unit in errors:
                raise ValueError(errors[binding.unit])
            obj = objects[binding.unit]
            subject = obj.function(binding.symbol)
            if subject.storage != 2 or len(obj.code_sections(subject)) != 1 or len(binding.spans) != 1:
                raise ValueError("linked library check requires one external code COMDAT and one original span")
            selected = publics.get(binding.symbol)
            member = PurePosixPath(binding.unit).name
            # Maps abbreviate archive member paths. Never let two different
            # members with the same basename silently select the requested one.
            candidates = [name for name in archive.members if PurePosixPath(name).name.casefold() == member.casefold()]
            if len(candidates) != 1:
                raise ValueError("ambiguous archive member basename in link map")
            if selected is None or not selected.function or selected.segment == 0:
                raise ValueError("missing placed public function in link map")
            row.update(linked_address=selected.address, map_owner=selected.owner)
            if selected.owner.casefold() != (library + ":" + member).casefold():
                row.update(result=Result.FAIL, reason="link map selected a different library/member")
                report["functions"].append(row)
                continue
            code, relocations, alignment = obj.subject(subject)
            targets, observations = {}, []
            for index, symbol in obj.symbols.items():
                target, basis = None, None
                if symbol.name in import_names:
                    target, basis = imports.get(symbol.name), "source_import_identity"
                elif symbol.section == subject.section:
                    target, basis = selected.address + symbol.value, "same_code_section"
                elif symbol.storage == 2 and symbol.section >= 0:
                    found = publics.get(symbol.name)
                    if found is not None and found.segment and found.address not in import_slots:
                        target, basis = found.address, "public_placement_only"
                if target is not None:
                    targets[str(index)] = target
                    if any(relocation.target == str(index) for relocation in relocations):
                        observations.append({"symbol": symbol.name, "address": target, "basis": basis})
            size = binding.spans[0].end - binding.spans[0].start
            span = Span(selected.address, selected.address + size)
            if not image.section_flags[selected.segment - 1] & 0x20:
                raise ValueError("linked function is not in a code section")
            proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                                       original_absolute_sites=image.absolute_sites(span), alignment=alignment)
            for relocation in proof.get("relocations", []):
                relocation["symbol"] = obj.symbols[int(relocation["target"])].name
            row.update(proof, target_placements=observations)
        except ValueError as error:
            row.update(result=Result.UNKNOWN, reason=str(error))
        report["functions"].append(row)
    results = [row["result"] for row in report["functions"]]
    report.update(counts=dict(sorted(Counter(results).items())),
                  result=Result.FAIL if Result.FAIL in results else Result.UNKNOWN
                  if not results or Result.UNKNOWN in results else Result.PASS)
    return report


def check_toolchain_link(root: Path, manifest: ToolchainManifest, archive_path: Path, *, library: str) -> dict:
    """Build and inspect together; no report replay or preexisting output input."""
    from .build_check import Build, generate_build
    from .driver import changes, working_state
    from .import_source import read_imports
    from .source_inventory import source_snapshot, assert_snapshot_current

    root = root.resolve()
    archive_path = Path(archive_path)
    archive_path = archive_path if archive_path.is_absolute() else root / archive_path
    archive_data = archive_path.read_bytes()
    if hashlib.sha256(archive_data).hexdigest() != manifest.archive_sha256:
        return {"check": "toolchain_linked_code", "result": Result.FAIL,
                "reason": "archive digest mismatch", "build_attempts": 0,
                "persisted": False, "ready_for_collection": False}
    settings = Build.read(root)
    expected = {format_name: [name for name, kind in settings.outputs if kind == format_name]
                for format_name in ("pe32-i386", "msvc-map")}
    if any(len(names) != 1 for names in expected.values()):
        raise ValueError("linked verification requires exactly one configured PE32 and one MSVC map output")
    state, before = working_state(root), settings.snapshot(root)
    source = source_snapshot(root, allow_empty=True)
    imports = read_imports(root)
    assert_snapshot_current(root, source)
    build = generate_build(root, settings, retain=True)
    artifacts = {}
    if build["result"] != Result.PASS:
        result = {"check": "toolchain_linked_code", "result": build["result"],
                  "reason": "fresh configured build did not pass", "persisted": False,
                  "ready_for_collection": False}
    else:
        directory = root / build["directory"]
        for output in build["outputs"]:
            if output["format"] not in expected:
                continue
            path = directory / output["path"]
            if not path.is_file() or any(part.is_symlink() for part in (path, *path.parents)
                                         if part.is_relative_to(root)):
                raise ValueError("linked build output is no longer a regular artifact")
            data = path.read_bytes()
            if hashlib.sha256(data).hexdigest() != output["sha256"]:
                raise ValueError("linked build output changed before verification")
            artifacts[output["format"]] = (path, data)
        result = verify_linked_toolchain(manifest, archive_data,
                                         artifacts["pe32-i386"][1], artifacts["msvc-map"][1],
                                         library=library, import_bindings=imports)
    assert_snapshot_current(root, source)
    if (settings.snapshot(root) != before or changes(state, working_state(root))
            or archive_path.read_bytes() != archive_data
            or any(any(part.is_symlink() for part in (path, *path.parents) if part.is_relative_to(root))
                   or path.read_bytes() != data for path, data in artifacts.values())):
        raise ValueError("inputs or linked artifacts changed during component verification")
    return {**result, "build_attempts": 1, "fresh_build": build}
