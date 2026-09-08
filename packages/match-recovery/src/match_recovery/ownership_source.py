"""Derive ownership candidates and structural impact from the current source map.

Legacy annotations are inputs at this boundary, not a permanent dependency of
the portable partition/task layers. No generated graph or census is persisted.
"""

import base64
from pathlib import Path
import re

from match_recovery.component_source import parse_components
from match_recovery.includes import include_graph
from match_recovery.model import Origin, Span
from match_recovery.partition import Candidate, partition, select_group
from match_recovery.source_inventory import source_snapshot
from .source_config import SourceConfiguration


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


def component_manifests(snapshot: dict):
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    return parse_components({name: base64.b64decode(row["content_base64"]).decode("utf-8")
                             for name, row in snapshot["files"].items()
                             if name.endswith(".recovery.h") and not configuration.is_seed(name)})


def project_view(root: Path, image: Path | None = None, *, allow_empty: bool = False) -> tuple[dict, dict]:
    snapshot = source_snapshot(root, image, allow_empty=allow_empty)
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    candidates = []
    for row in snapshot["records"]:
        seed = row["candidate_kind"] == "legacy_seed"
        catalogued = class_hint(row["catalogued_symbol"]) if configuration.symbol_format == "msvc" else None
        emitted = (class_hint(row["emitted_symbol_hint"])
                   if configuration.symbol_format == "msvc" and not seed else None)
        # Seed-emitted classes can be arbitrary compiler scaffolds. Product
        # member bindings describe today's source home; retain the catalogue's
        # different interpretation alongside it rather than rewriting history.
        candidates.append(Candidate(Origin(**row["origin"]),
                                    tuple(Span(*span) for span in row["spans"] if span not in row["invalid_spans"]),
                                    row["path"], row["line"], seed,
                                    row["emitted_symbol_hint"], emitted or catalogued,
                                    owner_hint_basis="source_symbol" if emitted else "catalogued_symbol",
                                    catalogued_owner_hint=catalogued))
    manifests = component_manifests(snapshot)
    for manifest in manifests:
        for binding in manifest.bindings:
            if (binding.origin.image_sha256 != snapshot["image_sha256"]
                    or binding.origin.address_space != configuration.address_space):
                raise ValueError("component bindings refer to a different original image/address space")
            candidates.append(Candidate(binding.origin, binding.spans,
                                        manifest.source_root + "/" + binding.unit, 1, False,
                                        binding.symbol, component=manifest.component.component_id))
    return partition(candidates), snapshot


def ownership_files(snapshot: dict, group: dict) -> list[str]:
    """Initial source/interface scope; not transitive editing authorization."""
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    roots = set(group["source_homes"])
    headers = {name.rsplit(".", 1)[0] + suffix for name in roots for suffix in configuration.interface_extensions}
    return sorted(roots | (headers & snapshot["files"].keys()))


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
