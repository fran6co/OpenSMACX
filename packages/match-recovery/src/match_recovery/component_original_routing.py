"""Exact joins between ORIGINAL markers and component-owned function rows.

An annotation file is not a build unit by virtue of its name. This adapter
requires one full identity/span match to a manifest in that same companion.
Component verification remains a distinct backend from product-TU measurement.
"""

from __future__ import annotations

import base64
from pathlib import Path

from .component_source import parse_component_entries
from .components import SystemImportManifest, ToolchainManifest, relative_path
from .model import Result
from .source_config import SourceConfiguration


def exact_component_record_bindings(snapshot: dict) -> dict[tuple[str, str], tuple[str, object, object]]:
    """Return `(companion path, entity id) -> (component id, manifest, binding)`.

    A row is returned only when exactly one ORIGINAL record in the same
    companion agrees on pinned origin and every full span. Mismatches are not
    repaired from the manifest; they simply do not join.
    """
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    records: dict[tuple[str, str], list[dict]] = {}
    for row in snapshot["records"]:
        records.setdefault((row["path"], row["entity_id"]), []).append(row)
    output = {}
    entries = parse_component_entries({
        name: base64.b64decode(file["content_base64"]).decode("utf-8")
        for name, file in snapshot["files"].items()
        if name.endswith(".recovery.h") and not configuration.is_seed(name)
    })
    for companion, manifest in entries:
        if not isinstance(companion, str) or not companion.endswith(".recovery.h"):
            continue
        for binding in manifest.bindings:
            if (binding.origin.image_sha256 != snapshot["image_sha256"]
                    or binding.origin.address_space != configuration.address_space):
                continue
            key = companion, binding.origin.entity_id
            candidates = records.get(key, [])
            expected = [[span.start, span.end] for span in binding.spans]
            matches = [row for row in candidates
                       if not row["invalid_spans"] and row["spans"] == expected
                       and row.get("function_fragment") is None]
            if len(candidates) == len(matches) == 1 and key not in output:
                output[key] = (manifest.component.component_id, manifest, binding)
    return output


def component_record_identity_candidates(snapshot: dict) -> frozenset[tuple[str, str]]:
    """Raw manifest origin/span candidates, including non-callable fragments.

    This is deliberately not a callable-owner join. Consumers use it only to
    refuse a source-TU shortcut when a component manifest also claims the same
    exact mapped identity; fragment rows remain excluded from the callable join.
    """
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    records: dict[tuple[str, str], list[dict]] = {}
    for row in snapshot["records"]:
        records.setdefault((row["path"], row["entity_id"]), []).append(row)
    entries = parse_component_entries({
        name: base64.b64decode(file["content_base64"]).decode("utf-8")
        for name, file in snapshot["files"].items()
        if name.endswith(".recovery.h") and not configuration.is_seed(name)
    })
    candidates = set()
    for companion, manifest in entries:
        if not isinstance(companion, str) or not companion.endswith(".recovery.h"):
            continue
        for binding in manifest.bindings:
            if (binding.origin.image_sha256 != snapshot["image_sha256"]
                    or binding.origin.address_space != configuration.address_space):
                continue
            expected = [[span.start, span.end] for span in binding.spans]
            for row in records.get((companion, binding.origin.entity_id), ()):
                if not row["invalid_spans"] and row["spans"] == expected:
                    candidates.add((companion, binding.origin.entity_id))
    return frozenset(candidates)


