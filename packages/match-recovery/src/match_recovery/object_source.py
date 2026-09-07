"""Source-owned original object identities, not inferred address aliases.

An annotation sits beside the owning definition. Its address identifies the
object start, not a pointer variable containing that address. Object extent,
layout, initialization and lifetime remain separate verification obligations.
"""

from dataclasses import dataclass
from pathlib import Path
import re

from match_recovery.model import Origin
from match_recovery.components import relative_path
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_OBJECT:"
PATTERN = re.compile(r"// RECOVERY_OBJECT: v1 (0x[0-9A-Fa-f]+) (\S+)\s*")


@dataclass(frozen=True)
class ObjectBinding:
    origin: Origin
    symbol: str
    unit: str
    line: int


def parse_objects(text: str, path: str, pin: str, *, address_space: str) -> tuple[ObjectBinding, ...]:
    """Parse current or historical object identities without reading live files."""
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if match is None:
            raise ValueError(f"invalid object annotation at {path}:{line}")
        bindings.append(ObjectBinding(Origin(pin, address_space, int(match[1], 16)), match[2], path, line))
    return tuple(bindings)


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
    return tuple(bindings)


def object_targets(bindings: tuple[ObjectBinding, ...], objects: dict) -> tuple[dict, list]:
    """Require the owner's compiled definition before resolving object references."""
    targets, reports = {}, []
    for binding in bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue  # The task does not prove objects owned by uncompiled TUs.
        matches = [symbol for symbol in obj.symbols.values() if symbol.name == binding.symbol
                   and symbol.storage == 2 and symbol.section > 0 and not symbol.type & 0x20]
        valid = len(matches) == 1 and not obj.sections[matches[0].section - 1].flags & 0x20
        reports.append({"symbol": binding.symbol, "unit": binding.unit,
                        "address": f"0x{binding.origin.address:08X}",
                        "definition": "pass" if valid else "unknown"})
        if valid:
            targets[binding.symbol] = binding.origin.address
    return targets, reports
