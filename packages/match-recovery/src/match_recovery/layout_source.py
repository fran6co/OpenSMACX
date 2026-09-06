"""Original member/base expectations, owned by C++ source comments.

These are evidence-backed interpretations of the original, not observations of
the current compiler output. A passing offset check is not full class recovery.
"""

from dataclasses import dataclass
from pathlib import Path
import re

from match_recovery.model import Origin
from match_recovery.components import relative_path


PREFIX = "// RECOVERY_MEMBER:"
PATTERN = re.compile(r"// RECOVERY_MEMBER: v(?P<version>[12]) (?P<class_name>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) (?P<target>[A-Za-z_]\w*) (?:(?P<class_type>[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) )?(?P<offset>0x[0-9a-fA-F]+) (?P<evidence>0x[0-9a-fA-F]+)")
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


@dataclass(frozen=True)
class BaseExpectation:
    class_name: str
    base: str
    offset: int
    evidence: Origin
    path: str
    line: int


def _parse(text, path, pin, prefix, pattern, factory, label):
    """Parse current or historical source without consulting the filesystem."""
    relative_path(path)
    parts = Path(path).parts
    if Path(path).suffix not in (".cpp", ".h") or parts[:2] in (("src", "recovered"), ("src", "unrecovered")):
        return ()
    rows = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(prefix):
            continue
        match = pattern.fullmatch(source_line.strip())
        if not match or max(int(match["offset"], 16), int(match["evidence"], 16)) > 0xFFFFFFFF:
            raise ValueError(f"invalid original {label} expectation at {path}:{line}")
        extra = {}
        if label == "member":
            if (match["version"] == "2") != (match["class_type"] is not None):
                raise ValueError(f"invalid original member expectation version/type at {path}:{line}")
            extra["class_type"] = match["class_type"]
        rows.append(factory(match["class_name"], match["target"], int(match["offset"], 16),
                            Origin(pin, "pe:va", int(match["evidence"], 16)), path, line, **extra))
    return tuple(rows)


def parse_members(text: str, path: str, pin: str) -> tuple[MemberExpectation, ...]:
    return _parse(text, path, pin, PREFIX, PATTERN, MemberExpectation, "member")


def parse_bases(text: str, path: str, pin: str) -> tuple[BaseExpectation, ...]:
    return _parse(text, path, pin, BASE_PREFIX, BASE_PATTERN, BaseExpectation, "base")


def _read(root, pin, parser, label, field):
    root = root.resolve()
    rows = []
    for path in sorted((root / "src").rglob("*")):
        if path.suffix not in (".cpp", ".h") or not path.is_file():
            continue
        if path.relative_to(root / "src").parts[0] in ("recovered", "unrecovered"):
            continue
        if not path.resolve().is_relative_to(root):
            raise ValueError(f"{label} annotation resolves outside source root")
        rows.extend(parser(path.read_text(encoding="utf-8"), path.relative_to(root).as_posix(), pin))
    if len({(row.class_name, getattr(row, field)) for row in rows}) != len(rows):
        raise ValueError(f"duplicate original {label} expectation")
    return tuple(rows)


def read_members(root: Path, pin: str) -> tuple[MemberExpectation, ...]:
    return _read(root, pin, parse_members, "member", "member")


def read_bases(root: Path, pin: str) -> tuple[BaseExpectation, ...]:
    return _read(root, pin, parse_bases, "base", "base")
