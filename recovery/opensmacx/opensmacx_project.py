"""Derive ownership candidates and structural impact from the current source map.

Legacy annotations are inputs at this boundary, not a permanent dependency of
the portable partition/task layers. No generated graph or census is persisted.
"""

import base64
from pathlib import Path
import re

from match_recovery.component_source import read_components
from match_recovery.includes import include_graph
from match_recovery.model import Origin, Span
from match_recovery.partition import Candidate, partition
from .opensmacx import legacy_snapshot


def class_hint(symbol: str) -> str | None:
    from decomp.mangled import qualified_name

    # Initializers/deleting thunks need ownership analysis, not an invented
    # class based on a quoted demangler description or an encoded variable name.
    if symbol.startswith("??") and not symbol.startswith(("??0", "??1")):
        return None
    if not symbol.startswith(("??0", "??1")) and not re.match(r'^\?[^@]+@.+?@@[ABCDEFIJKLMNQRSTUV]', symbol):
        return None  # a namespace-qualified free function is not a class method
    name = qualified_name(symbol)
    if "::" not in name or "`" in name or "'" in name:
        return None
    return name.rsplit("::", 1)[0]


def project_view(root: Path, image: Path | None = None) -> tuple[dict, dict]:
    snapshot = legacy_snapshot(root, image)
    candidates = []
    for row in snapshot["records"]:
        candidates.append(Candidate(Origin(**row["origin"]),
                                    tuple(Span(*span) for span in row["spans"] if span not in row["invalid_spans"]),
                                    row["path"], row["line"], row["candidate_kind"] == "legacy_seed",
                                    row["emitted_symbol_hint"], class_hint(row["catalogued_symbol"])))
    for manifest in read_components(root / "src"):
        for binding in manifest.bindings:
            if binding.origin.image_sha256 != snapshot["image_sha256"]:
                raise ValueError("component bindings refer to a different original image")
            candidates.append(Candidate(binding.origin, binding.spans,
                                        manifest.source_root + "/" + binding.unit, 1, False,
                                        binding.symbol, component=manifest.component.component_id))
    return partition(candidates), snapshot


def structural_impact(snapshot: dict, group: dict) -> dict:
    sources = {name: base64.b64decode(record["content_base64"]).decode("utf-8", errors="replace")
               for name, record in snapshot["files"].items()}
    graph, widened = include_graph(sources, ("src",))
    roots = set(group["source_homes"])
    # A source home's own interface is an initial structural-scope candidate.
    # All transitive includes are context; reverse includers are recheck targets.
    headers = {name.rsplit(".", 1)[0] + ".h" for name in roots}
    roots.update(headers & sources.keys())
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
