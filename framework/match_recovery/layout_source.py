"""Original direct-member offset expectations, owned by C++ source comments.

These are evidence-backed interpretations of the original, not observations of
the current compiler output. A passing offset check is not full class recovery.
"""

from dataclasses import dataclass
from pathlib import Path
import re

from .model import Origin
from .components import relative_path


PREFIX = "// RECOVERY_MEMBER:"
PATTERN = re.compile(r"// RECOVERY_MEMBER: v1 ([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*) ([A-Za-z_]\w*) (0x[0-9a-fA-F]+) (0x[0-9a-fA-F]+)")


@dataclass(frozen=True)
class MemberExpectation:
    class_name: str
    member: str
    offset: int
    evidence: Origin
    path: str
    line: int


def parse_members(text: str, path: str, pin: str) -> tuple[MemberExpectation, ...]:
    """Parse current or historical source without consulting the filesystem."""
    relative_path(path)
    parts = Path(path).parts
    if Path(path).suffix not in (".cpp", ".h") or parts[:2] in (("src", "recovered"), ("src", "unrecovered")):
        return ()
    rows = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if not match or max(int(match[3], 16), int(match[4], 16)) > 0xFFFFFFFF:
            raise ValueError(f"invalid original member expectation at {path}:{line}")
        rows.append(MemberExpectation(match[1], match[2], int(match[3], 16),
                                       Origin(pin, "pe:va", int(match[4], 16)), path, line))
    return tuple(rows)


def read_members(root: Path, pin: str) -> tuple[MemberExpectation, ...]:
    root = root.resolve()
    rows = []
    for path in sorted((root / "src").rglob("*")):
        if path.suffix not in (".cpp", ".h") or not path.is_file():
            continue
        if path.relative_to(root / "src").parts[0] in ("recovered", "unrecovered"):
            continue
        if not path.resolve().is_relative_to(root):
            raise ValueError("member annotation resolves outside source root")
        rows.extend(parse_members(path.read_text(encoding="utf-8"), path.relative_to(root).as_posix(), pin))
    if len({(row.class_name, row.member) for row in rows}) != len(rows):
        raise ValueError("duplicate original member expectation")
    return tuple(rows)
