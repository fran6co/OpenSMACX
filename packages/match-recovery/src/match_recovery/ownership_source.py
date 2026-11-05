"""Derive ownership candidates and structural impact from the current source map.

Legacy annotations are inputs at this boundary, not a permanent dependency of
the portable partition/task layers. No generated graph or census is persisted.
"""

import base64
from pathlib import Path
import re

from match_recovery.component_source import parse_component_entries
from match_recovery.components import ToolchainManifest, VendorManifest
from match_recovery.includes import include_graph
from match_recovery.model import Origin, Span
from match_recovery.partition import Candidate, partition, select_group
from match_recovery.source_inventory import source_snapshot
from .source_config import SourceConfiguration
from .component_original_routing import exact_component_record_bindings


def class_hint(symbol: str) -> str | None:
    from match_recovery.source.mangled import qualified_name

    # Deleting entries (including their adjusted entries) encode their class,
    # not a separate source-level thunk class. Keep this a name hint: it says
    # nothing about whether the candidate hierarchy or lifetime is correct.
    if symbol.startswith(("??_G", "??_E")):
        owner, _, operation = qualified_name(symbol).rpartition("::")
        if (operation in ("`scalar deleting destructor'", "`vector deleting destructor'")
                and re.fullmatch(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*", owner)):
            return owner
        return None  # unsupported scopes/templates must not invent an owner
    # Initializers name objects, not necessarily classes. Other special forms
    # need their own interpretation before supplying an ownership hint.
    if symbol.startswith("??") and not symbol.startswith(("??0", "??1")):
        return None
    # W and $4 are adjusted entries (fixed displacement and vtordisp). Their
    # qualified member names supply the same provisional class hint as U.
    if not symbol.startswith(("??0", "??1")) and not re.match(r'^\?[^@]+@.+?@@(?:[ABCDEFIJKLMNQRSTUVW]|\$4)', symbol):
        return None  # a namespace-qualified free function is not a class method
    name = qualified_name(symbol)
    if "::" not in name or "`" in name or "'" in name:
        return None
    return name.rsplit("::", 1)[0]


def component_entries(snapshot: dict):
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    sources = {name: base64.b64decode(row["content_base64"]).decode("utf-8")
               for name, row in snapshot["files"].items()
               if name.endswith(".recovery.h") and not configuration.is_seed(name)}
    return parse_component_entries(sources)


def component_manifests(snapshot: dict):
    return [manifest for _, manifest in component_entries(snapshot)]


def project_view(root: Path, image: Path | None = None, *, allow_empty: bool = False,
                 snapshot: dict | None = None) -> tuple[dict, dict]:
    if snapshot is None:
        snapshot = source_snapshot(root, image, allow_empty=allow_empty)
    elif image is not None:
        from .source_inventory import image_pin
        if image_pin(root, SourceConfiguration.from_dict(snapshot["source_configuration"])) != snapshot["image_sha256"]:
            raise ValueError("provided source snapshot does not match the configured image pin")
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    component_owners = exact_component_record_bindings(snapshot)
    candidates = []
    for row in snapshot["records"]:
        seed = row["candidate_kind"] == "legacy_seed"
        unbound = row.get("legacy_mode") == "unbound"
        catalogued = class_hint(row["catalogued_symbol"]) if configuration.symbol_format == "msvc" else None
        emitted = (class_hint(row["emitted_symbol_hint"])
                   if configuration.symbol_format == "msvc" and not seed else None)
        # Seed-emitted classes can be arbitrary compiler scaffolds. Product
        # member bindings describe today's source home; retain the catalogue's
        # different interpretation alongside it rather than rewriting history.
        candidates.append(Candidate(Origin(**row["origin"]),
                                    tuple(Span(*span) for span in row["spans"] if span not in row["invalid_spans"]),
                                    row["path"], row["line"], seed,
                                    "" if unbound else row["emitted_symbol_hint"],
                                    None if unbound else emitted or catalogued,
                                    component=(None if unbound else
                                               (component_owners.get((row["path"], row["entity_id"])) or (None,))[0]),
                                    owner_hint_basis=("catalogued_symbol" if unbound or not emitted
                                                      else "source_symbol"),
                                    catalogued_owner_hint=catalogued,
                                    function_fragment=None if unbound else row.get("function_fragment"),
                                    context_path="" if unbound else row.get("context_path", ""),
                                    unbound=unbound))
    for annotation, manifest in component_entries(snapshot):
        for binding in manifest.bindings:
            if (binding.origin.image_sha256 != snapshot["image_sha256"]
                    or binding.origin.address_space != configuration.address_space):
                raise ValueError("component bindings refer to a different original image/address space")
            # One exact companion marker carries the historical notes/claims;
            # the manifest contributes its component owner, not a duplicate
            # candidate for the same original identity.
            if (annotation, binding.origin.entity_id) in component_owners:
                continue
            candidates.append(Candidate(binding.origin, binding.spans,
                                        (manifest.source_root + "/" + binding.unit
                                         if isinstance(manifest, VendorManifest) else annotation), 1, False,
                                        binding.symbol, component=manifest.component.component_id))
        if isinstance(manifest, ToolchainManifest):
            declared_function_entries = {
                (binding.unit, binding.symbol, binding.origin.image_sha256, binding.origin.address)
                for binding in manifest.bindings}
            records_by_entity = {}
            for record in snapshot["records"]:
                records_by_entity.setdefault(record["entity_id"], []).append(record)
            for section in manifest.code_sections:
                for anchor in section.entries:
                    if (section.origin.image_sha256 != snapshot["image_sha256"]
                            or section.origin.address_space != configuration.address_space):
                        raise ValueError("component code entries refer to a different original image/address space")
                    origin = Origin(section.origin.image_sha256, section.origin.address_space,
                                    section.span.start + anchor.offset)
                    if (section.unit, anchor.name, origin.image_sha256, origin.address) in declared_function_entries:
                        # The existing function binding already owns this exact
                        # original identity; do not add a duplicate partition candidate.
                        continue
                    matching_records = records_by_entity.get(origin.entity_id, ())
                    existing_component = False
                    duplicate_legacy = False
                    for record in matching_records:
                        key = (record["path"], record["entity_id"])
                        names = {record.get("emitted_symbol_hint"), record.get("catalogued_symbol")}
                        if record.get("candidate_kind") == "legacy_seed":
                            # Historical seeds are retained as context, not a
                            # competing current owner for a component entry.
                            duplicate_legacy = duplicate_legacy or anchor.name in names
                            continue
                        owner = component_owners.get(key)
                        if owner is not None:
                            if (owner[0] == manifest.component.component_id
                                    and owner[2].symbol == anchor.name
                                    and owner[2].unit == section.unit):
                                existing_component = True
                                continue
                            raise ValueError("code-entry component owner conflicts with a different component binding")
                        if names or record.get("candidate_kind") == "source":
                            raise ValueError("code-entry component owner conflicts with a product source mapping")
                    if existing_component or duplicate_legacy:
                        continue
                    candidates.append(Candidate(origin, (), annotation, 1, False,
                                                anchor.name,
                                                component=manifest.component.component_id))
    return partition(candidates), snapshot


def ownership_files(snapshot: dict, group: dict) -> list[str]:
    """Initial source/interface scope; not transitive editing authorization."""
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    roots = set(group["source_homes"])
    headers = {name.rsplit(".", 1)[0] + suffix for name in roots for suffix in configuration.interface_extensions}
    # UNBOUND annotation hosts are editable evidence locations, but remain
    # pending mappings rather than source homes or historical TU owners.
    pending_hosts = {candidate["path"] for entity in group["entities"]
                     for candidate in entity["candidates"] if candidate.get("unbound")}
    return sorted(roots | pending_hosts | (headers & snapshot["files"].keys()))


def structural_impact(snapshot: dict, group: dict) -> dict:
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    sources = {name: base64.b64decode(record["content_base64"]).decode("utf-8", errors="replace")
               for name, record in snapshot["files"].items()}
    graph, widened = include_graph(sources, configuration.include_paths)
    # A source home's own interface is an initial structural-scope candidate.
    # All transitive includes are context; reverse includers are recheck targets.
    roots = set(ownership_files(snapshot, group))
    impacted = graph.impacted(roots)
    root_ids = {entity["entity_id"] for entity in group["entities"]}
    entities = sorted(root_ids | {record["entity_id"] for record in snapshot["records"]
                                  if record["path"] in impacted})
    return {"root_files": sorted(roots), "context_files": sorted(graph.prerequisites(roots)),
            "recheck_files": sorted(impacted), "recheck_entities": entities,
            "conservative_include_inputs": list(widened),
            "scope_complete": False,
            "limitations": ["include impact does not prove class/global ownership",
                            "indirect calls and unmodelled layout relationships still require discovery"]}


def partition_report(root: Path, *, group: str | None = None, details: bool = False,
                     impact: bool = False) -> dict:
    if impact and not group:
        raise ValueError("--impact needs one --group")
    view, snapshot = project_view(root)
    groups = [select_group(view, group)] if group else view["groups"]
    if impact:
        groups[0]["impact"] = structural_impact(snapshot, groups[0])
    if not details:
        groups = [{key: value for key, value in item.items() if key != "entities"} for item in groups]
    return {**view, "groups": groups}
