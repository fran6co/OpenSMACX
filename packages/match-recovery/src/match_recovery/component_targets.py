"""Fresh toolchain-component verification publishing scoped relocation targets.

A component annotation alone is never evidence that the linked build uses the
declared library code. When the pinned archive freshly verifies against the
live image — hash pin, member reads and relocation-aware checks — its bound
symbols may resolve task-scoped relocations against their verified original
origins. Everything unverified stays unresolved; this publishes targets, not
product claims or acceptance.
"""

import base64
from pathlib import Path

from .binary import AbsoluteConstant
from .component_binary import verify_toolchain
from .components import SystemImportManifest, ToolchainManifest
from .model import Result
from .records import read_json
from .import_source import parse_imports
from .source_config import SourceConfiguration


def component_targets(root: Path, snapshot: dict, image_data: bytes) -> tuple[dict, list]:
    """Return ({symbol: address or AbsoluteConstant}, reports) from fresh components.

    Configured archives whose verification does not pass contribute no
    targets and keep their symbols unresolved. A missing or invalid
    configuration refuses rather than inferring any dependency identity.
    """
    from .ownership_source import component_entries
    configuration = read_json(root / "recovery/project.json")
    declared = configuration.get("components") if isinstance(configuration, dict) else None
    configured = {}
    for kind in ("toolchain_archives", "system_import_archives"):
        value = declared.get(kind) if isinstance(declared, dict) else None
        if value is None:
            configured[kind] = {}
            continue
        if (not isinstance(value, dict) or not value
                or not all(isinstance(key, str) and isinstance(path, str) and path
                           for key, path in value.items())):
            raise ValueError(f"component {kind} must map component ids to archive paths")
        configured[kind] = value
    archives = configured["toolchain_archives"]
    system_archives = configured["system_import_archives"]
    if not archives and not system_archives:
        return {}, []
    targets, reports = {}, []
    interpretations = {}
    configuration_source = SourceConfiguration.from_dict(snapshot["source_configuration"])
    imports = tuple(binding for name, file in snapshot["files"].items()
                    if (Path(name).suffix in configuration_source.extensions
                        and any(name == source_root or name.startswith(source_root + "/")
                                for source_root in configuration_source.roots))
                    if not configuration_source.is_seed(name)
                    for binding in parse_imports(
                        base64.b64decode(file["content_base64"]).decode("utf-8"), name))
    if len({item.symbol for item in imports}) != len(imports):
        raise ValueError("duplicate source import symbol binding in component target snapshot")
    imported_names = {item.symbol for item in imports}
    from .component_original_routing import exact_component_record_bindings
    component_records = exact_component_record_bindings(snapshot)
    product_names = set()
    for record in snapshot.get("records", ()):
        if (record.get("path"), record.get("entity_id")) in component_records:
            continue
        if record.get("candidate_kind") == "source":
            product_names.update(symbol for symbol in
                                 (record.get("emitted_symbol_hint"), record.get("catalogued_symbol"))
                                 if isinstance(symbol, str))
    for _, manifest in component_entries(snapshot):
        if isinstance(manifest, SystemImportManifest):
            path = system_archives.get(manifest.component.component_id)
            if path is None:
                continue
            report = manifest.verify(image_data, Path(path).read_bytes(), imports)
            reports.append(report)
            if report["archive"]["result"] != Result.PASS:
                continue
            for row in report["functions"]:
                if any(row[dimension] != Result.PASS
                       for dimension in ("boundary", "bytes", "references")):
                    continue
                # The short record supplies the decorated function symbol.
                # Its distinct __imp_ pointer still resolves only through the
                # source import declaration and original loader table. Neither
                # target establishes current-link selection or a DLL body.
                symbol = row["physical_record"]["symbol"]
                interpretations.setdefault(symbol, []).append(int(row["address"], 16))
            continue
        if not isinstance(manifest, ToolchainManifest):
            continue
        if {item.symbol for item in manifest.data_starts} & imported_names:
            raise ValueError("component symbol is bound as both data start and import")
        path = archives.get(manifest.component.component_id)
        if path is None:
            continue
        report = verify_toolchain(manifest, image_data, Path(path).read_bytes())
        reports.append(report)
        if report["archive"]["result"] != Result.PASS:
            continue
        for row in report["functions"]:
            # A failed row contradicts the recorded extent or bytes and must
            # not publish. A passing or unresolved-relocation row verified its
            # extent and body; deeper CRT-internal obligations stay with the
            # component and never transfer to the scoped caller.
            if (row.get("result") == Result.FAIL or row.get("boundary") != Result.PASS
                    or row.get("bytes") not in (Result.PASS, Result.UNKNOWN)
                    or (row.get("bytes") == Result.UNKNOWN and row.get("differing_offsets") != [])
                    or row.get("references") not in (Result.PASS, Result.UNKNOWN)):
                continue
            value = (row["address"] if type(row["address"]) is int
                     else int(str(row["address"]), 16))
            interpretations.setdefault(row["symbol"], []).append(value)
        for section in report.get("code_sections", ()):
            for row in section.get("entries", ()):
                if (section.get("result") == Result.PASS
                        and section.get("boundary") == Result.PASS
                        and section.get("bytes") == Result.PASS
                        and section.get("references") == Result.PASS
                        and section.get("entry_inventory") == Result.PASS
                        and row.get("result") == Result.PASS and row.get("callable") is True
                        and row.get("scoped_reference_target") is True
                        and row.get("link_target") is False
                        and row.get("function_extent") == Result.UNKNOWN):
                    value = (int(str(row["address"]), 16) if isinstance(row.get("address"), str)
                             else row.get("address"))
                    if type(value) is int:
                        interpretations.setdefault(row["symbol"], []).append(value)
        for row in report.get("absolute_symbols", ()):
            # A member-verified absolute constant resolves every link's fields
            # to the same value; an unverified one stays unresolved.
            if row["result"] == Result.PASS:
                interpretations.setdefault(row["symbol"], []).append(
                    AbsoluteConstant(int(str(row["value"]), 16)))
    for symbol, values in interpretations.items():
        unique = set(values)
        if len(unique) != 1 or symbol in imported_names:
            continue
        value = next(iter(unique))
        if symbol in product_names:
            product_values = {record["origin"]["address"] for record in snapshot.get("records", ())
                              if record.get("candidate_kind") == "source"
                              and (record.get("path"), record.get("entity_id")) not in component_records
                              and symbol in (record.get("emitted_symbol_hint"),
                                             record.get("catalogued_symbol"))}
            if product_values != {value}:
                continue
        targets[symbol] = value
    return targets, reports