def component_function_observations(root: Path, image: Path, owners: dict) -> tuple[dict, list]:
    """Verify each selected owner through its existing component binary path.

    Missing toolchain archive configuration or absent per-function rows stays
    unknown. Vendor components use their existing compiler-backed verifier,
    which still requires the named physical object function to exist.
    """
    from .component_commands import execute
    from .components import VendorManifest
    from .records import read_json

    root = root.resolve()
    configuration = read_json(root / "recovery/project.json")
    component_options = configuration.get("components", {})
    archives = component_options.get("toolchain_archives", {}) if isinstance(component_options, dict) else {}
    system_archives = component_options.get("system_import_archives", {}) if isinstance(component_options, dict) else {}
    reports, manifests, observations = {}, {}, []
    for component_id, manifest in sorted({value[0]: value[1] for value in owners.values()}.items()):
        arguments = {"action": "check-binary", "component": component_id,
                     "root": root, "image": image}
        if isinstance(manifest, (ToolchainManifest, SystemImportManifest)):
            selected_archives = system_archives if isinstance(manifest, SystemImportManifest) else archives
            archive = selected_archives.get(component_id) if isinstance(selected_archives, dict) else None
            if not isinstance(archive, str) or not archive:
                reports[component_id] = None
                observations.append({"component": component_id, "result": Result.UNKNOWN,
                                    "reason": "no configured archive for this precompiled component"})
                continue
            archive_path = Path(archive)
            if archive_path.is_absolute():
                # Match component_targets/component_commands: an explicitly
                # configured absolute dependency artifact is a valid input.
                arguments["archive"] = archive_path
            else:
                source = SourceConfiguration.read(root)
                relative_path(archive)
                arguments["archive"] = source.path(root, archive)
        elif not isinstance(manifest, VendorManifest):
            reports[component_id] = None
            observations.append({"component": component_id, "result": Result.UNKNOWN,
                                "reason": "unsupported component manifest kind"})
            continue
        try:
            _, report = execute(root, arguments)
        except (OSError, ValueError) as error:
            reports[component_id] = None
            observations.append({"component": component_id, "result": Result.UNKNOWN,
                                "reason": str(error)})
            continue
        reports[component_id] = report
        manifests[component_id] = manifest
        observations.append({"component": component_id, "result": report["result"],
                            "check": report.get("check")})
    functions = {}
    for component_id, report in reports.items():
        if report is None:
            continue
        manifest = manifests[component_id]
        backend = ("system_import_original_thunk" if isinstance(manifest, SystemImportManifest)
                   else "component_binary")
        if isinstance(manifest, (ToolchainManifest, SystemImportManifest)):
            # A passing function subrow is meaningful only after the archive
            # identity itself passed.  Do not let rows synthesized from an
            # unread/unpinned archive route a historical function claim.
            archive_result = report.get("archive", {}).get("result")
            if archive_result != Result.PASS:
                prerequisite = (archive_result if archive_result in (Result.FAIL, Result.UNKNOWN)
                                else Result.UNKNOWN)
                for binding in manifest.bindings:
                    key = binding.origin.entity_id
                    if key in functions:
                        raise ValueError("component backends returned a duplicate original function identity")
                    functions[key] = {
                        "entity_id": key, "prerequisite": prerequisite,
                        "reason": "component archive identity did not pass",
                        "component": component_id, "component_check": report.get("check"),
                        "measurement_backend": backend,
                    }
                    if isinstance(manifest, SystemImportManifest):
                        functions[key]["dependency_verification"] = {
                            "result": archive_result, "archive_selection": Result.UNKNOWN,
                            "link_selection": Result.UNKNOWN,
                        }
                continue
        for row in report.get("functions", ()):
            key = row.get("entity_id")
            if key in functions:
                raise ValueError("component backends returned a duplicate original function identity")
            observed = {**row, "component": component_id,
                        "component_check": report.get("check"),
                        "measurement_backend": backend}
            if isinstance(manifest, SystemImportManifest):
                dimensions = {name: row.get(name) for name in ("bytes", "boundary", "references")}
                dependency_result = (Result.FAIL if Result.FAIL in dimensions.values() else
                                     Result.UNKNOWN if any(value != Result.PASS for value in dimensions.values())
                                     else Result.PASS)
                observed["dependency_verification"] = {
                    "result": dependency_result, "physical_dimensions": dimensions,
                    "physical_record": row.get("physical_record"), "import": row.get("import"),
                    "archive_selection": Result.UNKNOWN, "link_selection": Result.UNKNOWN,
                }
                # These fields feed historical claim promotion. Physical
                # original evidence is not current compiled/link-selected code.
                observed.update(bytes=Result.UNKNOWN, boundary=Result.UNKNOWN,
                                references=Result.UNKNOWN,
                                reason="current emitted thunk and selected link input are unproved")
            functions[key] = observed
    return functions, observations


def component_claim_result(observed: dict | None) -> tuple[str, str]:
    """Collapse the verifier's byte/boundary/reference dimensions for a claim."""
    if observed is None:
        return "unknown", "UNRESOLVED"
    if observed.get("measurement_backend") == "system_import_original_thunk":
        return "unknown", "UNRESOLVED"
    prerequisite = observed.get("prerequisite")
    if prerequisite == Result.FAIL:
        return "fail", "MISMATCH"
    if prerequisite is not None and prerequisite != Result.PASS:
        return "unknown", "UNRESOLVED"
    dimensions = tuple(observed.get(name) for name in ("bytes", "boundary", "references"))
    if any(item == Result.FAIL for item in dimensions):
        return "fail", "MISMATCH"
    if any(item != Result.PASS for item in dimensions):
        return "unknown", "UNRESOLVED"
    return "pass", "BYTE_EXACT"
