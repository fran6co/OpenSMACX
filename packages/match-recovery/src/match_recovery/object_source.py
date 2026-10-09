"""Source-owned original object identities, not inferred address aliases.

An annotation sits beside the owning definition. Its address identifies the
object start, not a pointer variable containing that address. Object extent,
layout, initialization and lifetime remain separate verification obligations.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.model import Origin, Span, digest, require_text
from match_recovery.components import relative_path
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_OBJECT:"
PATTERN = re.compile(r"// RECOVERY_OBJECT: v1 (0x[0-9A-Fa-f]+) (\S+)\s*")
BOUNDED = re.compile(r"// RECOVERY_OBJECT: v2 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)\s*")
MEMBER_UNBOUNDED = re.compile(r"// RECOVERY_OBJECT: v3 (0x[0-9A-Fa-f]+) (\S+) member (\S+) (\S+)\s*")
MEMBER_BOUNDED = re.compile(r"// RECOVERY_OBJECT: v3 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+) member (\S+) (\S+)\s*")
TARGET_UNBOUNDED = re.compile(r"// RECOVERY_OBJECT: v4 (0x[0-9A-Fa-f]+) (\S+) target (\S+)\s*")
TARGET_BOUNDED = re.compile(r"// RECOVERY_OBJECT: v4 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+) target (\S+)\s*")
_MEMBER_SEGMENT = re.compile(r"([A-Za-z_][A-Za-z_0-9]*)(?:\[(0|[1-9][0-9]*)\])?\Z")


def valid_member_path(member_path) -> bool:
    """Accept direct member names and canonical, fixed nonnegative indices."""
    return (isinstance(member_path, tuple) and bool(member_path)
            and all(isinstance(part, str) and _MEMBER_SEGMENT.fullmatch(part)
                    for part in member_path))


@dataclass(frozen=True)
class ObjectBinding:
    origin: Origin
    symbol: str
    unit: str
    line: int
    span: Span | None = None
    evidence: Origin | None = None
    parent_symbol: str | None = None
    member_path: tuple[str, ...] = ()
    target_symbol: str | None = None

    def __post_init__(self):
        relative_path(self.unit)
        require_text(self.symbol, "object symbol")
        if (self.span is None) != (self.evidence is None):
            raise ValueError("bounded object requires original evidence")
        if not isinstance(self.member_path, tuple):
            raise ValueError("object member path must be an immutable tuple")
        if (self.parent_symbol is None) != (not self.member_path):
            raise ValueError("member-backed object requires both parent symbol and member path")
        if self.parent_symbol is not None:
            require_text(self.parent_symbol, "parent object symbol")
            if not valid_member_path(self.member_path):
                raise ValueError("object member path must contain names and canonical fixed indices")
        if self.target_symbol is not None:
            require_text(self.target_symbol, "target object symbol")
            if self.parent_symbol is not None:
                raise ValueError("object target alias cannot be combined with a member owner")
        if self.span is not None:
            if self.span.start != self.origin.address:
                raise ValueError("object extent must begin at its original identity")
            if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
                raise ValueError("object evidence belongs to another original")

    @property
    def interpretation(self):
        if self.span is None:
            raise ValueError("unbounded object has no initializer interpretation")
        return {"kind": "object_initial_data", "origin": asdict(self.origin),
                "span": asdict(self.span), "symbol": self.symbol,
                "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        value = self.interpretation
        return digest({"entity_id": self.evidence.entity_id, "value": value})

    @property
    def emitted_symbol(self) -> str:
        """Current COFF definition name; ``symbol`` remains historical."""
        return self.parent_symbol or self.target_symbol or self.symbol


def parse_objects(text: str, path: str, pin: str, *, address_space: str) -> tuple[ObjectBinding, ...]:
    """Parse current or historical object identities without reading live files."""
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = BOUNDED.fullmatch(source_line.strip())
        if match:
            start, end, symbol, evidence = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          Span(int(start, 16), int(end, 16)), Origin(pin, address_space, int(evidence, 16))))
            continue
        match = TARGET_BOUNDED.fullmatch(source_line.strip())
        if match:
            start, end, symbol, evidence, target = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          Span(int(start, 16), int(end, 16)),
                                          Origin(pin, address_space, int(evidence, 16)),
                                          target_symbol=target))
            continue
        match = TARGET_UNBOUNDED.fullmatch(source_line.strip())
        if match:
            start, symbol, target = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          target_symbol=target))
            continue
        match = MEMBER_BOUNDED.fullmatch(source_line.strip())
        if match:
            start, end, symbol, evidence, parent, member_path = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          Span(int(start, 16), int(end, 16)), Origin(pin, address_space, int(evidence, 16)),
                                          parent, tuple(member_path.split("."))))
            continue
        match = MEMBER_UNBOUNDED.fullmatch(source_line.strip())
        if match:
            start, symbol, parent, member_path = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          parent_symbol=parent, member_path=tuple(member_path.split("."))))
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if match is None:
            raise ValueError(f"invalid object annotation at {path}:{line}")
        bindings.append(ObjectBinding(Origin(pin, address_space, int(match[1], 16)), match[2], path, line))
    result = tuple(bindings)
    validate_object_symbol_ownership(result)
    return result


def read_objects(root: Path, pin: str) -> tuple[ObjectBinding, ...]:
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_objects(path.read_text(encoding="utf-8"), relative, pin, address_space=configuration.address_space))
    if len({item.symbol for item in bindings}) != len(bindings):
        raise ValueError("duplicate original object symbol binding")
    if len({item.origin.entity_id for item in bindings}) != len(bindings):
        raise ValueError("multiple objects claim the same original start address")
    owners = {}
    for item in bindings:
        if item.parent_symbol is not None:
            owner = owners.setdefault(item.parent_symbol, item.unit)
            if owner != item.unit:
                raise ValueError("member parent symbol is bound from multiple source units")
    spans = sorted(item.span for item in bindings if item.span is not None)
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("original object extents overlap")
    validate_object_symbol_ownership(bindings)
    return tuple(bindings)


def validate_object_symbol_ownership(bindings) -> None:
    """Keep historical object IDs distinct from current member owner symbols.

    A member binding's old ``symbol`` remains an independent original
    identity, while ``parent_symbol`` names the current emitted COFF owner.
    Reusing one token across those roles would redirect a relocation that
    historically named the old child to the unrelated parent object. Multiple
    children may share a parent, but only from the same source unit.
    """
    rows = tuple(bindings)
    historical = [row.symbol for row in rows]
    if len(set(historical)) != len(historical):
        raise ValueError("duplicate original object symbol binding")
    member_ids = {row.symbol for row in rows if row.parent_symbol is not None}
    parent_units = {}
    for row in rows:
        if row.parent_symbol is None:
            continue
        owner = parent_units.setdefault(row.parent_symbol, row.unit)
        if owner != row.unit:
            raise ValueError("member parent symbol is bound from multiple source units")
    # A direct original identity can anchor its own emitted parent object.
    # Historical child names still cannot alias any emitted member owner.
    aliases = member_ids & set(parent_units)
    if aliases:
        raise ValueError("historical object identity conflicts with an emitted member owner: "
                         + sorted(aliases)[0])
    emitted = {}
    historical = {row.symbol: row for row in rows}
    for row in rows:
        name = row.emitted_symbol
        prior = emitted.setdefault(name, row)
        if prior.unit != row.unit:
            raise ValueError("current object owner must remain in one translation unit")
        if prior is not row:
            # A parent binding and its children deliberately share a COFF
            # definition; unrelated direct roots and sibling roots do not.
            if not (row.parent_symbol == name or prior.parent_symbol == name
                    or (row.parent_symbol == prior.emitted_symbol == name)):
                raise ValueError("one current object target is bound to multiple original objects")
        old = historical.get(name)
        same_parent_anchor = (old is not None and old is not row
                             and old.parent_symbol is None and old.emitted_symbol == name
                             and old.unit == row.unit and row.parent_symbol == name)
        if old is not None and old is not row and not same_parent_anchor:
            if old.unit != row.unit:
                raise ValueError("current object owner must remain in one translation unit")
            raise ValueError("current object target conflicts with another historical object identity")
    for parent, unit in parent_units.items():
        direct = [row for row in rows if row.parent_symbol is None and row.emitted_symbol == parent]
        if direct and (len(direct) != 1 or direct[0].unit != unit):
            raise ValueError("direct parent object identity must share the member owner's translation unit")
        if direct and any((row.origin.image_sha256, row.origin.address_space) !=
                          (direct[0].origin.image_sha256, direct[0].origin.address_space)
                          for row in rows if row.parent_symbol == parent):
            raise ValueError("direct parent and member identities belong to different original address domains")


def object_targets(bindings: tuple[ObjectBinding, ...], objects: dict) -> tuple[dict, list]:
    """Require the owner's compiled definition before resolving object references."""
    validate_object_symbol_ownership(bindings)
    targets, reports = {}, []
    emitted = {}
    for binding in bindings:
        symbol = binding.emitted_symbol
        identity = ("owner", binding.unit, symbol)
        if symbol in emitted and emitted[symbol] != identity:
            raise ValueError("one emitted object symbol is bound to multiple original object identities")
        emitted[symbol] = identity
    groups = {}
    for binding in bindings:
        if binding.parent_symbol is not None:
            groups.setdefault((binding.unit, binding.parent_symbol), []).append(binding)
    bad_groups = set()
    resolved_groups = {}
    for (unit, parent), rows in groups.items():
        obj = objects.get(unit)
        if obj is None:
            continue
        try:
            from .codeview_data import member_data_layout
            layouts = [member_data_layout(obj, parent, row.member_path) for row in rows]
            bases = [row.origin.address - layout["member_offset"] for row, layout in zip(rows, layouts)]
            if any(layout["section"] != layouts[0]["section"] or layout["symbol_offset"] != layouts[0]["symbol_offset"]
                   or layout["parent_size"] != layouts[0]["parent_size"]
                   for layout in layouts):
                raise ValueError("member bindings do not resolve to one parent definition")
            if (len(set(bases)) != 1 or not 0 <= bases[0] < 2**32
                    or bases[0] + layouts[0]["parent_size"] > 2**32):
                raise ValueError("member bindings do not imply one original parent base")
            resolved_groups[(unit, parent)] = (bases[0], layouts)
        except ValueError:
            bad_groups.add((unit, parent))
    direct_parents = {(row.unit, row.emitted_symbol): row for row in bindings
                      if row.parent_symbol is None and any(
                          member.unit == row.unit and member.parent_symbol == row.emitted_symbol
                          for member in bindings)}
    # A directly bound parent anchors the whole member group. Refuse its
    # target too if fresh member layout is missing or implies another base.
    for group, direct in direct_parents.items():
        resolved = resolved_groups.get(group)
        if group in bad_groups or resolved is None or resolved[0] != direct.origin.address:
            bad_groups.add(group)
    for binding in bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue  # The task does not prove objects owned by uncompiled TUs.
        target_symbol = binding.emitted_symbol
        matches = [symbol for symbol in obj.symbols.values() if symbol.name == target_symbol
                   and symbol.storage == 2 and symbol.section > 0 and not symbol.type & 0x20]
        valid = len(matches) == 1 and not obj.sections[matches[0].section - 1].flags & 0x20
        address = binding.origin.address
        if binding.parent_symbol is not None:
            group = (binding.unit, binding.parent_symbol)
            valid = valid and group not in bad_groups and group in resolved_groups
            if valid:
                address = resolved_groups[group][0]
        elif (binding.unit, binding.emitted_symbol) in direct_parents:
            group = (binding.unit, binding.emitted_symbol)
            valid = valid and group not in bad_groups and group in resolved_groups
        reports.append({"symbol": binding.symbol, "target_symbol": binding.emitted_symbol, "unit": binding.unit,
                        "address": f"0x{binding.origin.address:08X}",
                        "definition": "pass" if valid else "unknown",
                        **({"parent_symbol": binding.parent_symbol, "member_path": ".".join(binding.member_path),
                           "original_parent": f"0x{address:08X}"} if binding.parent_symbol is not None else {})})
        if valid:
            if binding.parent_symbol is None:
                targets[binding.emitted_symbol] = binding.origin.address
            else:
                old = targets.setdefault(binding.parent_symbol, address)
                if old != address:
                    targets.pop(binding.parent_symbol, None)
                    reports[-1]["definition"] = "unknown"
    return targets, reports


