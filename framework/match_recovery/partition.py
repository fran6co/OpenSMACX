"""A derived ownership view, not a claim to have recovered historical TUs.

Adapters supply evidence-bearing hints. Address proximity never assigns an owner,
and neither unknown ownership nor duplicate seeds may silently disappear.
"""

from collections import defaultdict
from dataclasses import asdict, dataclass
from typing import Iterable

from .components import relative_path
from .model import Origin, Span, require_text


@dataclass(frozen=True)
class Candidate:
    origin: Origin
    spans: tuple[Span, ...]
    path: str
    line: int
    seed: bool
    symbol: str
    owner_hint: str | None = None
    component: str | None = None

    def __post_init__(self):
        relative_path(self.path)
        if type(self.line) is not int or self.line < 1 or type(self.seed) is not bool:
            raise ValueError("invalid source candidate location")
        if self.owner_hint is not None:
            require_text(self.owner_hint, "owner hint")
        if self.component is not None:
            require_text(self.component, "component binding")


def partition(candidates: Iterable[Candidate]) -> dict:
    """Group evidence without selecting a winning body or inventing a layout."""
    by_entity = defaultdict(list)
    for candidate in candidates:
        by_entity[candidate.origin.entity_id].append(candidate)
    groups = defaultdict(list)
    for entity_id, items in sorted(by_entity.items()):
        items.sort(key=lambda item: (item.path, item.line, item.symbol))
        owners = sorted({item.owner_hint for item in items if item.owner_hint})
        components = sorted({item.component for item in items if item.component})
        sources = sorted({item.path for item in items if not item.seed})
        extents = {item.spans for item in items if item.spans}
        reasons = []
        if len(components) > 1:
            reasons.append("conflicting component assignments")
        if len(owners) > 1:
            reasons.append("conflicting owner hints")
        if len(extents) > 1:
            reasons.append("conflicting original extents")
        if len(sources) > 1 and not components:
            reasons.append("multiple current source candidates")
        if reasons:
            group = "conflict"
        elif components:
            group = "component:" + components[0]
        elif owners:
            group = "class:" + owners[0]
        elif len(sources) == 1:
            group = "source:" + sources[0]
        else:
            group = "unassigned"
        groups[group].append({"entity_id": entity_id, "origin": asdict(items[0].origin),
                              "candidates": [asdict(item) for item in items], "conflicts": reasons})
    result = []
    for key, entities in sorted(groups.items()):
        entities.sort(key=lambda entity: (entity["origin"]["image_sha256"],
                                          entity["origin"]["address_space"], entity["origin"]["address"]))
        paths = sorted({item["path"] for entity in entities for item in entity["candidates"] if not item["seed"]})
        result.append({"selector": key, "historical_tu": "unproven", "source_homes": paths,
                       "next_action": ("resolve_conflict" if key == "conflict" else
                                       "identify_owner" if key == "unassigned" else
                                       "verify_dependency" if key.startswith("component:") else
                                       "review_ownership_and_structure"),
                       "entity_count": len(entities), "entities": entities})
    return {"schema_version": 1, "entity_count": len(by_entity), "groups": result,
            "historical_tu_boundaries": "not_inferred", "persisted": False}


def select_group(view: dict, selector: str) -> dict:
    matches = [group for group in view["groups"] if group["selector"] == selector]
    if len(matches) != 1:
        raise ValueError(f"expected exactly one ownership group {selector!r}")
    return matches[0]
