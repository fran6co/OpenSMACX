"""Original member/base expectations, owned by C++ source comments.

These are evidence-backed interpretations of the original, not observations of
the current compiler output. A passing offset check is not full class recovery.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.model import Origin, digest
from match_recovery.components import relative_path
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_MEMBER:"
PATTERN = re.compile(r"// RECOVERY_MEMBER: v(?P<version>[1234]) (?P<class_name>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) (?P<target>[A-Za-z_]\w*) (?:(?P<class_type>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)(?:(?P<pointer>\*)|\[(?P<count>[1-9][0-9]*)\])? )?(?P<offset>0x[0-9a-fA-F]+) (?:(?P<array_size>0x[0-9a-fA-F]+) )?(?P<evidence>0x[0-9a-fA-F]+)")
BASE_PREFIX = "// RECOVERY_BASE:"
BASE_PATTERN = re.compile(r"// RECOVERY_BASE: v1 (?P<class_name>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) (?P<target>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) (?P<offset>0x[0-9a-fA-F]+) (?P<evidence>0x[0-9a-fA-F]+)")


@dataclass(frozen=True)
class MemberExpectation:
    class_name: str
    member: str
    offset: int
    evidence: Origin
    path: str
    line: int
    class_type: str | None = None
    class_pointer_type: str | None = None
    class_array_type: str | None = None
    array_count: int | None = None
    array_size: int | None = None

    def __post_init__(self):
        if any(value is not None for value in (self.class_array_type, self.array_count, self.array_size)):
            if (not self.class_array_type or self.class_type is not None or self.class_pointer_type is not None
                    or type(self.array_count) is not int or self.array_count <= 0
                    or type(self.array_size) is not int or self.array_size <= 0
                    or self.array_size % self.array_count):
                raise ValueError("invalid original member array expectation")

    @property
    def interpretation(self):
        return {"kind": "direct_member_offset", "class_name": self.class_name,
                "member": self.member, "offset": self.offset,
                **({"class_type": self.class_type} if self.class_type is not None else {}),
                **({"class_pointer_type": self.class_pointer_type} if self.class_pointer_type is not None else {}),
                **({"class_array_type": self.class_array_type, "array_count": self.array_count,
                    "array_size": self.array_size} if self.class_array_type is not None else {}),
                "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})


@dataclass(frozen=True)
class BaseExpectation:
    class_name: str
    base: str
    offset: int
    evidence: Origin
    path: str
    line: int

    @property
    def interpretation(self):
        return {"kind": "nonvirtual_base_offset", "class_name": self.class_name,
                "base": self.base, "offset": self.offset, "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})


def _parse(text, path, pin, prefix, pattern, factory, label, address_space):
    """Parse current or historical source without consulting the filesystem."""
    relative_path(path)
    rows = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(prefix):
            continue
        match = pattern.fullmatch(source_line.strip())
        if not match:
            raise ValueError(f"invalid original {label} expectation at {path}:{line}")
        extra = {}
        if label == "member":
            shape = tuple(match[key] is not None for key in ("class_type", "pointer", "count", "array_size"))
            if shape != {"1": (False, False, False, False), "2": (True, False, False, False),
                         "3": (True, True, False, False), "4": (True, False, True, True)}[match["version"]]:
                raise ValueError(f"invalid original member expectation version/type at {path}:{line}")
            if match["version"] == "4":
                extra.update(class_array_type=match["class_type"], array_count=int(match["count"]),
                             array_size=int(match["array_size"], 16))
            else:
                extra["class_pointer_type" if match["pointer"] else "class_type"] = match["class_type"]
        rows.append(factory(match["class_name"], match["target"], int(match["offset"], 16),
                            Origin(pin, address_space, int(match["evidence"], 16)), path, line, **extra))
    return tuple(rows)


def parse_members(text: str, path: str, pin: str, *, address_space: str) -> tuple[MemberExpectation, ...]:
    return _parse(text, path, pin, PREFIX, PATTERN, MemberExpectation, "member", address_space)


def parse_bases(text: str, path: str, pin: str, *, address_space: str) -> tuple[BaseExpectation, ...]:
    return _parse(text, path, pin, BASE_PREFIX, BASE_PATTERN, BaseExpectation, "base", address_space)


def _read(root, pin, parser, label, field):
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    rows = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        rows.extend(parser(path.read_text(encoding="utf-8"), relative, pin, address_space=configuration.address_space))
    if len({(row.class_name, getattr(row, field)) for row in rows}) != len(rows):
        raise ValueError(f"duplicate original {label} expectation")
    return tuple(rows)


def read_members(root: Path, pin: str) -> tuple[MemberExpectation, ...]:
    return _read(root, pin, parse_members, "member", "member")


def read_bases(root: Path, pin: str) -> tuple[BaseExpectation, ...]:
    return _read(root, pin, parse_bases, "base", "base")
