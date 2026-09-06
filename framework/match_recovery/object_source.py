"""Source-owned original object identities, not inferred address aliases.

An annotation sits beside the owning definition. Its address identifies the
object start, not a pointer variable containing that address. Object extent,
layout, initialization and lifetime remain separate verification obligations.
"""

from dataclasses import dataclass
from pathlib import Path
import re

from .model import Origin


PREFIX = "// RECOVERY_OBJECT:"
PATTERN = re.compile(r"// RECOVERY_OBJECT: v1 (0x[0-9A-Fa-f]+) (\S+)\s*")


@dataclass(frozen=True)
class ObjectBinding:
    origin: Origin
    symbol: str
    unit: str
    line: int


def read_objects(root: Path, pin: str) -> tuple[ObjectBinding, ...]:
    bindings = []
    for path in sorted((root / "src").rglob("*.cpp")):
        if path.relative_to(root / "src").parts[0] in ("recovered", "unrecovered"):
            continue
        if not path.resolve().is_relative_to(root.resolve()):
            raise ValueError("object annotation resolves outside source root")
        for line, text in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if not text.lstrip().startswith(PREFIX):
                continue
            match = PATTERN.fullmatch(text.strip())
            if match is None:
                raise ValueError(f"invalid object annotation at {path}:{line}")
            address = int(match[1], 16)
            if address > 0xFFFFFFFF:
                raise ValueError("original PE object address exceeds 32 bits")
            bindings.append(ObjectBinding(Origin(pin, "pe:va", address), match[2],
                                          path.relative_to(root).as_posix(), line))
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