def emitted_object_symbols(bindings) -> frozenset[str]:
    """Historical IDs plus current emitted names for namespace reservation."""
    return frozenset(symbol for binding in bindings
                     for symbol in (binding.symbol, binding.emitted_symbol)
                     if symbol is not None)


def require_disjoint_object_symbols(bindings, *other_namespaces) -> frozenset[str]:
    """Refuse aliases between old object IDs/current owners and peer namespaces."""
    validate_object_symbol_ownership(bindings)
    symbols = emitted_object_symbols(bindings)
    for namespace in other_namespaces:
        overlap = symbols & set(namespace)
        if overlap:
            raise ValueError("object symbol conflicts with another source namespace: " + sorted(overlap)[0])
    return symbols


def referenced_symbol_units(owners: dict[str, str], objects: dict) -> frozenset[str]:
    """Schedule declared owners named by actual external COFF relocations."""
    required = set()
    for obj in objects.values():
        for section in obj.sections:
            for _, index, kind in section.relocations:
                if kind not in (6, 20):
                    continue
                symbol = obj.symbols[index]
                if symbol.storage == 2 and symbol.name in owners:
                    required.add(owners[symbol.name])
    return frozenset(required)


def referenced_object_units(bindings: tuple[ObjectBinding, ...], objects: dict) -> frozenset[str]:
    """Find declared owners of actual external relocation references.

    This schedules read-only compilation, not target resolution or task editing
    authorization. Bare symbol declarations, local symbols and weak-external
    fallbacks cannot select a source owner. Callers retain definition checks and
    compile each dependency under the same source overlay and flag trial.
    """
    validate_object_symbol_ownership(bindings)
    owners = {}
    for binding in bindings:
        symbol = binding.emitted_symbol
        owner = owners.setdefault(symbol, binding.unit)
        if owner != binding.unit:
            raise ValueError("object relocation symbol has multiple declared source owners")
    if len({binding.symbol for binding in bindings}) != len(bindings):
        raise ValueError("duplicate original object symbol binding")
    return referenced_symbol_units(owners, objects)
